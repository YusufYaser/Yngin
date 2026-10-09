#include <Yngin/Services/SerializationService.h>
#include <Yngin/Core/GameObject.h>
#include <Yngin/Components/Components.h>
#include <Yngin/Core/Models.h>
#include <Yngin/Rendering/Textures.h>
#include "../SerializationService_Internal.h"
#include "../../Services_Internal.h"
#include "SerializationStructs.h"
#include <sstream>
#include <Yngin/Core/Scenes.h>

using namespace Yngin::Services::Serialization;
using namespace Yngin::Components;

namespace Yngin::Services {
	bool SerializationService::serialize(std::ostream& out, GameObjectsManager* input) {
		return serialize(out, input->getRootGameObject(), -1);
	}

	bool SerializationService::serialize(std::ostream& out, GameObject* obj, int childrenDepth) {
		if (impl->shouldSkip(obj->meta)) return true;

		std::stringstream s;

		OperationData op{};
		op.schemaVersion = schemaVersion;
		op.op = Operation::GAMEOBJECT;
		op.headerSize = sizeof(SerializedGameObjectData);

		SerializedGameObjectData header{};
		header.id = obj->getId();
		if (obj->getParent()) header.parent = obj->getParent()->getId();
		header.position = obj->getPosition();
		header.rotation = obj->getRotation();
		header.scale = obj->getScale();

		std::vector<GameObject*> children{};

		if (childrenDepth != 0) {
			children = obj->getChildren();
			for (auto& child : children) {
				if (!impl->shouldSkip(child->meta)) header.childrenCount++;
			}
		}

		std::vector<Components::Component*> components{};
		components.push_back((Components::Component*)obj->getComponent<Components::Mesh>());
		components.push_back((Components::Component*)obj->getComponent<Components::BoxCollider>());
		components.push_back((Components::Component*)obj->getComponent<Components::RigidBody>());
		components.push_back((Components::Component*)obj->getComponent<Components::PointLight>());
		components.push_back((Components::Component*)obj->getComponent<Components::DirectionalLight>());

		std::erase(components, nullptr);
		header.componentsCount = components.size();

		s.write(reinterpret_cast<const char*>(&header), op.headerSize);

		if (!serialize(s, obj->meta)) return false;

		for (auto& component : components) {
			if (!serialize(s, component)) return false;
		}

		for (auto& child : children) {
			if (!serialize(s, child, childrenDepth - 1)) return false;
		}

		op.dataSize = s.view().size();

		out.write(reinterpret_cast<const char*>(&op), sizeof(OperationData));
		if (!s.view().empty()) out << s.rdbuf();
		return out.good();
	}

	DESERIALIZATION_STATUS SerializationService::Impl::validateGameObject(std::istream& in, const Serialization::OperationData& op) {
		SerializedGameObjectData header{};

		if (!streamCheck(in, op.headerSize, sizeof(header))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		if (header.id == -1) return DESERIALIZATION_STATUS::INVALID_DATA;

		DESERIALIZATION_STATUS metaStatus;
		if ((metaStatus = validateOperation(in, Operation::META)) != DESERIALIZATION_STATUS::OK) return metaStatus;

		for (uint8_t i = 0; i < header.componentsCount; i++) {
			DESERIALIZATION_STATUS status;
			if ((status = validateOperation(in, Operation::COMPONENT)) != DESERIALIZATION_STATUS::OK) return status;
		}

		for (uint32_t i = 0; i < header.childrenCount; i++) {
			DESERIALIZATION_STATUS status;
			if ((status = validateOperation(in, Operation::GAMEOBJECT)) != DESERIALIZATION_STATUS::OK) return status;
		}

		return DESERIALIZATION_STATUS::OK;
	}

	DESERIALIZATION_STATUS SerializationService::Impl::deserializeGameObject(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx) {
		if (dsctx.scene.empty() || dsctx.scene.top() == nullptr || dsctx.scene.top()->getContext() != ctx) {
			// Skip the operation data in case we ignore missing context errors
			if (!streamCheck(in, op.dataSize, -1)) return DESERIALIZATION_STATUS::GENERIC_ERROR;
			in.seekg(op.dataSize, std::ios::cur);
			return DESERIALIZATION_STATUS::MISSING_CONTEXT;
		}

		Scene* scene = dsctx.scene.top();

		SerializedGameObjectData header{};

		if (!streamCheck(in, op.headerSize, sizeof(header))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		if (!dsctx.user.overrideConflictingId && scene->getGameObjectsManager()->getGameObject(header.id)) {
			// Skip the operation data in case we ignore conflicting id errors
			if (!streamCheck(in, op.dataSize - op.headerSize, -1)) return DESERIALIZATION_STATUS::GENERIC_ERROR;
			in.seekg(op.dataSize - op.headerSize, std::ios::cur);
			return DESERIALIZATION_STATUS::CONFLICTING_ID;
		}

		GameObject* obj = scene->getGameObjectsManager()->createGameObject(header.id, dsctx.user.overrideConflictingId);
		if (obj == nullptr && header.id == 0) obj = scene->getGameObjectsManager()->getGameObject(0);
		if (obj == nullptr) return DESERIALIZATION_STATUS::GENERIC_ERROR;

		obj->setParent(header.parent);
		obj->setPosition(header.position);
		obj->setRotation(header.rotation);
		obj->setScale(header.scale);

		dsctx.meta.push(&obj->meta);
		DESERIALIZATION_STATUS metaStatus = deserializeOperation(in, dsctx, Operation::META);
		dsctx.meta.pop();
		if (metaStatus != DESERIALIZATION_STATUS::OK) return metaStatus;

		dsctx.gameObject.push(obj);
		for (uint8_t i = 0; i < header.componentsCount; i++) {
			DESERIALIZATION_STATUS status;
			if ((status = deserializeOperation(in, dsctx, Operation::COMPONENT)) != DESERIALIZATION_STATUS::OK) return status;
		}
		dsctx.gameObject.pop();

		for (uint32_t i = 0; i < header.childrenCount; i++) {
			DESERIALIZATION_STATUS status;
			if ((status = deserializeOperation(in, dsctx, Operation::GAMEOBJECT)) != DESERIALIZATION_STATUS::OK) return status;
		}

		return DESERIALIZATION_STATUS::OK;
	}

	bool SerializationService::serialize(std::ostream& out, Components::Component* comp) {
		std::stringstream s;
		std::stringstream depend;

		OperationData op{};
		op.schemaVersion = schemaVersion;
		op.op = Operation::COMPONENT;
		op.headerSize = sizeof(GenericComponentHeader);

		GenericComponentHeader generic{};

		switch (comp->getType()) {
		case COMPONENT_TYPE::MESH:
		{
			generic.type = S_COMPONENT_TYPE::MESH;
			generic.headerSize = sizeof(MeshComponentData);

			s.write(reinterpret_cast<const char*>(&generic), op.headerSize);

			Components::Mesh* mesh = dynamic_cast<Components::Mesh*>(comp);
			MeshComponentData meshData{};
			meshData.modelId = mesh->getModel();
			meshData.textureId = mesh->getTexture();
			meshData.color = mesh->getColor();

			for (int i = 0; i < 256; i++) {
				meshData.materials[i] = mesh->getMaterial(i);
			}

			s.write(reinterpret_cast<const char*>(&meshData), generic.headerSize);

			break;
		}

		case COMPONENT_TYPE::POINT_LIGHT:
		{
			generic.type = S_COMPONENT_TYPE::POINT_LIGHT;
			generic.headerSize = sizeof(PointLightData);

			s.write(reinterpret_cast<const char*>(&generic), op.headerSize);

			Components::PointLight* pointLight = dynamic_cast<Components::PointLight*>(comp);
			PointLightData pointLightData{};
			pointLightData.intensity = pointLight->getIntensity();
			pointLightData.distance = pointLight->getDistance();
			pointLightData.color = pointLight->getColor();

			s.write(reinterpret_cast<const char*>(&pointLightData), generic.headerSize);

			break;
		}

		case COMPONENT_TYPE::DIRECTIONAL_LIGHT:
		{
			generic.type = S_COMPONENT_TYPE::DIRECTIONAL_LIGHT;
			generic.headerSize = sizeof(DirectionalLightData);

			s.write(reinterpret_cast<const char*>(&generic), op.headerSize);

			Components::DirectionalLight* dirLight = dynamic_cast<Components::DirectionalLight*>(comp);
			DirectionalLightData dirLightData{};
			dirLightData.intensity = dirLight->getIntensity();
			dirLightData.color = dirLight->getColor();

			s.write(reinterpret_cast<const char*>(&dirLightData), generic.headerSize);

			break;
		}

		case COMPONENT_TYPE::RIGID_BODY:
		{
			generic.type = S_COMPONENT_TYPE::RIGID_BODY;
			generic.headerSize = sizeof(RigidBodyData);

			s.write(reinterpret_cast<const char*>(&generic), op.headerSize);

			Components::RigidBody* rigidBody = dynamic_cast<Components::RigidBody*>(comp);
			RigidBodyData rigidBodyData{};
			rigidBodyData.mass = rigidBody->getMass();
			rigidBodyData.velocity = rigidBody->getVelocity();

			s.write(reinterpret_cast<const char*>(&rigidBodyData), generic.headerSize);

			break;
		}

		case COMPONENT_TYPE::BOX_COLLIDER:
		{
			generic.type = S_COMPONENT_TYPE::BOX_COLLIDER;
			generic.headerSize = sizeof(BoxColliderData);

			s.write(reinterpret_cast<const char*>(&generic), op.headerSize);

			Components::BoxCollider* boxCollider = dynamic_cast<Components::BoxCollider*>(comp);
			BoxColliderData boxColliderData{};
			boxColliderData.offset = boxCollider->getOffset();
			boxColliderData.size = boxCollider->getSize();

			s.write(reinterpret_cast<const char*>(&boxColliderData), generic.headerSize);

			break;
		}

		default:
			return false;
		}

		if (!depend.view().empty()) s << depend.rdbuf();
		op.dataSize = s.view().size();

		out.write(reinterpret_cast<const char*>(&op), sizeof(OperationData));
		if (!s.view().empty()) out << s.rdbuf();
		return out.good();
	}

	DESERIALIZATION_STATUS SerializationService::Impl::validateComponent(std::istream& in, const Serialization::OperationData& op) {
		GenericComponentHeader header{};

		if (!streamCheck(in, op.headerSize, sizeof(header))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		switch (header.type) {
		case S_COMPONENT_TYPE::MESH:
			if (!streamCheck(in, header.headerSize, sizeof(MeshComponentData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.seekg(header.headerSize, std::ios::cur);
			break;

		case S_COMPONENT_TYPE::POINT_LIGHT:
			if (!streamCheck(in, header.headerSize, sizeof(PointLightData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.seekg(header.headerSize, std::ios::cur);
			break;

		case S_COMPONENT_TYPE::DIRECTIONAL_LIGHT:
			if (!streamCheck(in, header.headerSize, sizeof(DirectionalLightData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.seekg(header.headerSize, std::ios::cur);
			break;

		case S_COMPONENT_TYPE::RIGID_BODY:
			if (!streamCheck(in, header.headerSize, sizeof(RigidBodyData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.seekg(header.headerSize, std::ios::cur);
			break;

		case S_COMPONENT_TYPE::BOX_COLLIDER:
			if (!streamCheck(in, header.headerSize, sizeof(BoxColliderData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.seekg(header.headerSize, std::ios::cur);
			break;

		default:
			return DESERIALIZATION_STATUS::INVALID_DATA;
		}

		return DESERIALIZATION_STATUS::OK;
	}

	DESERIALIZATION_STATUS SerializationService::Impl::deserializeComponent(std::istream& in, const Serialization::OperationData& op, InternalDeserializationContext& dsctx) {
		if (dsctx.gameObject.empty() || dsctx.gameObject.top() == nullptr || dsctx.gameObject.top()->getContext() != ctx) {
			// Skip the operation data in case we ignore missing context errors
			if (!streamCheck(in, op.dataSize, -1)) return DESERIALIZATION_STATUS::GENERIC_ERROR;
			in.seekg(op.dataSize, std::ios::cur);
			return DESERIALIZATION_STATUS::MISSING_CONTEXT;
		}

		GameObject* obj = dsctx.gameObject.top();

		GenericComponentHeader header{};

		if (!streamCheck(in, op.headerSize, sizeof(header))) return DESERIALIZATION_STATUS::INVALID_DATA;
		in.read(reinterpret_cast<char*>(&header), op.headerSize);

		switch (header.type) {
		case S_COMPONENT_TYPE::MESH:
		{
			MeshComponentData meshData{};
			if (!streamCheck(in, header.headerSize, sizeof(meshData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(&meshData), header.headerSize);

			Mesh* mesh = obj->createComponent<Mesh>();
			if (mesh == nullptr) return DESERIALIZATION_STATUS::GENERIC_ERROR;
			mesh->setModel(meshData.modelId);
			mesh->setTexture(meshData.textureId);
			mesh->setColor(meshData.color);
			for (int i = 0; i < 256; i++) {
				mesh->setMaterial(i, meshData.materials[i]);
			}

			break;
		}

		case S_COMPONENT_TYPE::POINT_LIGHT:
		{
			PointLightData pointLightData{};
			if (!streamCheck(in, header.headerSize, sizeof(pointLightData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(&pointLightData), header.headerSize);

			PointLight* pointLight = obj->createComponent<PointLight>();
			if (pointLight == nullptr) return DESERIALIZATION_STATUS::GENERIC_ERROR;
			pointLight->setIntensity(pointLightData.intensity);
			pointLight->setDistance(pointLightData.distance);
			pointLight->setColor(pointLightData.color);

			break;
		}

		case S_COMPONENT_TYPE::DIRECTIONAL_LIGHT:
		{
			DirectionalLightData directionalLightData{};
			if (!streamCheck(in, header.headerSize, sizeof(directionalLightData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(&directionalLightData), header.headerSize);

			DirectionalLight* directionalLight = obj->createComponent<DirectionalLight>();
			if (directionalLight == nullptr) return DESERIALIZATION_STATUS::GENERIC_ERROR;
			directionalLight->setIntensity(directionalLightData.intensity);
			directionalLight->setColor(directionalLightData.color);

			break;
		}

		case S_COMPONENT_TYPE::RIGID_BODY:
		{
			RigidBodyData rigidBodyData{};
			if (!streamCheck(in, header.headerSize, sizeof(rigidBodyData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(&rigidBodyData), header.headerSize);

			RigidBody* rigidBody = obj->createComponent<RigidBody>();
			if (rigidBody == nullptr) return DESERIALIZATION_STATUS::GENERIC_ERROR;
			rigidBody->setMass(rigidBodyData.mass);
			rigidBody->setVelocity(rigidBodyData.velocity);

			break;
		}

		case S_COMPONENT_TYPE::BOX_COLLIDER:
		{
			BoxColliderData boxColliderData{};
			if (!streamCheck(in, header.headerSize, sizeof(boxColliderData))) return DESERIALIZATION_STATUS::INVALID_DATA;
			in.read(reinterpret_cast<char*>(&boxColliderData), header.headerSize);

			BoxCollider* boxCollider = obj->createComponent<BoxCollider>();
			if (boxCollider == nullptr) return DESERIALIZATION_STATUS::GENERIC_ERROR;
			boxCollider->setSize(boxColliderData.size);
			boxCollider->setOffset(boxColliderData.offset);

			break;
		}

		default:
			return DESERIALIZATION_STATUS::INVALID_DATA;
		}

		return DESERIALIZATION_STATUS::OK;
	}
}
