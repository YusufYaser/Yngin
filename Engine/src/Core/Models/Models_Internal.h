#pragma once
#include <Yngin/Core/Context.h>
#include <glad/glad.h>
#include <map>

namespace Yngin {
	struct Model::Impl {
		Context* ctx;
		void init(const ModelData& d);

		uint32_t id;

		GLuint VAO;
		GLuint VBO;
		GLuint EBO;

		GLsizei indicesCount;

		MODEL_FRONT_FACE frontFace;

		ModelData modelData;

		uint32_t materials[256];
	};

	struct ModelsManager::Impl {
		Context* ctx;

		std::map<uint32_t, std::unique_ptr<Model>> models;
		uint32_t nextId = 0;

		std::map<uint32_t, Material> materials;
		uint32_t nextMaterialId = 1;

		void loadObj(const char* data, size_t length, ModelData& modelData);
		int loadMtl(const char* data, size_t length, ModelData& modelData, std::map<std::string, uint8_t>& matIds);
	};
}
