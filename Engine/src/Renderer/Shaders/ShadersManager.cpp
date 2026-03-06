#include <Yngin/Renderer/Shaders.h>
#include <stdexcept>
#include "Shaders_Internal.h"

namespace Yngin {
	ShadersManager::ShadersManager(Context* ctx) {
		if (ctx->getShadersManager()) {
			throw std::invalid_argument("Context already has a shaders manager!");
		}

		impl = std::make_unique<Impl>();

		impl->ctx = ctx;

		for (int i = 0; i < 3; i++) {
			impl->shaders[SHADER_TYPE(i)] = std::unique_ptr<Shader>(new Shader(ctx, SHADER_TYPE(i)));
		}
	}

	ShadersManager::~ShadersManager() = default;

	Shader* ShadersManager::getShader(SHADER_TYPE shaderType) {
		return impl->shaders[shaderType].get();
	}

	Shader* ShadersManager::getActive() {
		return impl->activeShader;
	}

	void ShadersManager::setActive(SHADER_TYPE type) {
		Shader* shader = getShader(type);
		shader->activate();
	}

	void ShadersManager::setActive(Shader* shader) {
		if (shader->impl->ctx == impl->ctx) {
			shader->activate();
		}
	}
}
