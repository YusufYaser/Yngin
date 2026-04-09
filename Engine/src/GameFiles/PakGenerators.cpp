#include "GameFiles.h"
#include "ResourcesPak.h"
#include "ScenePak.h"
#include <Yngin/Core/Models.h>
#include <Yngin/Core/Materials.h>
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
			if (model->meta.getMetaInt("#NoExport", 0)) break;

			op.op = OP::MODEL;
			W(op, Operation);

			const ModelData& data = model->getModelData();

			PakModelData pakModelData{};
			pakModelData.id = model->getId();
			pakModelData.frontFace = data.frontFace;

			pakModelData.materialsCount = data.materialsCount;
			for (int i = 0; i < data.materialsCount; i++) {
				pakModelData.defaultMaterials[i] = data.defaultMaterials[i];
			}

			pakModelData.verticesCount = uint16_t(data.vertices.size());
			pakModelData.indicesCount = uint16_t(data.indices.size());

			W(pakModelData, PakModelData);

			for (auto& vertex : data.vertices) {
				ModelVertexData v{};
				v.position = vertex.pos;
				v.texCoord = vertex.texCoord;
				v.normal = vertex.normal;
				v.material = vertex.matId;
				W(v, ModelVertexData);
			}

			for (auto& index : data.indices) {
				W(index, uint32_t);
			}

			meta(s, model->meta, mgr->getContext());
		}

		return true;
	}

	bool Generators::texturesManager(std::ostream& s, TexturesManager* mgr, bool compressed) {
		Operation op;

		Texture* activatedTexture = mgr->getActive();
		for (auto texture : mgr->getTextures()) {
			if (texture->meta.getMetaInt("#NoExport", 0)) break;

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

			if (compressed) {
				struct WriteContext {
					size_t offset = 0;
					size_t sizeCache = 0;
					std::vector<char> buf;
				} context;

				context.sizeCache = width * height * 4 + 18;
				context.buf.resize(context.sizeCache);

				stbi_write_png_to_func([](void* context, void* data, int size) {
					auto ctx = (WriteContext*)context;

					if (ctx->offset + size > ctx->sizeCache) {
						ctx->sizeCache = ctx->offset + size;
						ctx->buf.resize(ctx->sizeCache);
					}

					memcpy_s(ctx->buf.data() + ctx->offset, size, data, size);

					ctx->offset += size;
					}, &context, width, height, 4, pixels, 0);


				pakTexData.dataSize = context.offset;
				W(pakTexData, PakTextureData);

				s.write(context.buf.data(), pakTexData.dataSize);
			} else {
				pakTexData.dataFormat = TEXTURE_FORMAT::RAW;
				pakTexData.dataSize = sizeof(TextureRawDataHeader) + (width * height * 4);
				W(pakTexData, PakTextureData);

				TextureRawDataHeader rawDataHeader{};
				rawDataHeader.width = width;
				rawDataHeader.height = height;
				rawDataHeader.numCh = 4;
				W(rawDataHeader, TextureRawDataHeader);

				s.write(pixels, width * height * 4);
			}

			delete[] pixels;

			meta(s, texture->meta, mgr->getContext());
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

			if (script->meta.getMetaInt("#NoExport", 0)) break;

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

			meta(s, script->meta, mgr->getContext());
		}

		return true;
	}

	bool Generators::gameObjectsManager(std::ostream& s, const GameObjectsManager* mgr) {
		Operation op;

		for (auto& obj : mgr->getGameObjects()) {
			if (obj->impl->id == 0) continue;

			if (obj->meta.getMetaInt("#NoExport", 0)) break;

			op.op = OP::GAMEOBJECT;
			W(op, Operation);

			GameObjectData objData{};
			objData.id = obj->impl->id;
			objData.parent = obj->impl->parent->impl->id;
			objData.position = obj->impl->pos;
			objData.rotation = obj->impl->rotation;
			objData.scale = obj->impl->scale;

			W(objData, GameObjectData);

			meta(s, obj->meta, mgr->getContext());

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
				meshData.color = color;

				for (int i = 0; i < 256; i++) {
					meshData.materials[i] = mesh->getMaterial(i);
				}

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
				lightData.color = color;

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

				rigidBodyData.velocity = rigidBody->getVelocity();

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
				boxColliderData.offset = offset;
				glm::vec3 size = boxCollider->getSize();
				boxColliderData.size = size;

				W(boxColliderData, BoxColliderData);
			}
		}

		return true;
	}

	bool Generators::camerasManager(std::ostream& s, const CamerasManager* mgr) {
		Operation op;

		for (auto& camera : mgr->getCameras()) {
			if (camera->meta.getMetaInt("#NoExport", 0)) break;

			op.op = OP::CAMERA;
			W(op, Operation);

			CameraData cameraData{};
			cameraData.id = camera->getId();
			glm::vec3 position = camera->getPosition();
			cameraData.position = position;
			glm::vec3 orientation = camera->getOrientation();
			cameraData.orientation = orientation;

			cameraData.fov = camera->getFov();
			cameraData.weight = camera->getWeight();

			W(cameraData, CameraData);

			meta(s, camera->meta, mgr->getContext());
		}

		return true;
	}

	bool Generators::uiManager(std::ostream& s, const UI::UIManager* mgr) {
		Operation op;

		for (auto& element : mgr->getElements()) {
			if (element->getId() == 0) continue;

			if (element->meta.getMetaInt("#NoExport", 0)) break;

			op.op = OP::UIELEMENT;
			W(op, Operation);

			UI::UITransform pos = element->getPosition();
			UI::UITransform size = element->getSize();

			UIElementData elementData{};
			elementData.id = element->getId();
			elementData.parent = element->getParent()->getId();

			elementData.position = pos;
			elementData.size = size;

			elementData.crop = element->getCrop();

			elementData.color = element->getColor();

			elementData.pivot = element->getPivot();

			elementData.type = element->getType();

			W(elementData, UIElementData);

			switch (elementData.type) {
			case UI_TYPE::BUTTON:
			{
				UI::Button* button = dynamic_cast<UI::Button*>(element);

				UIButtonData buttonData{};

				buttonData.hoverColor = button->getHoverColor();
				buttonData.clickColor = button->getClickColor();

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

				textData.spacing = text->getSpacing();

				textData.centered[0] = text->isTextCentered().x == 1;
				textData.centered[1] = text->isTextCentered().y == 1;

				std::string textString = text->getText();
				textData.textLength = textString.size();

				W(textData, UITextData);

				s.write(textString.data(), textData.textLength);

				break;
			}
			}

			meta(s, element->meta, mgr->getContext());
		}

		return true;
	}

	namespace {
		META_TYPE getMetaType(const MetaValue& val) {
			if (std::holds_alternative<std::string>(val)) {
				return META_TYPE::STRING;
			} else if (std::holds_alternative<int>(val)) {
				return META_TYPE::INT;
			} else if (std::holds_alternative<float>(val)) {
				return META_TYPE::FLOAT;
			} else if (std::holds_alternative<void*>(val)) {
				return META_TYPE::POINTER;
			}

			return META_TYPE::POINTER;
		}
	}

	bool Generators::meta(std::ostream& s, const Meta& meta, Context* ctx) {
		PakGenSettings settings = ctx->getCurrentGenPakSettings();

		auto metas = meta.getMetas();

		MetaHeader metaHeader{};
		for (auto& [key, val] : metas) {
			if (key[0] == '#') continue;

			bool ignored = false;
			for (auto& prefix : settings.ignoredMetaPrefixes) {
				if (key.starts_with(prefix)) {
					ignored = true;
					break;
				}
			}
			if (ignored) continue;

			metaHeader.metasCount++;
		}
		W(metaHeader, MetaHeader);

		for (auto& [key, val] : metas) {
			if (key[0] == '#') continue;

			bool ignored = false;
			for (auto& prefix : settings.ignoredMetaPrefixes) {
				if (key.starts_with(prefix)) {
					ignored = true;
					break;
				}
			}
			if (ignored) continue;

			MetaGeneric generic{};
			generic.type = getMetaType(val);
			// pointers cannot be exported
			if (generic.type == META_TYPE::POINTER) continue;

			generic.keyLength = key.length();

			W(generic, MetaGeneric);

			s.write(key.c_str(), generic.keyLength);

			switch (generic.type) {
			case META_TYPE::STRING:
			{
				auto str = std::get_if<std::string>(&val);

				MetaStringData strData{};
				strData.length = str->length();
				W(strData, MetaStringData);

				s.write(str->c_str(), str->length());

				break;
			}

			case META_TYPE::INT:
			{
				auto intVal = std::get_if<int>(&val);

				s << *intVal;

				break;
			}

			case META_TYPE::FLOAT:
			{
				auto floatVal = std::get_if<float>(&val);

				s << *floatVal;

				break;
			}
			}
		}

		return true;
	}

	bool Generators::materialsManager(std::ostream& s, const MaterialsManager* mgr) {
		Operation op;

		for (auto& mat : mgr->getMaterials()) {
			op.op = OP::MATERIAL;
			W(op, Operation);

			PakMaterialData data{};
			data.id = mat->getId();

			data.ambientColor = mat->getAmbientColor();
			data.diffuseColor = mat->getDiffuseColor();
			data.specularColor = mat->getSpecularColor();
			data.specularComponent = mat->getSpecularComponent();
			W(data, PakMaterialData);
		}

		return true;
	}
}
