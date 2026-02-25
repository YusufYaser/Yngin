#include <Yngin/Core/GameObject.h>
#include <Yngin/Core/Models.h>
#include <Yngin/Renderer/Textures.h>
#include <Yngin/Components/Mesh.h>
#include "GameObject_Internal.h"
#include <type_traits>
#include <assert.h>


namespace Yngin {
	using namespace Components;

	GameObject::GameObject(Context* ctx, Scene* scene, GameObject* parent) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
		impl->scene = scene;
		impl->parent = parent;
	}

	GameObject::~GameObject() = default;

	uint32_t GameObject::getId() {
		return impl->id;
	}

	GameObject* GameObject::getParent() {
		return impl->parent;
	}

	void GameObject::setParent(GameObject* newParent) {
		impl->parent->moveChild(impl->id, newParent);
	}

	GameObject* GameObject::getChild(uint32_t childId) {
		auto it = impl->childs.find(childId);
		assert(it != impl->childs.end());

		if (it == impl->childs.end()) return nullptr;

		return it->second.get();
	}

	uint32_t GameObject::createChild() {
		auto gameObject = std::unique_ptr<GameObject>(new GameObject(impl->ctx, impl->scene, this));

		uint32_t id = gameObject->getId();

		impl->childs[id] = std::move(gameObject);

		return id;
	}

	void GameObject::deleteChild(uint32_t childId) {
		impl->childs.erase(childId);
	}

	void GameObject::moveChild(uint32_t childId, GameObject* newParent) {
		newParent->impl->childs[childId] = std::move(impl->childs[childId]);
		impl->childs.erase(childId);
	}

	glm::vec3 GameObject::getPos() {
		return impl->pos;
	}

	void GameObject::setPos(glm::vec3 newPos) {
		impl->pos = newPos;
	}

	glm::vec3 GameObject::getRotation() {
		return impl->rotation;
	}

	void GameObject::setRotation(glm::vec3 newRotation) {
		impl->rotation = newRotation;
	}

	template<typename T>
	inline T* GameObject::getComponent() {
		static_assert(std::is_base_of<Component, T>::value, "Type must be a component class");
		auto it = impl->components.find(std::type_index(typeid(T)));

		if (it == impl->components.end()) return nullptr;

		return dynamic_cast<T*>(it->second.get());
	}

	template<typename T>
	void GameObject::createComponent() {
		static_assert(std::is_base_of<Component, T>::value, "Type must be a component class");
		if (getComponent<T>() != nullptr) {
			return;
		}

		auto component = std::unique_ptr<T>(new T(impl->ctx, this));

		impl->components[std::type_index(typeid(T))] = std::move(component);
	}

	template<typename T>
	void GameObject::deleteComponent() {
		static_assert(std::is_base_of<Component, T>::value, "Type must be a component class");
		impl->components.erase(std::type_index(typeid(T)));
	}

	template void GameObject::createComponent<Components::Mesh>();
	template Components::Mesh* GameObject::getComponent<Components::Mesh>();

	void GameObject::render() {
		for (auto& kvp : impl->components) {
			kvp.second->onRender();
		}

		for (auto& kvp : impl->childs) {
			kvp.second->render();
		}
	}
}
