#pragma once
#include <Yngin/Yngin.h>
#include <glm/vec3.hpp>
#include <memory>

namespace Yngin {
	struct Vertex {
		glm::vec3 pos;
	};

	class Model {
	public:
		// Do not initialize a new model directly
		// You should use Context::createModel() instead
		Model(Context* ctx, std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		~Model();

		void render();

	private:
		Context* ctx;

		struct IDs;
		struct StructureInfo;
		std::unique_ptr<IDs> ids;
		std::unique_ptr<StructureInfo> structureInfo;
	};

	class ModelsManager {
	public:
		ModelsManager(Context* ctx);
		~ModelsManager();

		uint32_t createModel(std::vector<Vertex> vertices, std::vector<uint32_t> indices);

		void renderModel(uint32_t modelId);

	private:
		Context* ctx;

		std::map<uint32_t, Model*> models;
		uint32_t nextModelId = 0;
	};
}
