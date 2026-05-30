#pragma once
#include <Yngin/Rendering/Shaders.h>

namespace Yngin {
	namespace ShaderSources {
#ifndef __INTELLISENSE__
		const ShaderSource world = {
#include "Sources/World/World.vert"
			,
#include "Sources/World/World.frag"
		};

		const ShaderSource ui = {
#include "Sources/UI/UI.vert"
			,
#include "Sources/UI/UI.frag"
		};


		const ShaderSource skybox = {
#include "Sources/Skybox/Skybox.vert"
			,
#include "Sources/Skybox/Skybox.frag"
		};


		const ShaderSource depth = {
#include "Sources/Depth/Depth.vert"
		};
#else
		const ShaderSource world = {};
		const ShaderSource ui = {};
		const ShaderSource skybox = {};
		const ShaderSource depth = {};
#endif
	}
}
