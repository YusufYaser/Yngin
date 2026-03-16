#include <Yngin/Core/Context.h>
#include "ResourcesPak.h"
#include "Context_Internal.h"
#include <Yngin/Core/Models.h>
#include <Yngin/Rendering/Textures.h>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#define R(name, type) s.read(reinterpret_cast<char*>(&name), sizeof(type))
#define W(name, type) s.write(reinterpret_cast<const char*>(&name), sizeof(type))

using namespace Yngin::GameData;
using namespace Yngin::GameData::ResourcesPak;

namespace Yngin {
	void Context::loadResourcesPak(const char* bytes, size_t size) {
		bool stop = false;

		std::istringstream s(std::string(bytes, size), std::ios::binary);

		Header header{};
		R(header, Header);
		if (std::memcmp(header.magic, "YNGINRESOURCES", 14) != 0) return;
		if (header.version != VERSION) return;

		Operation op{};

		while (!stop && R(op, Operation)) {
			switch (op.op) {
			case OP::MODEL:
			{
				PakModelData pakModelData{};
				R(pakModelData, PakModelData);

				ModelData modelData{};
				modelData.frontFace = pakModelData.frontFace;

				for (int i = 0; i < pakModelData.verticesCount; i++) {
					ModelVertexData v{};
					R(v, ModelVertexData);
					modelData.vertices.push_back(Vertex{
						.pos = glm::make_vec3(v.position),
						.texCoord = glm::make_vec2(v.texCoord),
						.normal = glm::make_vec3(v.normal)
						});
				}

				for (int i = 0; i < pakModelData.indicesCount; i++) {
					uint32_t index = 0;
					R(index, uint32_t);
					modelData.indices.push_back(index);
				}

				impl->modelsManager->createModel(modelData, pakModelData.id, true);
				break;
			}

			case OP::TEXTURE:
			{
				PakTextureData pakTexData{};
				R(pakTexData, PakTextureData);

				// 100 MB
				if (pakTexData.dataSize > 1e+8) {
					stop = true;
					break;
				}

				TextureSettings settings{};
				settings.wrap = pakTexData.wrap;
				settings.filterMin = pakTexData.filterMin;
				settings.filterMag = pakTexData.filterMag;

				TextureData data{};

				char* pakBytes = new char[pakTexData.dataSize];
				s.read(pakBytes, pakTexData.dataSize);
				unsigned char* bytes = stbi_load_from_memory((const stbi_uc*)pakBytes, int(pakTexData.dataSize), &data.width, &data.height, &data.numCh, 0);
				delete[] pakBytes;

				if (!bytes) break;

				data.bytes = (const char*)bytes;

				impl->texturesManager->createTexture(data, settings, pakTexData.id, true);

				stbi_image_free(bytes);

				break;
			}
			}
		}
	}

	std::vector<char> Context::generateResourcesPak() {
		std::ostringstream s(std::ios::binary);

		Header header = {};
		Operation op = {};

		std::memcpy(header.magic, "YNGINRESOURCES", 14);
		header.version = VERSION;

		W(header, Header);

		for (auto& model : impl->modelsManager->getModels()) {
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

		Texture* activatedTexture = impl->texturesManager->getActive();
		for (auto texture : impl->texturesManager->getTextures()) {
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
			impl->texturesManager->setActive(activatedTexture->getId());
		} else {
			impl->texturesManager->setActive(0);
		}

		std::string_view sv = s.view();
		return std::vector<char>(sv.begin(), sv.end());
	}
}
