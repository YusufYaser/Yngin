#include <Yngin/Core/Materials.h>
#include "Materials_Internal.h"
#include <sstream>

namespace Yngin {
	MaterialsManager::MaterialsManager(Context* ctx) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
	}

	MaterialsManager::~MaterialsManager() = default;

	Material* MaterialsManager::createMaterial(uint32_t id, bool override) {
		if (id != -1) {
			if (getMaterial(id) != nullptr) {
				if (override) {
					deleteMaterial(id);
				} else {
					return nullptr;
				}
			}
		} else {
			id = impl->nextId;
		}

		Material* mat = new Material(impl->ctx);
		impl->nextId = std::max(impl->nextId, id + 1);
		mat->impl->id = id;
		impl->materials[id] = std::unique_ptr<Material>(mat);

		return mat;
	}

	Material* MaterialsManager::getMaterial(uint32_t materialId) {
		auto it = impl->materials.find(materialId);
		if (it == impl->materials.end()) return nullptr;

		return it->second.get();
	}

	void MaterialsManager::deleteMaterial(uint32_t materialId) {
		impl->materials.erase(materialId);
	}

	void MaterialsManager::deleteMaterial(Material* material) {
		if (material && material->getContext() == impl->ctx) {
			deleteMaterial(material->impl->id);
		}
	}

	size_t MaterialsManager::getMaterialsCount() const {
		return impl->materials.size();
	}

	std::vector<Material*> MaterialsManager::getMaterials() const {
		std::vector<Material*> materials;
		for (auto& kvp : impl->materials) {
			materials.push_back(kvp.second.get());
		}
		return materials;
	}

	std::map<std::string, uint32_t> MaterialsManager::loadMtl(const char* data, size_t length) {
		std::string str(data, length);
		std::stringstream stream(str);

		std::string l;

		Material* currentMat = nullptr;

		std::map<std::string, uint32_t > createdMats;

		while (std::getline(stream, l)) {
			if (l.empty()) continue;

			std::stringstream s(l);

			std::string cmd;
			s >> cmd;

			if (cmd[0] == '#') continue;

			if (cmd == "newmtl") {
				std::string v;
				s >> v;
				if (impl->nextId != std::numeric_limits<uint32_t>::max()) {
					currentMat = createMaterial();
					createdMats[v] = currentMat->getId();
				}
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
