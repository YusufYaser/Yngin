#include <Yngin/Core/Materials.h>
#include "Materials_Internal.h"
#include <sstream>

#define LOGGER_NAME Materials
#include "../../Internal/Logger.h"

namespace Yngin {
	MaterialsManager::MaterialsManager(Context* ctx) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
	}

	MaterialsManager::~MaterialsManager() = default;

	Context* MaterialsManager::getContext() const {
		return impl->ctx;
	}

	std::optional<uint16_t> MaterialsManager::Impl::getAvailableId() {
		if (loadedMaterials >= MAX_MATERIALS) return std::nullopt;

		uint16_t id = nextId;
		while (materials[id] && !deletedIds.empty()) {
			id = deletedIds.back();
			deletedIds.pop_back();
		}

		while (materials[id] && nextId++ < MAX_MATERIALS) {
			id = nextId;
			if (nextId >= MAX_MATERIALS - 1) {
				nextId = 0;
				break;
			}
		}

		if (materials[id]) return std::nullopt;

		return id;
	}

	Material* MaterialsManager::createMaterial() {
		auto id = impl->getAvailableId();
		if (!id.has_value()) return nullptr;

		return createMaterial(id.value());
	}

	Material* MaterialsManager::createMaterial(uint16_t id, bool override) {
		if (getMaterial(id) != nullptr) {
			if (override) {
				deleteMaterial(id);
			} else {
				return nullptr;
			}
		}

		Material* mat = new Material(impl->ctx);
		if (id == impl->nextId) impl->nextId++;
		mat->impl->id = id;
		impl->materials[id] = std::unique_ptr<Material>(mat);
		impl->loadedMaterials++;

		DEBUG("Created material %d", id);

		return mat;
	}

	Material* MaterialsManager::getMaterial(uint16_t id) {
		return impl->materials[id].get();
	}

	void MaterialsManager::deleteMaterial(uint16_t id) {
		if (!impl->materials[id]) return;

		impl->materials[id].reset();
		impl->loadedMaterials--;
		impl->deletedIds.push_back(id);

		DEBUG("Deleted material %d", id);
	}

	void MaterialsManager::deleteMaterial(Material* material) {
		if (material && material->getContext() == impl->ctx) {
			deleteMaterial(material->impl->id);
		}
	}

	size_t MaterialsManager::getMaxMaterialsCount() const {
		return MAX_MATERIALS;
	}

	size_t MaterialsManager::getMaterialsCount() const {
		return impl->loadedMaterials;
	}

	std::vector<Material*> MaterialsManager::getMaterials() const {
		std::vector<Material*> materials;
		for (auto& material : impl->materials) {
			if (material) materials.push_back(material.get());
		}
		return materials;
	}

	std::map<std::string, uint16_t> MaterialsManager::loadMtl(const char* data, size_t length) {
		std::string str(data, length);
		std::stringstream stream(str);

		std::string l;

		Material* currentMat = nullptr;

		std::map<std::string, uint16_t> createdMats;

		while (std::getline(stream, l)) {
			if (l.empty()) continue;

			std::stringstream s(l);

			std::string cmd;
			s >> cmd;

			if (cmd[0] == '#') continue;

			if (cmd == "newmtl") {
				std::string v;
				s >> v;
				currentMat = createMaterial();
				if (currentMat) createdMats[v] = currentMat->getId();
			} else if (currentMat != nullptr) {
				if (cmd == "Ka") {
					glm::vec3 v;
					s >> v.x >> v.y >> v.z;

					currentMat->setAmbientColor(v);
				} else if (cmd == "Kd") {
					glm::vec3 v;
					s >> v.x >> v.y >> v.z;

					currentMat->setDiffuseColor(v);
				} else if (cmd == "Ks") {
					glm::vec3 v;
					s >> v.x >> v.y >> v.z;

					currentMat->setSpecularColor(v);
				} else if (cmd == "Ns") {
					float v;
					s >> v;

					currentMat->setSpecularComponent(v);
				}
			}

			// TODO: implement other commands
		}

		return createdMats;
	}
}
