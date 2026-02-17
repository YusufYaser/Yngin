#include <Yngin/Models.h>
#include <glad/glad.h>
#include <stdexcept>
#include "Models_Internal.h"

namespace Yngin {
	ModelsManager::ModelsManager(Context* ctx) {
		if (ctx->getModelsManager()) {
			throw std::invalid_argument("Context already has a model manager!");
		}

		this->ctx = ctx;
	}

	ModelsManager::~ModelsManager() = default;

	uint32_t ModelsManager::createModel(std::vector<Vertex> vertices, std::vector<uint32_t> indices) {
		uint32_t modelId = nextModelId++;
		models[modelId] = std::make_unique<Model>(ctx, vertices, indices);
		return modelId;
	}

	void ModelsManager::deleteModel(uint32_t modelId) {
		assert(models.find(modelId) != models.end());

		models.erase(modelId);
	}

	void ModelsManager::render(uint32_t modelId) {
		auto it = models.find(modelId);
		assert(it != models.end());
		if (it == models.end()) return;

		it->second->render();
	}
}
