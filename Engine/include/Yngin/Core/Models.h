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
		Context* getContext() const;

		Model* createModel(const ModelData& data);
		Model* createModel(const MODEL_FILE_TYPE& type, const char* data, size_t length);
		Model* createModel(const ModelData& data, uint16_t id, bool override = false);
		Model* createModel(const MODEL_FILE_TYPE& type, const char* data, size_t length, uint16_t id, bool override = false);
		void deleteModel(uint16_t modelId);
		void deleteModel(Model* model);

		bool parseObjFile(const char* data, size_t length, ModelData& modelData);

		size_t getMaxModelsCount() const;
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
		uint16_t getId() const;
		Context* getContext() const;

		Meta meta;

		const ModelData& getModelData() const;

		size_t getSubmeshesCount() const;

		glm::vec3 getCenter() const;
		float getRadius() const;

	private:
		Model(Context* ctx);
		~Model();

		friend class ModelsManager;
		friend struct std::default_delete<Model>;
		friend class Context;
		friend class Rendering::Renderer;
		friend class UI::Image;
		friend class UI::Text;

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
