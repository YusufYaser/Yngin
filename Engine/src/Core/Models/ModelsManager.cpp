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

	Model* ModelsManager::createModel(const ModelData& data) {
		Model* model = new Model(impl->ctx);

		uint32_t modelId = impl->nextId++;
		model->impl->id = modelId;
		impl->models[modelId] = std::unique_ptr<Model>(model);

		model->impl->init(data.vertices, data.indices);

		return model;
	}

	Model* ModelsManager::createModel(MODEL_FILE_TYPE type, const char* data, size_t length) {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		switch (type) {
		case MODEL_FILE_TYPE::OBJ:
			impl->loadObj(data, length, vertices, indices);
			break;
		default:
			throw std::invalid_argument("Invalid model type");
		}

		return createModel({ vertices, indices });
	}

	void ModelsManager::deleteModel(uint32_t modelId) {
		impl->models.erase(modelId);
	}

	void ModelsManager::deleteModel(Model* model) {
		if (model->getContext() == impl->ctx) {
			deleteModel(model->impl->id);
		}
	}

	Model* ModelsManager::getModel(uint32_t modelId) {
		auto it = impl->models.find(modelId);
		if (it == impl->models.end()) return nullptr;

		return it->second.get();
	}
}
