#include <Yngin/Core/Models.h>
#include "Models_Internal.h"
#include <string>
#include <sstream>
#include <glm/vec4.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <tuple>
#include <fstream>
#include <cstring>
#include <filesystem>

#define VERTEX_STATEMENT 1

namespace fs = std::filesystem;

namespace Yngin {
	void ModelsManager::Impl::loadObj(const char* data, size_t length, ModelData& d) {
		// not all obj features are implemented yet

		d.frontFace = MODEL_FRONT_FACE::CCW;

		std::string str(data, length);
		std::stringstream stream(str);

		std::string l;

		// pos, texCoord, normal: vertex id
		std::map<std::tuple<int, int, int, int>, int> verticesCache;

		std::map<int, glm::vec3> positions;
		std::map<int, glm::vec2> texCoords;
		std::map<int, glm::vec3> normals;

		std::map<std::string, uint8_t> matIds;
		uint32_t currentMtl = 0;

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
				// TODO: calculate normal if there are no normals in model file
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
							if (sn > 2) {
								sn = 2; // temporary fix
							}
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
					auto it = verticesCache.find(std::tuple<int, int, int, int>(s[0], s[1], s[2], currentMtl));
					if (it == verticesCache.end()) {
						Vertex vertex{};
						vertex.pos = positions[s[0]];
						if (s[1] != 0)
							vertex.texCoord = texCoords[s[1]];
						if (s[2] != 0)
							vertex.normal = normals[s[2]];

						vertex.matId = currentMtl;

						vid = nVertexId++;
						d.vertices.push_back(vertex);
						verticesCache[std::tuple<int, int, int, int>(s[0], s[1], s[2], currentMtl)] = vid;
					} else {
						vid = it->second;
					}

					d.indices.push_back(vid);
				}
			} else if (cmd == "mtllib") {
				std::string filename;
				s >> filename;

				fs::path path(filename);
				filename = path.filename().string();

				std::ifstream file(filename);

				if (file.is_open()) {
					std::stringstream fileData;
					fileData << file.rdbuf();

					file.close();

					uint32_t oldModelMatsCount = d.materialsCount;
					uint32_t oldMatsNextId = nextMaterialId;

					loadMtl(fileData.str().c_str(), fileData.str().size(), d, matIds);

					for (int i = oldModelMatsCount; i < d.materialsCount; i++) {
						d.defaultMaterials[i] = oldMatsNextId + (i - oldModelMatsCount);
					}
				} else {
					printf("[Yngin] Failed to load model material from file %s\n", filename.c_str());
				}
			} else if (cmd == "usemtl") {
				std::string v;
				s >> v;
				auto it = matIds.find(v);
				if (it != matIds.end()) {
					currentMtl = it->second;
				} else {
					currentMtl = 0;
				}
			}
		}
	}

	int ModelsManager::Impl::loadMtl(const char* data, size_t length, ModelData& d, std::map<std::string, uint8_t>& matIds) {
		std::string str(data, length);
		std::stringstream stream(str);

		std::string l;

		uint8_t currentId = 0;

		int count = 0;

		while (std::getline(stream, l)) {
			if (l.empty()) continue;

			std::stringstream s(l);

			std::string cmd;
			s >> cmd;

			if (cmd[0] == '#') continue;

			if (cmd == "newmtl") {
				std::string v;
				s >> v;
				if (nextId != std::numeric_limits<uint8_t>::max()) {
					currentId = nextMaterialId++;
					matIds[v] = d.materialsCount++;
					materials[currentId] = Material{};
					count++;
				}
			} else if (cmd == "Ka") {
				glm::vec3 v;
				s >> v.x >> v.y >> v.z;

				materials[currentId].ambientColor = v;
			} else if (cmd == "Kd") {
				glm::vec3 v;
				s >> v.x >> v.y >> v.z;

				materials[currentId].diffuseColor = v;
			} else if (cmd == "Ks") {
				glm::vec3 v;
				s >> v.x >> v.y >> v.z;

				materials[currentId].specularColor = v;
			} else if (cmd == "Ns") {
				float v;
				s >> v;

				materials[currentId].specularComponent = v;
			}

			// TODO: implement other commands
		}

		return count;
	}
}