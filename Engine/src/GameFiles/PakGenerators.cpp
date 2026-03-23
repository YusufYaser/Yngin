#include "GameFiles.h"
#include "../Core/Context/ResourcesPak.h"
#include "../Core/Scenes/ScenePak.h"
#include <Yngin/Core/Models.h>
#include <Yngin/Rendering/Textures.h>
#include <Yngin/Core/Scripting.h>
#include <Yngin/Core/Scenes.h>
#include <Yngin/Core/GameObject.h>
#include <Yngin/Rendering/Cameras.h>
#include <Yngin/Components/Components.h>
#include <Yngin/UI/UIManager.h>
#include <Yngin/UI/Elements/Elements.h>
#include "../Core/Scripting/Scripting_Internal.h"
#include "../Core/GameObject/GameObject_Internal.h"
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image_write.h>
#include <glad/glad.h>

#define W(name, type) s.write(reinterpret_cast<const char*>(&name), sizeof(type))

using namespace Yngin::GameFiles;
using namespace Yngin::GameFiles::ResourcesPak;
using namespace Yngin::GameFiles::ScenePak;

namespace Yngin::GameFiles {
	bool Generators::modelsManager(std::ostream& s, const ModelsManager* mgr) {
		Operation op;

		for (auto& model : mgr->getModels()) {
			op.op = OP::MODEL;
			W(op, Operation);

			const ModelData& data = model->getModelData();

			PakModelData pakModelData{};
			pakModelData.id = model->getId();
			pakModelData.frontFace = data.frontFace;
			pakModelData.verticesCount = uint8_t(data.vertices.size());
			pakModelData.indicesCount = uint8_t(data.indices.size());

			W(pakModelData, PakModelData);

			for (auto& vertex : data.vertices) {
				ModelVertexData v{};
				std::memcpy(v.position, glm::value_ptr(vertex.pos), sizeof(float) * 3);
				std::memcpy(v.texCoord, glm::value_ptr(vertex.texCoord), sizeof(float) * 2);
				std::memcpy(v.normal, glm::value_ptr(vertex.normal), sizeof(float) * 3);
				W(v, ModelVertexData);
			}

			for (auto& index : data.indices) {
				W(index, uint32_t);
			}
		}

		return true;
	}

	bool Generators::texturesManager(std::ostream& s, TexturesManager* mgr) {
		Operation op;

		Texture* activatedTexture = mgr->getActive();
		for (auto texture : mgr->getTextures()) {
			op.op = OP::TEXTURE;
			W(op, Operation);

			const TextureSettings& settings = texture->getTextureSettings();

			PakTextureData pakTexData{};
			pakTexData.id = texture->getId();
			pakTexData.wrap = settings.wrap;
			pakTexData.filterMin = settings.filterMin;
			pakTexData.filterMag = settings.filterMag;

			pakTexData.dataSize = 0;

			int width, height;
			texture->activate();

			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

			char* pixels = new char[width * height * 4];

			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

			struct WriteContext {
				size_t size;
				std::vector<char> buf;
			} context;

			stbi_write_png_to_func([](void* context, void* data, int size) {
				auto ctx = (WriteContext*)context;

				ctx->size += size;

				const char* bytes = static_cast<const char*>(data);

				ctx->buf.insert(ctx->buf.end(), bytes, bytes + size);
				}, &context, width, height, 4, pixels, 0);

			pakTexData.dataSize = context.buf.size();
			W(pakTexData, PakTextureData);

			s.write(context.buf.data(), pakTexData.dataSize);

			delete[] pixels;
		}
		if (activatedTexture != nullptr) {
			mgr->setActive(activatedTexture->getId());
		} else {
			mgr->setActive(0);
		}

		return true;
	}

	bool Generators::scriptsManager(std::ostream& s, const ScriptsManager* mgr, Scene* scene) {
		Operation op;

		for (auto script : mgr->getScripts()) {
			if (script->getScene() != scene) continue;

			op.op = OP::SCRIPT;
			W(op, Operation);

			ScriptData scriptData{};
			scriptData.id = script->getId();
			if (script->getScene() != nullptr) {
				scriptData.scene = script->getScene()->getId();
			} else {
				scriptData.scene = -1;
			}
			scriptData.enabled = script->isEnabled();
			scriptData.dataSize = script->impl->byteCode.size();

			W(scriptData, ScriptData);

			s.write(script->impl->byteCode.data(), scriptData.dataSize);
		}

		return true;
	}

	bool Generators::gameObjectsManager(std::ostream& s, const GameObjectsManager* mgr) {
		Operation op;

		for (auto& obj : mgr->getGameObjects()) {
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

		return true;
	}

	bool Generators::camerasManager(std::ostream& s, const CamerasManager* mgr) {
		Operation op;

		for (auto& camera : mgr->getCameras()) {
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

		return true;
	}

	bool Generators::uiManager(std::ostream& s, const UI::UIManager* mgr) {
		Operation op;

		for (auto& element : mgr->getElements()) {
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

		return true;
	}
}
