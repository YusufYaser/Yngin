#pragma once
#include <Yngin/Yngin.h>
#include <glm/vec3.hpp>
#include <memory>

namespace Yngin {
	struct Vertex {
		glm::vec3 pos;
	};

	class ModelsManager {
	public:
		ModelsManager(Context* ctx);
		~ModelsManager();

		uint32_t createModel(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		void deleteModel(uint32_t modelId);

		void render(uint32_t modelId);

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class Model {
	public:
		~Model();

		void render();

	private:
		Model(Context* ctx, std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		friend class ModelsManager;

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
