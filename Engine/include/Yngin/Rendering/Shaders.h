#pragma once
#include <Yngin/Forward.h>
#include <memory>
#include <glm/mat4x4.hpp>
#include <Yngin/Utils/Meta.h>

namespace Yngin {
	enum class SHADER_TYPE : uint8_t {
		NONE = 0,
		WORLD,
		SKYBOX,
		UI,
		DEPTH
	};

	struct ShaderSource {
		const char* vertex;
		const char* fragment;
	};

	class ShadersManager {
	public:
		Shader* getShader(const SHADER_TYPE& shaderType) const;

		Shader* getActive() const;
		void setActive(const SHADER_TYPE& shaderType);
		void setActive(Shader* shader);

	private:
		friend class Context;
		friend struct std::default_delete<ShadersManager>;
		friend class Shader;

		ShadersManager(Context* ctx);
		~ShadersManager();

		struct Impl;
		std::unique_ptr<Impl> impl;
	};

	class Shader {
	public:
		SHADER_TYPE getType() const;
		Context* getContext() const;

		Meta meta;

		bool setSource(const ShaderSource& src, bool hasFragment = true);
		void activate();

		void setMat3(const char* name, glm::mat3 v);
		void setMat4(const char* name, glm::mat4 v);

		void setFloat(const char* name, float v);
		void setInt(const char* name, int v);

		void setIVec2(const char* name, glm::ivec2 v);

		void setVec2(const char* name, glm::vec2 v);
		void setVec3(const char* name, glm::vec3 v);
		void setVec4(const char* name, glm::vec4 v);

	private:
		friend class ShadersManager;
		friend struct std::default_delete<Shader>;

		struct Impl;
		std::unique_ptr<Impl> impl;

		Shader(Context* ctx, SHADER_TYPE type);
		~Shader();
	};
}
