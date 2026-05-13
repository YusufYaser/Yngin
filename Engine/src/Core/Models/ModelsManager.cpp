#include <Yngin/Core/Models.h>
#include <glad/glad.h>
#include <stdexcept>
#include "Models_Internal.h"

#define LOGGER_NAME Models
#include "../../Internal/Logger.h"

namespace Yngin {
	ModelsManager::ModelsManager(Context* ctx) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
	}

	ModelsManager::~ModelsManager() = default;

	Context* ModelsManager::getContext() const {
		return impl->ctx;
	}

	std::optional<uint16_t> ModelsManager::Impl::getAvailableId() {
		if (loadedModels >= MAX_MODELS) return std::nullopt;

		uint16_t id = nextId;
		while (models[id] && !deletedIds.empty()) {
			id = deletedIds.back();
			deletedIds.pop_back();
		}

		while (models[id] && nextId++ < MAX_MODELS) {
			id = nextId;
			if (nextId >= MAX_MODELS - 1) {
				nextId = 0;
				break;
			}
		}

		if (models[id]) return std::nullopt;

		return id;
	}

	Model* ModelsManager::createModel(const ModelData& data) {
		auto id = impl->getAvailableId();
		if (!id.has_value()) return nullptr;

		return createModel(data, id.value());
	}

	Model* ModelsManager::createModel(const MODEL_FILE_TYPE& type, const char* data, size_t length) {
		auto id = impl->getAvailableId();
		if (!id.has_value()) return nullptr;
		return createModel(type, data, length, id.value());
	}

	Model* ModelsManager::createModel(const ModelData& data, uint16_t id, bool override) {
		if (getModel(id) != nullptr) {
			if (override) {
				deleteModel(id);
			} else {
				return nullptr;
			}
		}

		if (impl->loadedModels >= MAX_MODELS) return nullptr;

		Model* model = new Model(impl->ctx);

		if (id == impl->nextId) impl->nextId++;
		model->impl->id = id;
		impl->models[id] = std::unique_ptr<Model>(model);
		impl->loadedModels++;

		DEBUG("Created model %d", id);

		model->impl->init(data);

		return model;
	}

	Model* ModelsManager::createModel(const MODEL_FILE_TYPE& type, const char* data, size_t length, uint16_t id, bool override) {
		ModelData modelData{};

		switch (type) {
		case MODEL_FILE_TYPE::OBJ:
			if (!parseObjFile(data, length, modelData)) {
				return nullptr;
			}
			break;
		default:
			throw std::invalid_argument("Invalid model type");
		}

		return createModel(modelData, id, override);
	}

	void ModelsManager::deleteModel(uint16_t modelId) {
		if (!impl->models[modelId]) return;

		impl->models[modelId].reset();
		impl->loadedModels--;
		impl->deletedIds.push_back(modelId);

		DEBUG("Deleted model %d", modelId);
	}

	void ModelsManager::deleteModel(Model* model) {
		if (model->getContext() == impl->ctx) {
			deleteModel(model->impl->id);
		}
	}

	size_t ModelsManager::getMaxModelsCount() const {
		return MAX_MODELS;
	}

	size_t ModelsManager::getModelsCount() const {
		return impl->loadedModels;
	}

	std::vector<Model*> ModelsManager::getModels() const {
		std::vector<Model*> models;
		for (auto& model : impl->models) {
			if (model) models.push_back(model.get());
		}
		return models;
	}

	Model* ModelsManager::getModel(uint32_t modelId) const {
		return impl->models[modelId].get();
	}
}
