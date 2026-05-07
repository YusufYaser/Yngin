#include <Yngin/Rendering/Textures.h>
#include <stb/stb_image.h>
#include "GameFiles.h"
#include "ScenePak.h"
#include "ResourcesPak.h"

#define R(name, type) s.read(reinterpret_cast<char*>(&name), sizeof(type))

using namespace Yngin::GameFiles;
using namespace Yngin::GameFiles::ResourcesPak;
using namespace Yngin::GameFiles::ScenePak;

namespace Yngin::GameFiles {
	bool Validators::modelsManager(std::istream& s) {
		PakModelData pakModelData{};
		if (!R(pakModelData, PakModelData)) return false;

		for (int i = 0; i < pakModelData.verticesCount; i++) {
			ModelVertexData v{};
			if (!R(v, ModelVertexData)) return false;
		}

		for (int i = 0; i < pakModelData.indicesCount; i++) {
			uint32_t index = 0;
			if (!R(index, uint32_t)) return false;
		}

		if (!meta(s)) return false;

		return true;
	}

	bool Validators::texturesManager(std::istream& s) {
		PakTextureData pakTexData{};
		if (!R(pakTexData, PakTextureData)) return false;

		// 100 MB
		if (pakTexData.dataSize > 1e+8) {
			return false;
		}

		switch (pakTexData.dataFormat) {
		case TEXTURE_FORMAT::RAW:
		{
			TextureRawDataHeader rawDataHeader{};
			if (!R(rawDataHeader, TextureRawDataHeader)) return false;

			TextureData data{};

			data.width = rawDataHeader.width;
			data.height = rawDataHeader.height;
			data.numCh = rawDataHeader.numCh;

			size_t size = data.width * data.height * data.numCh;

			ValidatorCheck(char, size);
			Seek(size);

			break;
		}

		case TEXTURE_FORMAT::PNG:
		{
			ValidatorCheck(char, pakTexData.dataSize);

			char* pakBytes = new char[pakTexData.dataSize];
			s.read(pakBytes, pakTexData.dataSize);

			TextureData data{};

			unsigned char* bytes = stbi_load_from_memory((const stbi_uc*)pakBytes, int(pakTexData.dataSize), &data.width, &data.height, &data.numCh, 0);
			delete[] pakBytes;

			if (bytes == NULL) return false;

			stbi_image_free(bytes);

			break;
		}

		case TEXTURE_FORMAT::PATH:
		{
			ValidatorCheck(char, pakTexData.dataSize);
			Seek(pakTexData.dataSize);

			break;
		}
		}

		if (!meta(s)) return false;

		return true;
	}

	bool Validators::scriptsManager(std::istream& s) {
		ScriptData scriptData;
		if (!R(scriptData, ScriptData)) return false;

		// 20 MB
		if (scriptData.dataSize > 2e+7) {
			return false;
		}

		ValidatorCheck(char, scriptData.dataSize);
		Seek(scriptData.dataSize);

		if (!meta(s)) return false;

		return true;
	}

	bool Validators::camerasManager(std::istream& s) {
		CameraData cameraData{};
		if (!R(cameraData, CameraData)) return false;

		if (!meta(s)) return false;

		return true;
	}

	bool Validators::uiManager(std::istream& s) {
		UIElementData elementData{};
		if (!R(elementData, UIElementData)) return false;

		switch (elementData.type) {
		case UI_TYPE::NONE:
		{
			break;
		}

		case UI_TYPE::BUTTON:
		{
			UIButtonData buttonData{};
			if (!R(buttonData, UIButtonData)) return false;
		}
		case UI_TYPE::IMAGE:
		{
			UIImageData imageData{};
			if (!R(imageData, UIImageData)) return false;

			if (elementData.type != UI_TYPE::BUTTON) {
				break;
			}
		}

		case UI_TYPE::TEXT:
		{
			UITextData textData{};
			if (!R(textData, UITextData)) return false;

			ValidatorCheck(char, textData.textLength);
			Seek(textData.textLength);

			break;
		}

		default:
		{
			return false;
		}
		}

		if (!meta(s)) return false;

		return true;
	}

	bool Validators::materialsManager(std::istream& s) {
		PakMaterialData data{};
		if (!R(data, PakMaterialData)) return false;

		if (!meta(s)) return false;

		return true;
	}

	bool Validators::meta(std::istream& s) {
		MetaHeader metaHeader{};
		if (!R(metaHeader, MetaHeader)) return false;

		for (int i = 0; i < metaHeader.metasCount; i++) {
			MetaGeneric generic{};
			if (!R(generic, MetaGeneric)) return false;

			ValidatorCheck(char, generic.keyLength);

			char* keyBytes = new char[generic.keyLength];
			s.read(keyBytes, generic.keyLength);
			std::string key(keyBytes, generic.keyLength);
			delete[] keyBytes;

			switch (generic.type) {
			case META_TYPE::STRING:
			{
				MetaStringData stringData{};
				if (!R(stringData, MetaStringData)) return false;

				ValidatorCheck(char, stringData.length);
				Seek(stringData.length);

				break;
			}

			case META_TYPE::INT:
			{
				ValidatorCheck(int, 1);
				Seek(sizeof(int));

				break;
			}

			case META_TYPE::FLOAT:
			{
				ValidatorCheck(float, 1);
				Seek(sizeof(float));

				break;
			}
			}
		}

		return true;
	}
}
