#include <Yngin/Services/SerializationService.h>
#include <Yngin/Rendering/Cameras.h>
#include "../SerializationService_Internal.h"
#include "SerializationStructs.h"
#include <sstream>

using namespace Yngin::Services::Serialization;

namespace Yngin::Services {
	bool SerializationService::serialize(std::ostream& out, CamerasManager* input) {
		input->getContext()->makeCurrent();
		auto items = input->getCameras();

		std::stringstream s;

		for (auto& item : items) {
			if (!serialize(s, item)) return false;
		}

		out << s.rdbuf();
		return out.good();
	}

	bool SerializationService::serialize(std::ostream& out, Camera* camera) {
		if (impl->shouldSkip(camera->meta)) return true;

		std::stringstream s;

		OperationData op{};
		op.schemaVersion = schemaVersion;
		op.op = Operation::CAMERA;
		op.headerSize = sizeof(SerializedCameraData);

		SerializedCameraData header{};
		header.id = camera->getId();
		header.position = camera->getPosition();
		header.orientation = camera->getOrientation();
		header.fov = camera->getFov();
		header.weight = camera->getWeight();

		s.write(reinterpret_cast<const char*>(&header), op.headerSize);

		if (!serialize(s, camera->meta)) return false;

		op.dataSize = s.view().size();

		out.write(reinterpret_cast<const char*>(&op), sizeof(OperationData));
		out << s.rdbuf();
		return out.good();
	}

	bool SerializationService::Impl::validateCamera(std::istream& in, const Serialization::OperationData& op) {
		SerializedCameraData header;

		if (!streamCheck(in, op.headerSize)) return false;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		if (header.id == -1) return false;

		if (!validateOperation(in, Operation::META)) return false;

		return true;
	}
}
