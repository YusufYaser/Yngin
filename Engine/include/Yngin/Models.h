#pragma once
#include <Yngin/Yngin.h>
#include <glm/vec3.hpp>
#include <memory>

namespace Yngin {
	struct Vertex {
		glm::vec3 pos;
	};

	class Model;

	class ModelsManager {
	public:
		ModelsManager(Context* ctx);
		~ModelsManager();

		uint32_t createModel(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		void deleteModel(uint32_t modelId);

		void renderModel(uint32_t modelId);

	private:
		Context* ctx;

		std::map<uint32_t, std::unique_ptr<Model>> models;
		uint32_t nextModelId = 0;
	};
}
