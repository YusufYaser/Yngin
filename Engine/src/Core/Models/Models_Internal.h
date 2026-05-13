#pragma once
#include <Yngin/Core/Models.h>
#include <Yngin/Core/Context.h>
#include <glad/glad.h>
#include <map>
#include <optional>

namespace Yngin {
	constexpr size_t MAX_MODELS = std::numeric_limits<uint16_t>::max();

	struct InternalSubmesh {
		~InternalSubmesh();

		Model* model;

		GLuint VAO;
		GLuint VBO;
		GLuint EBO;
		uint32_t matId;

		size_t verticesCount;
		size_t indicesCount;

		glm::vec3 center;
		float radius;
	};

	struct Model::Impl {
		Context* ctx;
		void init(const ModelData& d);

		Model* owner;

		void render(int instances = 1);
		void renderWithMaterials(const uint32_t materialsMap[256], int instances = 1);

		uint16_t id;

		std::vector<std::unique_ptr<InternalSubmesh>> submeshes;

		GLsizei indicesCount;

		ModelData modelData;

		uint32_t materials[256];
	};

	struct ModelsManager::Impl {
		Context* ctx;

		size_t loadedModels = 0;
		std::unique_ptr<Model> models[MAX_MODELS];

		std::optional<uint16_t> getAvailableId();
		uint16_t nextId = 0;
		std::vector<uint16_t> deletedIds;
	};
}
