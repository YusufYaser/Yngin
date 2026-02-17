#include <Yngin/Models.h>
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
		uint32_t modelId = impl->nextModelId++;
		impl->models[modelId] = std::make_unique<Model>(impl->ctx, vertices, indices);
		return modelId;
	}

	void ModelsManager::deleteModel(uint32_t modelId) {
		assert(impl->models.find(modelId) != impl->models.end());

		impl->models.erase(modelId);
	}

	void ModelsManager::render(uint32_t modelId) {
		auto it = impl->models.find(modelId);
		assert(it != impl->models.end());
		if (it == impl->models.end()) return;

		it->second->render();
	}
}
