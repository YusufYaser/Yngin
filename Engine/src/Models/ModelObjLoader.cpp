#include <Yngin/Models.h>
#include "Models_Internal.h"
#include <string>
#include <sstream>
#include <glm/vec4.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <tuple>

#define VERTEX_STATEMENT 1

namespace Yngin {
	void ModelsManager::Impl::loadObj(const char* data, size_t length, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
		// not all obj features are implemented yet

		std::string str(data, length);
		std::stringstream stream(str);

		std::string l;

		// pos, texCoord, normal: vertex id
		std::map<std::tuple<int, int, int>, int> verticesCache;

		std::map<int, glm::vec3> positions;
		std::map<int, glm::vec2> texCoords;
		std::map<int, glm::vec3> normals;

		int nPosId = 1;
		int nTexCoordId = 1;
		int nNormalId = 1;

		int nVertexId = 0;

		while (std::getline(stream, l)) {
			if (l.empty()) continue;

			std::stringstream s(l);

			std::string cmd;
			s >> cmd;

			if (cmd[0] == '#') continue;

			if (cmd == "v") {
				glm::vec3 v;
				s >> v.x >> v.y >> v.z;

				positions[nPosId++] = v;
			} else if (cmd == "vt") {
				glm::vec2 v;
				s >> v.x >> v.y;

				texCoords[nTexCoordId++] = v;
			} else if (cmd == "vn") {
				glm::vec3 v;
				s >> v.x >> v.y >> v.z;

				v = glm::normalize(v);

				normals[nNormalId++] = v;
			} else if (cmd == "f") {
				// TODO: add support for more than 4 vertices
				std::string p[4];
				s >> p[0] >> p[1] >> p[2] >> p[3];

				int verticesCount = 3;
				if (p[3] != "")
					verticesCount = 4;

				for (int i = 0; i < 3 * (verticesCount - 2); i++) {
					std::string v = p[i % 3];
					if (i > 3) v = p[i % 3 + 1];

					int s[3] = {};
					int sn = 0;
					for (char c : v) {
						if (c == '/') {
							sn++;
							continue;
						}

						if (c >= '0' && c <= '9') {
							s[sn] *= 10;
							s[sn] += c - '0';
						}
					}

					if (s[0] >= nPosId) {
						break;
					}
					if (s[1] >= nTexCoordId)
						s[1] = 0;
					if (s[2] >= nNormalId)
						s[2] = 0;

					int vid = 0;
					auto it = verticesCache.find(std::tuple<int, int, int>(s[0], s[1], s[2]));
					if (it == verticesCache.end()) {
						Vertex vertex{};
						vertex.pos = positions[s[0]];
						if (s[1] != 0)
							vertex.texCoord = texCoords[s[1]];
						if (s[2] != 0)
							vertex.normal = normals[s[2]];

						vid = nVertexId++;
						vertices.push_back(vertex);
						verticesCache[std::tuple<int, int, int>(s[0], s[1], s[2])] = vid;
					} else {
						vid = it->second;
					}

					indices.push_back(vid);
				}
			}
		}
	}
}