#include <Yngin/Core/Materials.h>
#include "Materials_Internal.h"
#include <Yngin/Services/SerializationService.h>
#include "../../Services/SerializationService/SerializationService_Internal.h"

namespace Yngin {
	Material::Material(Context* ctx) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
	}

	Material::~Material() {
		if (impl->ctx->getStatus() != CONTEXT_STATUS::CLEANING_UP)
			impl->ctx->getService<Services::SerializationService>()->impl->notifyObjectRemoved(impl->streamId, Services::StreamableObjectType::MATERIAL, impl->id);
	}

	uint16_t Material::getId() const {
		return impl->id;
	}

	Context* Material::getContext() const {
		return impl->ctx;
	}

	glm::vec3 Yngin::Material::getAmbientColor() const {
		impl->ctx->getService<Services::SerializationService>()->impl->notifyDataAccessed(impl->streamId, Services::StreamableObjectType::MATERIAL, impl->id);
		return impl->ambientColor;
	}

	void Material::setAmbientColor(glm::vec3 color) {
		impl->ctx->getService<Services::SerializationService>()->impl->notifyDataModified(impl->streamId, Services::StreamableObjectType::MATERIAL, impl->id);
		impl->ambientColor = color;
	}

	glm::vec3 Material::getDiffuseColor() const {
		impl->ctx->getService<Services::SerializationService>()->impl->notifyDataAccessed(impl->streamId, Services::StreamableObjectType::MATERIAL, impl->id);
		return impl->diffuseColor;
	}

	void Material::setDiffuseColor(glm::vec3 color) {
		impl->ctx->getService<Services::SerializationService>()->impl->notifyDataModified(impl->streamId, Services::StreamableObjectType::MATERIAL, impl->id);
		impl->diffuseColor = color;
	}

	glm::vec3 Material::getSpecularColor() const {
		impl->ctx->getService<Services::SerializationService>()->impl->notifyDataAccessed(impl->streamId, Services::StreamableObjectType::MATERIAL, impl->id);
		return impl->specularColor;
	}

	void Material::setSpecularColor(glm::vec3 color) {
		impl->ctx->getService<Services::SerializationService>()->impl->notifyDataModified(impl->streamId, Services::StreamableObjectType::MATERIAL, impl->id);
		impl->specularColor = color;
	}

	float Material::getSpecularComponent() const {
		impl->ctx->getService<Services::SerializationService>()->impl->notifyDataAccessed(impl->streamId, Services::StreamableObjectType::MATERIAL, impl->id);
		return impl->specularComponent;
	}

	void Material::setSpecularComponent(float component) {
		impl->ctx->getService<Services::SerializationService>()->impl->notifyDataModified(impl->streamId, Services::StreamableObjectType::MATERIAL, impl->id);
		impl->specularComponent = component;
	}
}
