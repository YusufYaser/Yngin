#pragma once
#include <Yngin/Yngin.h>
#include <glm/vec3.hpp>
#include <memory>

namespace Yngin {
	struct Vertex {
		glm::vec3 pos;
		glm::vec2 texCoord;
		glm::vec3 normal;
	};

	enum class MODEL_FILE_TYPE : uint8_t {
		OBJ
	};

	class ModelsManager {
	public:
		ModelsManager(Context* ctx);
		~ModelsManager();

		uint32_t createModel(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		uint32_t createModel(MODEL_FILE_TYPE type, const char* data, size_t length);
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
