#include <Yngin/Core/Models.h>
#include <glad/glad.h>
#include <stdexcept>
#include "Models_Internal.h"

namespace Yngin {
	ModelsManager::ModelsManager(Context* ctx) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
	}

	ModelsManager::~ModelsManager() = default;

	Model* ModelsManager::createModel(const ModelData& data) {
		return createModel(data, impl->nextId);
	}

	Model* ModelsManager::createModel(const MODEL_FILE_TYPE& type, const char* data, size_t length) {
		return createModel(type, data, length, impl->nextId);
	}

	Model* ModelsManager::createModel(const ModelData& data, uint32_t id, bool override) {
		if (getModel(id) != nullptr) {
			if (override) {
				deleteModel(id);
			} else {
				return nullptr;
			}
		}

		Model* model = new Model(impl->ctx);

		impl->nextId = std::max(impl->nextId, id + 1);
		model->impl->id = id;
		impl->models[id] = std::unique_ptr<Model>(model);

		model->impl->init(data);

		return model;
	}

	Model* ModelsManager::createModel(const MODEL_FILE_TYPE& type, const char* data, size_t length, uint32_t id, bool override) {
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

	void ModelsManager::deleteModel(uint32_t modelId) {
		impl->models.erase(modelId);
	}

	void ModelsManager::deleteModel(Model* model) {
		if (model->getContext() == impl->ctx) {
			deleteModel(model->impl->id);
		}
	}

	size_t ModelsManager::getModelsCount() const {
		return impl->models.size();
	}

	std::vector<Model*> ModelsManager::getModels() const {
		std::vector<Model*> models;
		for (auto& kvp : impl->models) {
			models.push_back(kvp.second.get());
		}
		return models;
	}

	Model* ModelsManager::getModel(uint32_t modelId) const {
		auto it = impl->models.find(modelId);
		if (it == impl->models.end()) return nullptr;

		return it->second.get();
	}
}
