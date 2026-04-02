#include <Yngin/Core/GameObject.h>
#include <Yngin/Core/Models.h>
#include <Yngin/Rendering/Textures.h>
#include <Yngin/Components/Mesh.h>
#include "GameObject_Internal.h"
#include <type_traits>
#include <assert.h>
#include <Yngin/Core/Scenes.h>
#include <Yngin/Services/Tween.h>

#define DEFINE_FOR_COMPONENT(T) \
template void GameObject::deleteComponent<T>(); \
template T* GameObject::createComponent<T>(); \
template T* GameObject::getComponent<T>() const;

namespace Yngin {
	using namespace Components;

	GameObject::GameObject(Context* ctx, Scene* scene, GameObject* parent) {
		impl = std::make_unique<Impl>();

		impl->ctx = ctx;
		impl->scene = scene;
		impl->parent = parent;
	}

	GameObject::~GameObject() = default;

	uint32_t GameObject::getId() const {
		return impl->id;
	}

	Context* GameObject::getContext() const {
		return impl->ctx;
	}

	Scene* GameObject::getScene() const {
		return impl->scene;
	}

	GameObject* GameObject::getParent() const {
		return impl->parent;
	}

	void GameObject::setParent(uint32_t newParentId) {
		if (impl->parent->getId() == newParentId) return;
		GameObject* newParent = impl->scene->getGameObjectsManager()->getGameObject(newParentId);
		if (newParent) setParent(newParent);
	}

	void GameObject::setParent(GameObject* newParent) {
		impl->parent->moveChild(impl->id, newParent);
	}

	std::vector<GameObject*> GameObject::getChildren() const {
		std::vector<GameObject*> children;
		for (auto& kvp : impl->childs) {
			children.push_back(kvp.second.get());
		}
		return children;
	}

	GameObject* GameObject::getChild(uint32_t childId) const {
		auto it = impl->childs.find(childId);

		if (it == impl->childs.end()) return nullptr;

		return it->second.get();
	}

	GameObject* GameObject::createChild() {
		return createChild(impl->scene->getGameObjectsManager()->impl->nextId);
	}

	GameObject* GameObject::createChild(uint32_t id, bool override) {
		if (id == 0) return nullptr;

		GameObjectsManager* mgr = impl->scene->getGameObjectsManager();

		if (mgr->getGameObject(id) != nullptr) {
			if (override) {
				mgr->deleteGameObject(id);
			} else {
				return nullptr;
			}
		}

		auto gameObject = std::unique_ptr<GameObject>(new GameObject(impl->ctx, impl->scene, this));

		uint32_t nextId = mgr->impl->nextId;
		mgr->impl->nextId = std::max(nextId, id + 1);
		gameObject->impl->id = id;

		impl->childs[id] = std::move(gameObject);
		GameObject* obj = impl->childs[id].get();
		mgr->impl->gameObjects[id] = obj;

		return obj;
	}

	void GameObject::deleteChild(uint32_t childId) {
		GameObject* child = getChild(childId);
		if (child) deleteChild(child);
	}

	void GameObject::deleteChild(GameObject* child) {
		for (auto& obj : child->getChildren()) {
			child->deleteChild(obj);
		}
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

		it->second->impl->parent = newParent;

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

	glm::vec3 GameObject::getPosition() const {
		return impl->pos;
	}

	void GameObject::setPosition(glm::vec3 newPos) {
		impl->pos = newPos;
	}

	glm::vec3 GameObject::getRotation() const {
		return impl->rotation;
	}

	void GameObject::setRotation(glm::vec3 newRotation) {
		impl->rotation = newRotation;
	}

	void GameObject::setScale(glm::vec3 newScale) {
		impl->scale = newScale;
	}

	glm::vec3 GameObject::getScale() const {
		return impl->scale;
	}

	template<typename T>
	inline T* GameObject::getComponent() const {
		auto it = impl->components.find(std::type_index(typeid(T)));

		if (it == impl->components.end()) return nullptr;

		return dynamic_cast<T*>(it->second.get());
	}

	template<typename T>
	T* GameObject::createComponent() {
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
		impl->components.erase(std::type_index(typeid(T)));
	}

	DEFINE_FOR_COMPONENT(Components::Component);
	DEFINE_FOR_COMPONENT(Components::Mesh);
	DEFINE_FOR_COMPONENT(Components::Light);
	DEFINE_FOR_COMPONENT(Components::BoxCollider);
	DEFINE_FOR_COMPONENT(Components::RigidBody);
}
