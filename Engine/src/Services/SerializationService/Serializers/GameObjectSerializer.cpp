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

namespace Yngin::Services {
	bool SerializationService::serialize(std::ostream& out, GameObjectsManager* input, bool includeComponentDependencies) {
		return serialize(out, input->getRootGameObject(), -1, includeComponentDependencies);
	}

	bool SerializationService::serialize(std::ostream& out, GameObject* obj, int childrenDepth, bool includeComponentDependencies) {
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
		if (includeComponentDependencies) {
			components.push_back((Components::Component*)obj->getComponent<Components::Mesh>());
			components.push_back((Components::Component*)obj->getComponent<Components::BoxCollider>());
			components.push_back((Components::Component*)obj->getComponent<Components::RigidBody>());
			components.push_back((Components::Component*)obj->getComponent<Components::PointLight>());
			components.push_back((Components::Component*)obj->getComponent<Components::DirectionalLight>());

			std::erase(components, nullptr);
		}
		header.componentsCount = components.size();

		s.write(reinterpret_cast<const char*>(&header), op.headerSize);

		if (!serialize(s, obj->meta)) return false;

		for (auto& component : components) {
			if (!serialize(s, component, includeComponentDependencies)) return false;
		}

		for (auto& child : children) {
			if (!serialize(s, child, childrenDepth - 1, includeComponentDependencies)) return false;
		}

		op.dataSize = s.view().size();

		out.write(reinterpret_cast<const char*>(&op), sizeof(OperationData));
		out << s.rdbuf();
		return out.good();
	}

	bool SerializationService::serialize(std::ostream& out, Components::Component* comp, bool includeDependencies) {
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

			if (includeDependencies) {
				// TODO: make sure models and textures are serialized once per one serialization command

				Model* model = Service::impl->ctx->getModelsManager()->getModel(meshData.modelId);
				if (model != nullptr) {
					if (!serialize(depend, model)) return false;
				}

				Texture* texture = Service::impl->ctx->getTexturesManager()->getTexture(meshData.textureId);
				if (texture != nullptr) {
					if (!serialize(depend, texture)) return false;
				}
			}

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
			generic.type = S_COMPONENT_TYPE::RIGID_BODY;
			{
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

		s << depend.rdbuf();
		op.dataSize = s.view().size();

		out.write(reinterpret_cast<const char*>(&op), sizeof(OperationData));
		out << s.rdbuf();
		return out.good();
	}
}
