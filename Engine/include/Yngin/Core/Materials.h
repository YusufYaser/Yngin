#pragma once
#include <Yngin/Forward.h>
#include <vector>
#include <memory>
#include <glm/vec3.hpp>
#include <string>
#include <map>
#include <Yngin/Utils/Meta.h>

namespace Yngin {
	class MaterialsManager {
	public:
		Context* getContext() const;

		Material* createMaterial();
		Material* createMaterial(uint16_t id, bool override = false);
		Material* getMaterial(uint16_t materialId);

		void deleteMaterial(uint16_t materialId);
		void deleteMaterial(Material* material);

		size_t getMaxMaterialsCount() const;
		size_t getMaterialsCount() const;
		std::vector<Material*> getMaterials() const;

		std::map<std::string, uint16_t> loadMtl(const char* data, size_t length);

	private:
		friend class Context;
		friend struct std::default_delete<MaterialsManager>;
		friend class Material;

		MaterialsManager(Context* ctx);
		~MaterialsManager();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class Material {
	public:
		uint16_t getId() const;
		Context* getContext() const;

		Meta meta;

		glm::vec3 getAmbientColor() const;
		void setAmbientColor(glm::vec3 color);

		glm::vec3 getDiffuseColor() const;
		void setDiffuseColor(glm::vec3 color);

		glm::vec3 getSpecularColor() const;
		void setSpecularColor(glm::vec3 color);

		float getSpecularComponent() const;
		void setSpecularComponent(float component);

	private:
		Material(Context* ctx);
		~Material();

		friend class MaterialsManager;
		friend struct std::default_delete<Material>;
		friend class Context;

		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}
