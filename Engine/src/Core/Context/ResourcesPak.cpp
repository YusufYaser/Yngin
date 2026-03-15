#include <Yngin/Core/Context.h>
#include "ResourcesPak.h"
#include "Context_Internal.h"
#include <Yngin/Core/Models.h>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

#define R(name, type) s.read(reinterpret_cast<char*>(&name), sizeof(type))
#define W(name, type) s.write(reinterpret_cast<const char*>(&name), sizeof(type))

using namespace Yngin::GameData;
using namespace Yngin::GameData::ResourcesPak;

namespace Yngin {
	void Context::loadResourcesPak(const char* bytes, size_t size) {
		std::istringstream s(std::string(bytes, size), std::ios::binary);

		Header header{};
		R(header, Header);
		if (std::memcmp(header.magic, "YNGINRESOURCES", 14) != 0) return;
		if (header.version != VERSION) return;

		Operation op{};

		while (R(op, Operation)) {
			switch (op.op) {
			case OP::MODEL:
			{
				PakModelData pakModelData{};
				R(pakModelData, PakModelData);

				ModelData modelData{};
				modelData.frontFace = Yngin::MODEL_FRONT_FACE(pakModelData.frontFace);

				for (int i = 0; i < pakModelData.verticesCount; i++) {
					ModelVertexData v{};
					R(v, ModelVertexData);
					modelData.vertices.push_back(Vertex{
						.pos = glm::make_vec3(v.position),
						.texCoord = glm::make_vec2(v.texCoord),
						.normal = glm::make_vec3(v.normal)
						});
				}

				for (int i = 0; i < pakModelData.indicesCount; i++) {
					uint32_t index = 0;
					R(index, uint32_t);
					modelData.indices.push_back(index);
				}

				Model* model = impl->modelsManager->createModel(modelData, pakModelData.id, true);
				break;
			}
			}
		}
	}

	std::vector<char> Context::generateResourcesPak() {
		std::ostringstream s(std::ios::binary);

		Header header = {};
		Operation op = {};

		std::memcpy(header.magic, "YNGINRESOURCES", 14);
		header.version = VERSION;

		W(header, Header);

		auto models = impl->modelsManager->getModels();

		for (auto model : models) {
			op.op = OP::MODEL;
			W(op, Operation);

			const ModelData& data = model->getModelData();

			PakModelData pakModelData{};
			pakModelData.id = model->getId();
			pakModelData.frontFace = PAK_MODEL_FRONT_FACE(data.frontFace);
			pakModelData.verticesCount = data.vertices.size();
			pakModelData.indicesCount = data.indices.size();

			W(pakModelData, PakModelData);

			for (auto& vertex : data.vertices) {
				ModelVertexData v{};
				std::memcpy(v.position, glm::value_ptr(vertex.pos), sizeof(float) * 3);
				std::memcpy(v.texCoord, glm::value_ptr(vertex.texCoord), sizeof(float) * 2);
				std::memcpy(v.normal, glm::value_ptr(vertex.normal), sizeof(float) * 3);
				W(v, ModelVertexData);
			}

			for (auto& index : data.indices) {
				W(index, uint32_t);
			}
		}

		std::string_view sv = s.view();
		return std::vector<char>(sv.begin(), sv.end());
	}
}
