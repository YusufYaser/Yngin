#pragma once
#include <Yngin/Yngin.h>

namespace Yngin {
	class Model {
	public:
		Model(Context* ctx, std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		~Model();

		void render();

	private:
		Context* ctx;

		struct IDs;
		struct StructureInfo;
		std::unique_ptr<IDs> ids;
		std::unique_ptr<StructureInfo> structureInfo;
	};
}
