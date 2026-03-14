#pragma once
#include <Yngin/Core/Models.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

const Yngin::ModelData cubeModelData = {
	{
		Yngin::Vertex{glm::vec3(0.5f, 0.5f,  -0.5f),  glm::vec2(0, 1)},
		Yngin::Vertex{glm::vec3(-0.5f, 0.5f,  -0.5f),  glm::vec2(1, 1)},
		Yngin::Vertex{glm::vec3(-0.5f,  0.5f,  0.5f),  glm::vec2(1, 0)},
		Yngin::Vertex{glm::vec3(0.5f,  0.5f,  0.5f),  glm::vec2(0, 0)},

		Yngin::Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0, 1)},
		Yngin::Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1, 1)},
		Yngin::Vertex{glm::vec3(0.5f,  -0.5f, 0.5f), glm::vec2(1, 0)},
		Yngin::Vertex{glm::vec3(-0.5f,  -0.5f, 0.5f), glm::vec2(0, 0)},

		Yngin::Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0, 1)},
		Yngin::Vertex{glm::vec3(0.5f, 0.5f,  -0.5f), glm::vec2(1, 1)},
		Yngin::Vertex{glm::vec3(0.5f,  0.5f,  0.5f), glm::vec2(1, 0)},
		Yngin::Vertex{glm::vec3(0.5f,  -0.5f, 0.5f), glm::vec2(0, 0)},

		Yngin::Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0, 1)},
		Yngin::Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1, 1)},
		Yngin::Vertex{glm::vec3(-0.5f,  -0.5f, 0.5f), glm::vec2(1, 0)},
		Yngin::Vertex{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(0, 0)},

		Yngin::Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(1, 1)},
		Yngin::Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1, 0)},
		Yngin::Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0, 0)},
		Yngin::Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0, 1)},

		Yngin::Vertex{glm::vec3(0.5f,  0.5f,  0.5f), glm::vec2(1, 1)},
		Yngin::Vertex{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec2(1, 0)},
		Yngin::Vertex{glm::vec3(-0.5f,  -0.5f, 0.5f), glm::vec2(0, 0)},
		Yngin::Vertex{glm::vec3(0.5f,  -0.5f, 0.5f), glm::vec2(0, 1)}
	},

	{
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23
	},

	Yngin::MODEL_FRONT_FACE::CCW
};