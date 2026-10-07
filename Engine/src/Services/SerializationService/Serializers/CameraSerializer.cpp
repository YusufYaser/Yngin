#include <Yngin/Services/SerializationService.h>
#include <Yngin/Rendering/Cameras.h>
#include <Yngin/Core/Scenes.h>
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

		if (!s.view().empty()) out << s.rdbuf();
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
		if (!s.view().empty()) out << s.rdbuf();
		return out.good();
	}

	DESERIALIZATION_STATUS SerializationService::Impl::validateCamera(std::istream& in, const Serialization::OperationData& op) {
		SerializedCameraData header;

		if (!streamCheck(in, op.headerSize, sizeof(header))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		if (header.id == -1) return DESERIALIZATION_STATUS::INVALID_DATA;

		DESERIALIZATION_STATUS metaStatus;
		if ((metaStatus = validateOperation(in, Operation::META)) != DESERIALIZATION_STATUS::OK) return metaStatus;

		return DESERIALIZATION_STATUS::OK;
	}

	DESERIALIZATION_STATUS SerializationService::Impl::deserializeCamera(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx) {
		Scene* scene = dsctx.user.scene;
		if (scene == nullptr || scene->getContext() != ctx) {
			// Skip the operation data in case we ignore missing context errors
			if (!streamCheck(in, op.dataSize, -1)) return DESERIALIZATION_STATUS::GENERIC_ERROR;
			in.seekg(op.dataSize, std::ios::cur);
			return DESERIALIZATION_STATUS::MISSING_CONTEXT;
		}

		SerializedCameraData header{};

		if (!streamCheck(in, op.headerSize, sizeof(header))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		if (scene->getCamerasManager()->getCamera(header.id)) {
			if (!dsctx.user.overrideConflictingId) {
				// Skip the operation data in case we ignore conflicting id errors
				if (streamCheck(in, op.dataSize - op.headerSize, -1)) return DESERIALIZATION_STATUS::GENERIC_ERROR;
				in.seekg(op.dataSize - op.headerSize, std::ios::cur);
				return DESERIALIZATION_STATUS::CONFLICTING_ID;
			}
		}

		Camera* camera = scene->getCamerasManager()->createCamera(header.id, dsctx.user.overrideConflictingId);
		if (camera == nullptr) return DESERIALIZATION_STATUS::GENERIC_ERROR;

		camera->setPosition(header.position);
		camera->setOrientation(header.orientation);
		camera->setFov(header.fov);
		camera->setWeight(header.weight);

		dsctx.meta = &camera->meta;
		DESERIALIZATION_STATUS metaStatus = deserializeOperation(in, dsctx, Operation::META);
		dsctx.meta = nullptr;
		if (metaStatus != DESERIALIZATION_STATUS::OK) return metaStatus;

		return DESERIALIZATION_STATUS::OK;
	}
}
