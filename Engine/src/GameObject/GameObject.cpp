#include <Yngin/GameObject.h>
#include <Yngin/Models.h>
#include "GameObject_Internal.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Yngin {
	GameObject::GameObject(Context* ctx, Scene* scene, GameObject* parent) {
		impl = std::make_unique<Impl>();

		if (parent != nullptr) {
			impl->id = scene->getGameObjectsManager()->acquireId();
			scene->getGameObjectsManager()->impl->gameObjects[impl->id] = this;
		} else {
			impl->id = 0;
		}
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
		impl->childs[childId].reset();
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

	void GameObject::render() {
		glm::mat4 model = glm::mat4(1.0f);

		model = glm::translate(model, impl->pos);

		GLuint shaderId = impl->ctx->getShaderId();
		glUseProgram(shaderId);
		GLuint modelLoc = glGetUniformLocation(shaderId, "model");

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		// for now we'll render a test model
		impl->ctx->getModelsManager()->render(0);

		for (auto& kvp : impl->childs) {
			kvp.second->render();
		}
	}
}
