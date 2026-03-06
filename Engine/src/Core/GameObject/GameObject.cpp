#include <Yngin/Core/GameObject.h>
#include <Yngin/Core/Models.h>
#include <Yngin/Renderer/Textures.h>
#include <Yngin/Components/Mesh.h>
#include "GameObject_Internal.h"
#include <type_traits>
#include <assert.h>
#include <Yngin/Core/Scenes.h>
#include <Yngin/Services/Tween.h>

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

	Context* GameObject::getContext() {
		return impl->ctx;
	}

	Scene* GameObject::getScene() {
		return impl->scene;
	}

	GameObject* GameObject::getParent() {
		return impl->parent;
	}

	void GameObject::setParent(uint32_t newParentId) {
		GameObject* newParent = impl->scene->getGameObjectsManager()->getGameObject(newParentId);
		if (newParent) setParent(newParent);
	}

	void GameObject::setParent(GameObject* newParent) {
		impl->parent->moveChild(impl->id, newParent);
	}

	GameObject* GameObject::getChild(uint32_t childId) {
		auto it = impl->childs.find(childId);

		if (it == impl->childs.end()) return nullptr;

		return it->second.get();
	}

	GameObject* GameObject::createChild() {
		auto gameObject = std::unique_ptr<GameObject>(new GameObject(impl->ctx, impl->scene, this));

		uint32_t id = impl->scene->getGameObjectsManager()->acquireId();
		gameObject->impl->id = id;

		impl->childs[id] = std::move(gameObject);
		GameObject* obj = impl->childs[id].get();
		impl->scene->getGameObjectsManager()->impl->gameObjects[id] = obj;

		return obj;
	}

	void GameObject::deleteChild(uint32_t childId) {
		GameObject* child = getChild(childId);
		if (child) deleteChild(child);
	}

	void GameObject::deleteChild(GameObject* child) {
		uint32_t childId = child->getId();
		impl->childs.erase(childId);
		impl->scene->getGameObjectsManager()->impl->gameObjects.erase(childId);
	}

	void GameObject::moveChild(uint32_t childId, GameObject* newParent) {
		assert(newParent->getContext() == impl->ctx);
		assert(newParent->getScene() == impl->scene);

		if (newParent->getContext() != impl->ctx) return;
		if (newParent->getScene() != impl->scene) return;

		auto it = impl->childs.find(childId);
		if (it == impl->childs.end()) return;

		newParent->impl->childs[childId] = std::move(it->second);
		impl->childs.erase(childId);
	}

	void GameObject::moveChild(GameObject* child, GameObject* newParent) {
		moveChild(child->getId(), newParent);
	}

	void GameObject::moveChild(GameObject* child, uint32_t newParentId) {
		GameObject* newParent = impl->scene->getGameObjectsManager()->getGameObject(newParentId);
		if (newParent) moveChild(child->getId(), newParent);
	}

	void GameObject::moveChild(uint32_t childId, uint32_t newParentId) {
		GameObject* newParent = impl->scene->getGameObjectsManager()->getGameObject(newParentId);
		if (newParent) moveChild(childId, newParent);
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
	T* GameObject::createComponent() {
		static_assert(std::is_base_of<Component, T>::value, "Type must be a component class");

		assert(getComponent<T>() == nullptr);
		if (getComponent<T>() != nullptr) {
			return nullptr;
		}

		auto component = std::unique_ptr<T>(new T(this));

		impl->components[std::type_index(typeid(T))] = std::move(component);

		return getComponent<T>();
	}

	template<typename T>
	void GameObject::deleteComponent() {
		static_assert(std::is_base_of<Component, T>::value, "Type must be a component class");
		impl->components.erase(std::type_index(typeid(T)));
	}

	template void GameObject::deleteComponent<Components::Mesh>();
	template Components::Mesh* GameObject::createComponent<Components::Mesh>();
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
