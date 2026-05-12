#include <Yngin/Rendering/Shaders.h>
#include <stdexcept>
#include "Shaders_Internal.h"

namespace Yngin {
	ShadersManager::ShadersManager(Context* ctx) {
		if (ctx->getShadersManager()) {
			throw std::invalid_argument("Context already has a shaders manager!");
		}

		impl = std::make_unique<Impl>();

		impl->ctx = ctx;

		for (int i = 1; i <= 4; i++) {
			impl->shaders[SHADER_TYPE(i)] = std::unique_ptr<Shader>(new Shader(ctx, SHADER_TYPE(i)));
		}
	}

	ShadersManager::~ShadersManager() = default;

	Shader* ShadersManager::getShader(const SHADER_TYPE& shaderType) const {
		return impl->shaders[shaderType].get();
	}

	Shader* ShadersManager::getActive() const {
		return impl->activeShader;
	}

	void ShadersManager::setActive(const SHADER_TYPE& type) {
		Shader* shader = getShader(type);
		shader->activate();
	}

	void ShadersManager::setActive(Shader* shader) {
		if (shader->impl->ctx == impl->ctx) {
			shader->activate();
		}
	}
}
