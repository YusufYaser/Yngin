#pragma once
#include <Yngin/Rendering/Shaders.h>

#ifndef __INTELLISENSE__
namespace Yngin {
	namespace ShaderSources {
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
			,
#include "Sources/Depth/Depth.frag"
		};
	}
}
#endif
