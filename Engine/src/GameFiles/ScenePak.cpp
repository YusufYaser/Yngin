#include <Yngin/Core/Scenes.h>
#include <Yngin/Rendering/Cameras.h>
#include <Yngin/UI/UIManager.h>
#include <Yngin/UI/Elements/Elements.h>
#include "../Core/GameObject/GameObject_Internal.h"
#include "../Core/Scenes/Scenes_Internal.h"
#include <sstream>
#include "ScenePak.h"
#include <glm/gtc/type_ptr.hpp>
#include "GameFiles.h"

#define R(name, type) s.read(reinterpret_cast<char*>(&name), sizeof(type))
#define W(name, type) s.write(reinterpret_cast<const char*>(&name), sizeof(type))

using namespace Yngin::GameFiles;
using namespace Yngin::GameFiles::ScenePak;

namespace Yngin {
	bool ScenesManager::validateScenePak(const char* bytes, size_t size) {
		std::istringstream s(std::string(bytes, size), std::ios::binary);

		Header header{};
		if (!R(header, Header)) return false;
		if (std::memcmp(header.magic, "YNGINSCENE", 10) != 0) return false;
		if (header.version != VERSION) return false;

		Operation op{};

		bool gameObjectActive = false;
		std::map<COMPONENT_TYPE, bool> addedComponents;

		while (R(op, Operation)) {
			switch (op.op) {
			case OP::SCENE:
			{
				SceneData sceneData{};
				if (!R(sceneData, SceneData)) return false;

				if (!Validators::meta(s)) return false;

				break;
			}

			case OP::GAMEOBJECT:
			{
				GameObjectData objData{};
				if (!R(objData, GameObjectData)) return false;
				gameObjectActive = true;
				addedComponents.clear();

				if (!Validators::meta(s)) return false;

				break;
			}

			case OP::COMPONENT:
			{
				if (!gameObjectActive) return false;

				ComponentData componentData{};
				if (!R(componentData, ComponentData)) return false;

				switch (componentData.componentType) {
				case COMPONENT_TYPE::MESH:
				{
					if (addedComponents[COMPONENT_TYPE::MESH]) return false;
					addedComponents[COMPONENT_TYPE::MESH] = true;

					MeshData meshData{};
					if (!R(meshData, MeshData)) return false;

					break;
				}
				case COMPONENT_TYPE::LIGHT:
				{
					if (addedComponents[COMPONENT_TYPE::LIGHT]) return false;
					addedComponents[COMPONENT_TYPE::LIGHT] = true;

					LightData lightData{};
					if (!R(lightData, LightData)) return false;

					break;
				}
				case COMPONENT_TYPE::RIGIDBODY:
				{
					if (addedComponents[COMPONENT_TYPE::RIGIDBODY]) return false;
					addedComponents[COMPONENT_TYPE::RIGIDBODY] = true;

					RigidBodyData rigidBodyData{};
					if (!R(rigidBodyData, RigidBodyData)) return false;

					for (int i = 0; i < rigidBodyData.forcesCount; i++) {
						ValidatorCheck(float, 4);
						Seek(sizeof(float) * 4);
					}

					break;
				}
				case COMPONENT_TYPE::BOXCOLLIDER:
				{
					if (addedComponents[COMPONENT_TYPE::BOXCOLLIDER]) return false;
					addedComponents[COMPONENT_TYPE::BOXCOLLIDER] = true;

					BoxColliderData boxColliderData{};
					if (!R(boxColliderData, BoxColliderData)) return false;

					break;
				}
				}

				break;
			}

			case OP::CAMERA:
			{
				if (!Validators::camerasManager(s)) return false;
				break;
			}

			case OP::UIELEMENT:
			{
				if (!Validators::uiManager(s)) return false;
				break;
			}

			case OP::SCRIPT:
			{
				if (!Validators::scriptsManager(s)) return false;
				break;
			}

			default:
			{
				return false;
			}
			}
		}

		return true;
	}

	void Scene::Impl::loadPak(const char* bytes, size_t size) {
		if (!ScenesManager::validateScenePak(bytes, size)) return;

		std::istringstream s(std::string(bytes, size), std::ios::binary);

		Header header{};
		R(header, Header);
		if (std::memcmp(header.magic, "YNGINSCENE", 10) != 0) return;
		if (header.version != VERSION) return;

		Operation op{};

		bool stop = false;

		std::map<int, int> gameObjectsParentsQueue;
		std::map<int, int> uiElementsParentsQueue;
		GameObject* obj = nullptr;
		while (!stop && R(op, Operation)) {
			switch (op.op) {
			case OP::SCENE:
			{
				SceneData sceneData{};
				R(sceneData, SceneData);
				skyboxTexId = sceneData.skyboxTexture;
				lightSettings = { .ambientLight = sceneData.ambientLight };
				gravity = sceneData.gravity;

				Loaders::meta(s, owner->meta, ctx);
				break;
			}

			case OP::GAMEOBJECT:
			{
				GameObjectData objData{};
				R(objData, GameObjectData);
				obj = gameObjectsManager->getRootGameObject()->createChild(objData.id, true);

				if (obj) {
					gameObjectsParentsQueue[obj->getId()] = objData.parent;
					obj->impl->pos = glm::make_vec3(objData.position);
					obj->impl->rotation = glm::make_vec3(objData.rotation);
					obj->impl->scale = glm::make_vec3(objData.scale);
				}

				Loaders::meta(s, obj->meta, ctx);
				break;
			}

			case OP::COMPONENT:
			{
				ComponentData componentData{};
				R(componentData, ComponentData);

				if (obj == nullptr) break;

				switch (componentData.componentType) {
				case COMPONENT_TYPE::MESH:
				{
					MeshData meshData{};
					R(meshData, MeshData);
					if (obj) {
						auto mesh = obj->createComponent<Components::Mesh>();
						if (mesh) {
							mesh->setModel(meshData.modelId);
							mesh->setTexture(meshData.textureId);
							glm::vec3 color = glm::make_vec3(meshData.color);
							mesh->setColor(color);

							for (int i = 0; i < 256; i++) {
								mesh->setMaterial(i, meshData.materials[i]);
							}
						}
					}
					break;
				}
				case COMPONENT_TYPE::LIGHT:
				{
					LightData lightData{};
					R(lightData, LightData);
					if (obj) {
						auto light = obj->createComponent<Components::Light>();
						if (light) {
							light->setIntensity(lightData.intensity);
							light->setDistance(lightData.distance);
							glm::vec3 color = glm::make_vec3(lightData.color);
							light->setColor(color);
						}
					}
					break;
				}
				case COMPONENT_TYPE::RIGIDBODY:
				{
					RigidBodyData rigidBodyData{};
					R(rigidBodyData, RigidBodyData);
					if (obj) {
						auto rigidBody = obj->createComponent<Components::RigidBody>();
						if (rigidBody) {
							rigidBody->setMass(rigidBodyData.mass);
							rigidBody->setElasticity(rigidBodyData.elasticity);
							rigidBody->setCanBounce(rigidBodyData.canBounce);
							glm::vec3 velocity = glm::make_vec3(rigidBodyData.velocity);
							rigidBody->setVelocity(velocity);
							for (int i = 0; i < rigidBodyData.forcesCount; i++) {
								float force[4]{};
								R(force[0], float);
								R(force[1], float);
								R(force[2], float);
								R(force[3], float);
								glm::vec4 forceVec = glm::make_vec4(force);
								rigidBody->applyForce(forceVec, forceVec.w);
							}
						}
					}
					break;
				}
				case COMPONENT_TYPE::BOXCOLLIDER:
				{
					BoxColliderData boxColliderData{};
					R(boxColliderData, BoxColliderData);
					if (obj) {
						auto boxCollider = obj->createComponent<Components::BoxCollider>();
						if (boxCollider) {
							glm::vec3 offset = glm::make_vec3(boxColliderData.offset);
							boxCollider->setOffset(offset);
							glm::vec3 size = glm::make_vec3(boxColliderData.size);
							boxCollider->setSize(size);
						}
					}
					break;
				}
				}

				break;
			}

			case OP::CAMERA:
			{
				stop = !Loaders::camerasManager(s, camerasManager.get());
				break;
			}

			case OP::UIELEMENT:
			{
				stop = !Loaders::uiManager(s, uiManager.get(), uiElementsParentsQueue);
				break;
			}

			case OP::SCRIPT:
			{
				stop = !Loaders::scriptsManager(s, ctx->getScriptsManager());
				break;
			}

			default:
			{
				stop = true;
				break;
			}
			}
		}

		if (stop) return;

		for (auto& kvp : gameObjectsParentsQueue) {
			int objId = kvp.first;
			int parentId = kvp.second;
			GameObject* obj = gameObjectsManager->getGameObject(objId);
			if (obj == nullptr) continue;
			obj->setParent(parentId);
		}

		for (auto& kvp : uiElementsParentsQueue) {
			int objId = kvp.first;
			int parentId = kvp.second;
			UI::UIElement* obj = uiManager->getElement(objId);
			if (obj == nullptr) continue;
			obj->setParent(parentId);
		}
	}

	std::vector<char> Scene::generatePak() {
		std::ostringstream s(std::ios::binary);

		Header header = {};
		Operation op = {};

		std::memcpy(header.magic, "YNGINSCENE", 10);
		header.version = VERSION;

		W(header, Header);

		op.op = OP::SCENE;
		W(op, Operation);

		SceneData scene{};
		scene.skyboxTexture = impl->skyboxTexId;
		impl->lightSettings.ambientLight = scene.ambientLight;
		scene.gravity = impl->gravity;

		W(scene, SceneData);

		Generators::meta(s, meta, impl->ctx);

		Generators::gameObjectsManager(s, impl->gameObjectsManager.get());
		Generators::camerasManager(s, impl->camerasManager.get());
		Generators::uiManager(s, impl->uiManager.get());
		Generators::scriptsManager(s, impl->ctx->getScriptsManager(), this);

		std::string_view sv = s.view();
		return std::vector<char>(sv.begin(), sv.end());
	}
}
