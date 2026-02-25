#include <Yngin/Core/Models.h>
#include <glad/glad.h>
#include <stdexcept>
#include "Models_Internal.h"

namespace Yngin {
	ModelsManager::ModelsManager(Context* ctx) {
		if (ctx->getModelsManager()) {
			throw std::invalid_argument("Context already has a model manager!");
		}

		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
	}

	ModelsManager::~ModelsManager() = default;

	uint32_t ModelsManager::createModel(std::vector<Vertex> vertices, std::vector<uint32_t> indices) {
		Model* model = new Model(impl->ctx);

		uint32_t modelId = impl->nextId++;
		model->impl->id = modelId;
		impl->models[modelId] = std::unique_ptr<Model>(model);

		model->impl->init(vertices, indices);

		return modelId;
	}

	uint32_t ModelsManager::createModel(MODEL_FILE_TYPE type, const char* data, size_t length) {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		switch (type) {
		case MODEL_FILE_TYPE::OBJ:
			impl->loadObj(data, length, vertices, indices);
			break;
		default:
			throw std::invalid_argument("Invalid model type");
		}

		return createModel(vertices, indices);
	}

	void ModelsManager::deleteModel(uint32_t modelId) {
		assert(impl->models.find(modelId) != impl->models.end());

		impl->models.erase(modelId);
	}

	Model* ModelsManager::getModel(uint32_t modelId) {
		auto it = impl->models.find(modelId);
		assert(it != impl->models.end());
		if (it == impl->models.end()) return nullptr;

		return it->second.get();
	}
}
