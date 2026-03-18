#include <Yngin/Core/Scenes.h>
#include <Yngin/Rendering/Cameras.h>
#include <Yngin/UI/UIManager.h>
#include <Yngin/UI/Elements/Elements.h>
#include "../GameObject/GameObject_Internal.h"
#include "Scenes_Internal.h"
#include <sstream>
#include "ScenePak.h"
#include <glm/gtc/type_ptr.hpp>

#define R(name, type) s.read(reinterpret_cast<char*>(&name), sizeof(type))
#define W(name, type) s.write(reinterpret_cast<const char*>(&name), sizeof(type))

using namespace Yngin::GameData;
using namespace Yngin::GameData::ScenePak;

namespace Yngin {
	void Scene::Impl::loadPak(const char* bytes, size_t size) {
		std::istringstream s(std::string(bytes, size), std::ios::binary);

		Header header{};
		R(header, Header);
		if (std::memcmp(header.magic, "YNGINSCENE", 10) != 0) return;
		if (header.version != VERSION) return;

		Operation op{};

		std::map<int, int> gameObjectsParentsQueue;
		std::map<int, int> uiElementsParentsQueue;
		GameObject* obj = nullptr;
		while (R(op, Operation)) {
			switch (op.op) {
			case OP::SCENE:
			{
				SceneData sceneData{};
				R(sceneData, SceneData);
				skyboxTexId = sceneData.skyboxTexture;
				lightSettings = { .ambientLight = glm::make_vec3(sceneData.ambientLight) };
				gravity = sceneData.gravity;
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
				CameraData cameraData{};
				R(cameraData, CameraData);

				Camera* camera = camerasManager->createCamera(cameraData.id, true);
				glm::vec3 position = glm::make_vec3(cameraData.position);
				camera->setPosition(position);
				glm::vec3 orientation = glm::make_vec3(cameraData.orientation);
				camera->setOrientation(orientation);

				camera->setFov(cameraData.fov);
				camera->setWeight(cameraData.weight);

				break;
			}

			case OP::UIELEMENT:
			{
				UIElementData elementData{};
				R(elementData, UIElementData);

				UI::UIElement* element = nullptr;

				switch (elementData.type) {
				case UI_TYPE::NONE:
				{
					element = uiManager->getRootElement()->createChild<UI::UIElement>(elementData.id, true);
					break;
				}

				case UI_TYPE::BUTTON:
				{
					UIButtonData buttonData{};
					R(buttonData, UIButtonData);

					UI::Button* button = uiManager->getRootElement()->createChild<UI::Button>(elementData.id, true);
					element = button;

					glm::vec4 hoverColor = glm::make_vec4(buttonData.hoverColor);
					button->setHoverColor(hoverColor);

					glm::vec4 clickColor = glm::make_vec4(buttonData.clickColor);
					button->setClickColor(clickColor);

					// continue to image and text
				}
				case UI_TYPE::IMAGE:
				{
					UIImageData imageData{};
					R(imageData, UIImageData);

					UI::Image* image = nullptr;
					if (elementData.type != UI_TYPE::BUTTON) {
						image = uiManager->getRootElement()->createChild<UI::Image>(elementData.id, true);
						element = image;
					} else {
						image = dynamic_cast<UI::Button*>(element)->getImage();
					}

					image->setTexture(imageData.textureId);

					if (elementData.type != UI_TYPE::BUTTON) {
						break;
					}
				}

				case UI_TYPE::TEXT:
				{
					UITextData textData{};
					R(textData, UITextData);

					UI::Text* text = nullptr;
					if (elementData.type != UI_TYPE::BUTTON) {
						text = uiManager->getRootElement()->createChild<UI::Text>(elementData.id, true);
						element = text;
					} else {
						text = dynamic_cast<UI::Button*>(element)->getTextElement();
					}

					text->setTextSize(textData.size);

					text->setGlyph(textData.glyphId);

					glm::ivec2 spacing = glm::make_vec2(textData.spacing);
					text->setSpacing(spacing);

					glm::ivec2 centered = glm::make_vec2(textData.centered);
					text->setTextCentered(centered);

					std::vector<char> textChars(textData.textLength);
					s.read(textChars.data(), textData.textLength);

					std::string textString(textChars.data(), textChars.size());

					text->setText(textString);

					break;
				}
				}

				uiElementsParentsQueue[elementData.id] = elementData.parent;

				UI::UITransform position{
					.xScale = elementData.positionScale[0],
					.xOffset = elementData.positionOffset[0],
					.yScale = elementData.positionScale[1],
					.yOffset = elementData.positionOffset[1],
				};

				element->setPosition(position);

				UI::UITransform size{
					.xScale = elementData.sizeScale[0],
					.xOffset = elementData.sizeOffset[0],
					.yScale = elementData.sizeScale[1],
					.yOffset = elementData.sizeOffset[1],
				};

				element->setSize(size);

				UI::UICrop crop{
					.start = glm::make_vec2(elementData.cropStart),
					.end = glm::make_vec2(elementData.cropEnd)
				};

				element->setCrop(crop);

				glm::vec4 color = glm::make_vec4(elementData.color);
				element->setColor(color);

				glm::vec2 pivot = glm::make_vec2(elementData.pivot);
				element->setPivot(pivot);
			}
			}
		}

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
		std::memcpy(scene.ambientLight, glm::value_ptr(impl->lightSettings.ambientLight), sizeof(float) * 3);
		scene.gravity = impl->gravity;

		W(scene, SceneData);

		for (auto& obj : impl->gameObjectsManager->getGameObjects()) {
			if (obj->impl->id == 0) continue;

			op.op = OP::GAMEOBJECT;
			W(op, Operation);

			GameObjectData objData{};
			objData.id = obj->impl->id;
			objData.parent = obj->impl->parent->impl->id;
			std::memcpy(objData.position, glm::value_ptr(obj->impl->pos), sizeof(float) * 3);
			std::memcpy(objData.rotation, glm::value_ptr(obj->impl->rotation), sizeof(float) * 3);
			std::memcpy(objData.scale, glm::value_ptr(obj->impl->scale), sizeof(float) * 3);

			W(objData, GameObjectData);

			ComponentData compData{};
			Components::Mesh* mesh = obj->getComponent<Components::Mesh>();
			if (mesh) {
				op.op = OP::COMPONENT;
				W(op, Operation);

				compData.componentType = COMPONENT_TYPE::MESH;
				W(compData, ComponentData);

				MeshData meshData{};
				meshData.modelId = mesh->getModel();
				meshData.textureId = mesh->getTexture();
				glm::vec3 color = mesh->getColor();
				std::memcpy(meshData.color, glm::value_ptr(color), sizeof(float) * 3);

				W(meshData, MeshData);
			}

			Components::Light* light = obj->getComponent<Components::Light>();
			if (light) {
				op.op = OP::COMPONENT;
				W(op, Operation);

				compData.componentType = COMPONENT_TYPE::LIGHT;
				W(compData, ComponentData);

				LightData lightData{};
				lightData.intensity = light->getIntensity();
				lightData.distance = light->getDistance();
				glm::vec3 color = light->getColor();
				std::memcpy(lightData.color, glm::value_ptr(color), sizeof(float) * 3);

				W(lightData, LightData);
			}

			Components::RigidBody* rigidBody = obj->getComponent<Components::RigidBody>();
			if (rigidBody) {
				op.op = OP::COMPONENT;
				W(op, Operation);

				compData.componentType = COMPONENT_TYPE::RIGIDBODY;
				W(compData, ComponentData);

				RigidBodyData rigidBodyData{};
				rigidBodyData.mass = rigidBody->getMass();
				rigidBodyData.elasticity = rigidBody->getElasticity();
				rigidBodyData.canBounce = rigidBody->canBounce();

				std::memcpy(rigidBodyData.velocity, glm::value_ptr(rigidBody->getVelocity()), sizeof(float) * 3);

				auto forces = rigidBody->getForces();
				rigidBodyData.forcesCount = uint8_t(forces.size());

				W(rigidBodyData, RigidBodyData);

				for (auto& force : forces) {
					W(force.x, float);
					W(force.y, float);
					W(force.z, float);
					W(force.w, float);
				}
			}

			Components::BoxCollider* boxCollider = obj->getComponent<Components::BoxCollider>();
			if (boxCollider) {
				op.op = OP::COMPONENT;
				W(op, Operation);

				compData.componentType = COMPONENT_TYPE::BOXCOLLIDER;
				W(compData, ComponentData);

				BoxColliderData boxColliderData{};
				glm::vec3 offset = boxCollider->getOffset();
				std::memcpy(boxColliderData.offset, glm::value_ptr(offset), sizeof(float) * 3);
				glm::vec3 size = boxCollider->getSize();
				std::memcpy(boxColliderData.size, glm::value_ptr(size), sizeof(float) * 3);

				W(boxColliderData, BoxColliderData);
			}
		}

		for (auto& camera : impl->camerasManager->getCameras()) {
			op.op = OP::CAMERA;
			W(op, Operation);

			CameraData cameraData{};
			cameraData.id = camera->getId();
			glm::vec3 position = camera->getPosition();
			std::memcpy(cameraData.position, glm::value_ptr(position), sizeof(float) * 3);
			glm::vec3 orientation = camera->getOrientation();
			std::memcpy(cameraData.orientation, glm::value_ptr(orientation), sizeof(float) * 3);

			cameraData.fov = camera->getFov();
			cameraData.weight = camera->getWeight();

			W(cameraData, CameraData);
		}

		for (auto& element : impl->uiManager->getElements()) {
			if (element->getId() == 0) continue;

			op.op = OP::UIELEMENT;
			W(op, Operation);

			UI::UITransform pos = element->getPosition();
			UI::UITransform size = element->getSize();

			UIElementData elementData{};
			elementData.id = element->getId();
			elementData.parent = element->getParent()->getId();

			elementData.positionScale[0] = pos.xScale;
			elementData.positionScale[1] = pos.yScale;
			elementData.positionOffset[0] = pos.xOffset;
			elementData.positionOffset[1] = pos.yOffset;
			elementData.sizeScale[0] = size.xScale;
			elementData.sizeScale[1] = size.yScale;
			elementData.sizeOffset[0] = size.xOffset;
			elementData.sizeOffset[1] = size.yOffset;

			std::memcpy(elementData.cropStart, glm::value_ptr(element->getCrop().start), sizeof(float) * 2);
			std::memcpy(elementData.cropEnd, glm::value_ptr(element->getCrop().end), sizeof(float) * 2);

			std::memcpy(elementData.color, glm::value_ptr(element->getColor()), sizeof(float) * 4);

			std::memcpy(elementData.pivot, glm::value_ptr(element->getPivot()), sizeof(float) * 2);

			elementData.type = element->getType();

			W(elementData, UIElementData);

			switch (elementData.type) {
			case UI_TYPE::BUTTON:
			{
				UI::Button* button = dynamic_cast<UI::Button*>(element);

				UIButtonData buttonData{};

				std::memcpy(buttonData.hoverColor, glm::value_ptr(button->getHoverColor()), sizeof(float) * 4);
				std::memcpy(buttonData.clickColor, glm::value_ptr(button->getClickColor()), sizeof(float) * 4);

				W(buttonData, UIButtonData);

				// continue to image and text
			}

			case UI_TYPE::IMAGE:
			{
				UI::Image* image = dynamic_cast<UI::Image*>(element);
				if (elementData.type == UI_TYPE::BUTTON) {
					image = dynamic_cast<UI::Button*>(element)->getImage();
				}

				UIImageData imageData{};
				imageData.textureId = image->getTexture();

				W(imageData, UIImageData);

				if (elementData.type != UI_TYPE::BUTTON) {
					break;
				}
			}

			case UI_TYPE::TEXT:
			{
				UI::Text* text = dynamic_cast<UI::Text*>(element);
				if (elementData.type == UI_TYPE::BUTTON) {
					text = dynamic_cast<UI::Button*>(element)->getTextElement();
				}

				UITextData textData{};

				textData.size = text->getTextSize();
				textData.glyphId = text->getGlyph();

				std::memcpy(textData.spacing, glm::value_ptr(text->getSpacing()), sizeof(int) * 2);

				textData.centered[0] = text->isTextCentered().x == 1;
				textData.centered[1] = text->isTextCentered().y == 1;

				std::string textString = text->getText();
				textData.textLength = textString.size();

				W(textData, UITextData);

				s.write(textString.data(), textData.textLength);

				break;
			}
			}
		}

		std::string_view sv = s.view();
		return std::vector<char>(sv.begin(), sv.end());
	}
}
