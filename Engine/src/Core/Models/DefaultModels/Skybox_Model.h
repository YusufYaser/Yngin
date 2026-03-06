#pragma once
#include <Yngin/Core/Models.h>

namespace Yngin {
	namespace DefaultModels {
		const ModelData skybox = {
			{
				Vertex{glm::vec3(0.5f, 0.5f,  -0.5f),  glm::vec2(2.0 / 4, 2.0 / 3)},
				Vertex{glm::vec3(-0.5f, 0.5f,  -0.5f),  glm::vec2(3.0 / 4, 2.0 / 3)},
				Vertex{glm::vec3(-0.5f,  0.5f,  0.5f),  glm::vec2(3.0 / 4, 1.0 / 3)},
				Vertex{glm::vec3(0.5f,  0.5f,  0.5f),  glm::vec2(2.0 / 4, 1.0 / 3)},

				Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0, 2.0 / 3)},
				Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0 / 4, 2.0 / 3)},
				Vertex{glm::vec3(0.5f,  -0.5f, 0.5f), glm::vec2(1.0 / 4, 1.0 / 3)},
				Vertex{glm::vec3(-0.5f,  -0.5f, 0.5f), glm::vec2(0, 1 / 3.0)},

				Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0 / 4, 2.0 / 3)},
				Vertex{glm::vec3(0.5f, 0.5f,  -0.5f), glm::vec2(2.0 / 4, 2.0 / 3)},
				Vertex{glm::vec3(0.5f,  0.5f,  0.5f), glm::vec2(2.0 / 4, 1.0 / 3)},
				Vertex{glm::vec3(0.5f,  -0.5f, 0.5f), glm::vec2(1.0 / 4, 1.0 / 3)},

				Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(3.0 / 4, 2.0 / 3)},
				Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1, 2.0 / 3)},
				Vertex{glm::vec3(-0.5f,  -0.5f, 0.5f), glm::vec2(1, 1.0 / 3)},
				Vertex{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(3.0 / 4, 1.0 / 3)},

				Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(2.0 / 4, 1)},
				Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(2.0 / 4, 2.0 / 3)},
				Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0 / 4, 2.0 / 3)},
				Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0 / 4, 1)},

				Vertex{glm::vec3(0.5f,  0.5f,  0.5f), glm::vec2(2.0 / 4, 1.0 / 3)},
				Vertex{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(2.0 / 4, 0)},
				Vertex{glm::vec3(-0.5f,  -0.5f, 0.5f), glm::vec2(1.0 / 4, 0)},
				Vertex{glm::vec3(0.5f,  -0.5f, 0.5f), glm::vec2(1.0 / 4, 1.0 / 3)}
		   },

		   {
			   0, 1, 2, 0, 2, 3,
			   4, 5, 6, 4, 6, 7,
			   8, 9, 10, 8, 10, 11,
			   12, 13, 14, 12, 14, 15,
			   16, 17, 18, 16, 18, 19,
			   20, 21, 22, 20, 22, 23
		   },

			MODEL_FRONT_FACE::CW
		};
	}
}
