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

	enum class MODEL_FRONT_FACE : uint8_t {
		NONE,
		CCW,
		CW
	};

	enum class MODEL_FILE_TYPE : uint8_t {
		OBJ
	};

	struct ModelData {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		MODEL_FRONT_FACE frontFace = MODEL_FRONT_FACE::CCW;
	};

	class Model;

	class ModelsManager {
	public:
		Model* createModel(const ModelData& data);
		Model* createModel(const MODEL_FILE_TYPE& type, const char* data, size_t length);
		void deleteModel(uint32_t modelId);
		void deleteModel(Model* model);

		Model* getModel(uint32_t modelId) const;

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
		uint32_t getId() const;
		Context* getContext() const;

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
