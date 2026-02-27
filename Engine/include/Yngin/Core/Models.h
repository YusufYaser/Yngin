#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <memory>

namespace Yngin {
	struct Vertex {
		glm::vec3 pos;
		glm::vec2 texCoord;
		glm::vec3 normal;
	};

	struct ModelData {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};

	enum class MODEL_FILE_TYPE : uint8_t {
		OBJ
	};

	class Model;

	class ModelsManager {
	public:
		uint32_t createModel(const ModelData& data);
		uint32_t createModel(MODEL_FILE_TYPE type, const char* data, size_t length);
		void deleteModel(uint32_t modelId);

		Model* getModel(uint32_t modelId);

	private:
		friend class Context;
		friend struct std::default_delete<ModelsManager>;

		ModelsManager(Context* ctx);
		~ModelsManager();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class Model {
	public:
		void render();

	private:
		Model(Context* ctx);
		~Model();

		friend class ModelsManager;
		friend struct std::default_delete<Model>;

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
