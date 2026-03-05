#pragma once
#include <Yngin/Core/Models.h>

namespace Yngin {
	namespace DefaultModels {
		const ModelData square = {
			{
				Vertex{glm::vec3(+0.5f, +0.5f, 0.0f), glm::vec2(1.0f, 0.0f)},
				Vertex{glm::vec3(-0.5f, +0.5f, 0.0f), glm::vec2(0.0f, 0.0f)},
				Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 1.0f)},
				Vertex{glm::vec3(+0.5f, -0.5f, 0.0f), glm::vec2(1.0f, 1.0f)},
		   },

		   {
			   0, 1, 2, 0, 2, 3
		   }
		};
	}
}
