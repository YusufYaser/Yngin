#pragma once
#include <Yngin/Forward.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <Yngin/Utils/Meta.h>

namespace Yngin {
	struct Vertex {
		glm::vec3 pos;
		glm::vec2 texCoord;
		glm::vec3 normal;
		uint32_t matId;
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
		uint8_t materialsCount = 0;
		uint32_t defaultMaterials[256];
	};

	class ModelsManager {
	public:
		Model* createModel(const ModelData& data);
		Model* createModel(const MODEL_FILE_TYPE& type, const char* data, size_t length);
		Model* createModel(const ModelData& data, uint32_t id, bool override = false);
		Model* createModel(const MODEL_FILE_TYPE& type, const char* data, size_t length, uint32_t id, bool override = false);
		void deleteModel(uint32_t modelId);
		void deleteModel(Model* model);

		size_t getModelsCount() const;
		std::vector<Model*> getModels() const;

		Model* getModel(uint32_t modelId) const;

	private:
		friend class Context;
		friend struct std::default_delete<ModelsManager>;
		friend class Model;

		ModelsManager(Context* ctx);
		~ModelsManager();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class Model {
	public:
		uint32_t getId() const;
		Context* getContext() const;

		Meta meta;

		void render();

		const ModelData& getModelData() const;

	private:
		Model(Context* ctx);
		~Model();

		friend class ModelsManager;
		friend struct std::default_delete<Model>;
		friend class Context;

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
