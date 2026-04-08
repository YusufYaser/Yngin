#include "GameFiles.h"
#include "../Core/Context/ResourcesPak.h"
#include "../Core/Scenes/ScenePak.h"
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
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>
#include <sol/sol.hpp>
#include "../Core/Scripting/Scripting_Internal.h"

#define R(name, type) s.read(reinterpret_cast<char*>(&name), sizeof(type))

using namespace Yngin::GameFiles;
using namespace Yngin::GameFiles::ResourcesPak;
using namespace Yngin::GameFiles::ScenePak;

namespace Yngin::GameFiles {
	bool Loaders::modelsManager(std::istream& s, ModelsManager* mgr) {
		PakModelData pakModelData{};
		R(pakModelData, PakModelData);

		ModelData modelData{};
		modelData.frontFace = pakModelData.frontFace;

		modelData.materialsCount = pakModelData.materialsCount;
		for (int i = 0; i < pakModelData.materialsCount; i++) {
			modelData.defaultMaterials[i] = pakModelData.defaultMaterials[i];
		}

		for (int i = 0; i < pakModelData.verticesCount; i++) {
			ModelVertexData v{};
			R(v, ModelVertexData);
			modelData.vertices.push_back(Vertex{
				.pos = glm::make_vec3(v.position),
				.texCoord = glm::make_vec2(v.texCoord),
				.normal = glm::make_vec3(v.normal),
				.matId = v.material
				});
		}

		for (int i = 0; i < pakModelData.indicesCount; i++) {
			uint32_t index = 0;
			R(index, uint32_t);
			modelData.indices.push_back(index);
		}

		Model* model = mgr->createModel(modelData, pakModelData.id, true);

		meta(s, model->meta);

		return true;
	}

	bool Loaders::texturesManager(std::istream& s, TexturesManager* mgr) {
		PakTextureData pakTexData{};
		R(pakTexData, PakTextureData);

		// 100 MB
		if (pakTexData.dataSize > 1e+8) {
			return false;
		}

		TextureSettings settings{};
		settings.wrap = pakTexData.wrap;
		settings.filterMin = pakTexData.filterMin;
		settings.filterMag = pakTexData.filterMag;

		Texture* tex = nullptr;

		switch (pakTexData.dataFormat) {
		case TEXTURE_FORMAT::RAW:
		{
			TextureData data{};

			TextureRawDataHeader rawDataHeader{};
			R(rawDataHeader, TextureRawDataHeader);

			data.width = rawDataHeader.width;
			data.height = rawDataHeader.height;
			data.numCh = rawDataHeader.numCh;

			char* pixels = new char[data.width * data.height * data.numCh];
			s.read(pixels, data.width * data.height * data.numCh);

			data.bytes = pixels;

			tex = mgr->createTexture(data, settings, pakTexData.id, true);

			delete[] pixels;

			break;
		}

		case TEXTURE_FORMAT::PNG:
		{
			char* pakBytes = new char[pakTexData.dataSize];
			s.read(pakBytes, pakTexData.dataSize);

			TextureData data{};

			unsigned char* bytes = stbi_load_from_memory((const stbi_uc*)pakBytes, int(pakTexData.dataSize), &data.width, &data.height, &data.numCh, 0);
			delete[] pakBytes;

			if (!bytes) break;

			data.bytes = (const char*)bytes;

			tex = mgr->createTexture(data, settings, pakTexData.id, true);

			stbi_image_free(bytes);

			break;
		}

		case TEXTURE_FORMAT::PATH:
		{
			char* pakBytes = new char[pakTexData.dataSize];
			s.read(pakBytes, pakTexData.dataSize);

			tex = mgr->createTexture(pakBytes, settings, pakTexData.id, true);

			delete[] pakBytes;
			break;
		}
		}


		if (tex == nullptr) return false;

		meta(s, tex->meta);

		return true;
	}

	bool Loaders::scriptsManager(std::istream& s, ScriptsManager* mgr) {
		ScriptData scriptData;
		R(scriptData, ScriptData);

		// 20 MB
		if (scriptData.dataSize > 2e+7) {
			return false;
		}

		std::vector<char> bytes(scriptData.dataSize);
		s.read(bytes.data(), scriptData.dataSize);

		Scene* scene = nullptr;
		if (scriptData.scene != -1) {
			scene = mgr->getContext()->getScenesManager()->getScene(scriptData.scene);
		}

		Script* script = mgr->createScript(scene, "", scriptData.id, true);

		script->impl->enabled = scriptData.enabled;

		sol::load_result chunk = mgr->impl->lua.load(std::string_view(bytes.data(), bytes.size()));

		if (!chunk.valid()) {
			sol::error error = chunk;

			printf("[Yngin] [Script #%i] Error while loading script from resources: %s\n", scriptData.id, error.what());
			return true;
		}

		script->impl->byteCode = bytes;

		lua_State* L = mgr->impl->lua.lua_state();

		sol::protected_function func = chunk;

		func.push(L);
		script->impl->env.push(L);
		lua_setupvalue(L, -2, 1);
		lua_pop(L, 1);

		sol::protected_function_result res = func();

		if (!res.valid()) {
			sol::error error = res;

			printf("[Yngin] [Script #%i] Error while loading script from resources:: %s\n", scriptData.id, error.what());
		}

		meta(s, script->meta);

		return true;
	}

	bool Loaders::camerasManager(std::istream& s, CamerasManager* mgr) {
		CameraData cameraData{};
		R(cameraData, CameraData);

		Camera* camera = mgr->createCamera(cameraData.id, true);
		glm::vec3 position = glm::make_vec3(cameraData.position);
		camera->setPosition(position);
		glm::vec3 orientation = glm::make_vec3(cameraData.orientation);
		camera->setOrientation(orientation);

		camera->setFov(cameraData.fov);
		camera->setWeight(cameraData.weight);

		meta(s, camera->meta);

		return true;
	}

	bool Loaders::uiManager(std::istream& s, UI::UIManager* mgr, std::map<int, int>& parentsQueue) {
		UIElementData elementData{};
		R(elementData, UIElementData);

		UI::UIElement* element = nullptr;

		switch (elementData.type) {
		case UI_TYPE::NONE:
		{
			element = mgr->getRootElement()->createChild<UI::UIElement>(elementData.id, true);
			break;
		}

		case UI_TYPE::BUTTON:
		{
			UIButtonData buttonData{};
			R(buttonData, UIButtonData);

			UI::Button* button = mgr->getRootElement()->createChild<UI::Button>(elementData.id, true);
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
				image = mgr->getRootElement()->createChild<UI::Image>(elementData.id, true);
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
				text = mgr->getRootElement()->createChild<UI::Text>(elementData.id, true);
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

		parentsQueue[elementData.id] = elementData.parent;

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

		meta(s, element->meta);

		return true;
	}

	bool Loaders::meta(std::istream& s, Meta& meta) {
		MetaHeader metaHeader{};
		R(metaHeader, MetaHeader);

		for (int i = 0; i < metaHeader.metasCount; i++) {
			MetaGeneric generic{};
			R(generic, MetaGeneric);

			char* keyBytes = new char[generic.keyLength];
			s.read(keyBytes, generic.keyLength);
			std::string key(keyBytes, generic.keyLength);
			delete[] keyBytes;

			switch (generic.type) {
			case META_TYPE::STRING:
			{
				MetaStringData stringData{};
				R(stringData, MetaStringData);

				char* strBytes = new char[stringData.length];
				s.read(strBytes, stringData.length);
				std::string str(strBytes, stringData.length);
				delete[] strBytes;

				meta.setMeta(key, str);

				break;
			}

			case META_TYPE::INT:
			{
				int intVal;
				s >> intVal;

				meta.setMeta(key, intVal);

				break;
			}

			case META_TYPE::FLOAT:
			{
				int floatVal;
				s >> floatVal;

				meta.setMeta(key, floatVal);

				break;
			}
			}
		}

		return true;
	}

	bool Loaders::materialsManager(std::istream& s, MaterialsManager* mgr) {
		PakMaterialData data{};
		R(data, PakMaterialData);

		Material* mat = mgr->createMaterial(data.id, true);

		mat->setAmbientColor(data.ambientColor);
		mat->setDiffuseColor(data.diffuseColor);
		mat->setSpecularColor(data.specularColor);
		mat->setSpecularComponent(data.specularComponent);

		return true;
	}
}
