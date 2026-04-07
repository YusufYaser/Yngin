#include <Yngin/Rendering/Renderer.h>
#include "Renderer_Internal.h"
#include <Yngin/Core/Scenes.h>
#include <Yngin/Core/Models.h>
#include <Yngin/UI/Elements/UIElement.h>
#include <Yngin/UI/UIManager.h>
#include <Yngin/Rendering/Shaders.h>
#include <Yngin/Rendering/Cameras.h>
#include <Yngin/Rendering/Textures.h>
#include <glad/glad.h>
#include "../../Core/Scenes/Scenes_Internal.h"
#include "../Cameras/Cameras_Internal.h"
#include "../../Core/GameObject/GameObject_Internal.h"
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include "../../Core/Context/Context_Internal.h"
#include <Yngin/Core/GameObject.h>
#include "../../Components/Components_Internal.h"
#include <Yngin/Components/Component.h>
#include <Yngin/Components/Mesh.h>
#include "../../UI/Elements/UI_Elements_Internal.h"

#define MAX_LIGHTS 32

namespace Yngin::Rendering {
	Renderer::Renderer(Context* ctx) {
		impl = std::make_unique<Impl>();
		impl->ctx = ctx;
	}

	Renderer::~Renderer() = default;

	Context* Renderer::getContext() const {
		return impl->ctx;
	}

	bool Renderer::isLightingEnabled() const {
		return impl->lightingEnabled;
	}

	void Renderer::setLightingEnabled(bool enabled) {
		impl->lightingEnabled = enabled;
	}

	float Renderer::getRenderDistance() const {
		return impl->renderDistance;
	}

	void Renderer::setRenderDistance(float renderDistance) {
		impl->renderDistance = renderDistance;
	}

	void Renderer::Impl::render(Scene* scene) {
		ctx->makeCurrent();

		glm::vec3 camPos = scene->impl->camerasManager->getFinalPos();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = scene->impl->camerasManager->impl->getFinalPerspectiveProjection();
		glm::mat4 view = scene->impl->camerasManager->impl->getFinalView();

		Model* skybox = ctx->getInternalModelsManager()->getModel(INTERNAL_MODEL_SKYBOX_ID);
		Texture* skyboxTex = ctx->getTexturesManager()->getTexture(scene->impl->skyboxTexId);
		if (skyboxTex && skybox) {
			Shader* skyboxShader = ctx->getShadersManager()->getShader(SHADER_TYPE::SKYBOX);
			skyboxShader->activate();

			skyboxShader->setMat4("projection", proj);
			skyboxShader->setMat4("view", glm::mat4(glm::mat3(view)));

			skyboxTex->activate();
			skybox->render();
			glClear(GL_DEPTH_BUFFER_BIT);
		}

		Shader* worldShader = ctx->getShadersManager()->getShader(SHADER_TYPE::WORLD);
		worldShader->activate();

		worldShader->setMat4("projection", proj);
		worldShader->setMat4("view", view);

		if (lightingEnabled) {
			// register lights
			int lightsCount = 0;
			for (auto& kvp : scene->impl->gameObjectsManager->impl->gameObjects) {
				GameObject* obj = kvp.second;

				glm::vec3 delta = obj->impl->pos - camPos;
				float distSq = glm::dot(delta, delta);

				if (distSq > renderDistance * renderDistance) continue;

				Components::Light* light = obj->getComponent<Components::Light>();

				if (light == nullptr) continue;

				worldShader->setVec3(std::string("lights[" + std::to_string(lightsCount) + "].position").c_str(), obj->getPosition());
				worldShader->setVec3(std::string("lights[" + std::to_string(lightsCount) + "].color").c_str(), light->getColor());
				worldShader->setFloat(std::string("lights[" + std::to_string(lightsCount) + "].distance").c_str(), light->getDistance());
				worldShader->setFloat(std::string("lights[" + std::to_string(lightsCount) + "].intensity").c_str(), light->getIntensity());

				lightsCount++;

				if (lightsCount >= MAX_LIGHTS) {
					break;
				}
			}

			worldShader->setInt("lightsCount", lightsCount);
			worldShader->setVec3("scene.ambientLight", scene->impl->lightSettings.ambientLight);
		}

		worldShader->setVec3("cameraPos", scene->impl->camerasManager->getFinalPos());

		render(scene->impl->gameObjectsManager->getRootGameObject(), -1);
		render(scene->impl->uiManager->getRootElement(), -1);
	}

	void Renderer::Impl::render(GameObject* gameObject, int renderChildren) {
		glm::vec3 camPos = gameObject->impl->scene->impl->camerasManager->getFinalPos();
		glm::vec3 delta = gameObject->impl->pos - camPos;
		float distSq = glm::dot(delta, delta);

		if (distSq <= renderDistance * renderDistance) {
			for (auto& kvp : gameObject->impl->components) {
				if (kvp.first == typeid(Components::Mesh)) {
					render(dynamic_cast<Components::Mesh*>(kvp.second.get()));
				}
				kvp.second->onRender();
			}
		}

		if (renderChildren != 0) {
			for (auto& kvp : gameObject->impl->childs) {
				render(kvp.second.get(), renderChildren - 1);
			}
		}
	}

	void Renderer::Impl::render(UI::UIElement* element, int renderChildren) {
		// UI Elements handle their own rendering logic
		element->render();

		if (renderChildren != 0) {
			for (auto& kvp : element->impl->childs) {
				render(kvp.second.get(), renderChildren - 1);
			}
		}
	}

	void Renderer::Impl::render(Components::Mesh* mesh) {
		auto cimpl = dynamic_cast<Components::Component*>(mesh)->impl.get();
		auto mimpl = mesh->impl.get();

		Scene* scene = cimpl->gameObject->impl->scene;

		Model* model = cimpl->ctx->getModelsManager()->getModel(mimpl->modelId);
		if (model == nullptr) return;

		Texture* tex = cimpl->ctx->getTexturesManager()->getTexture(mimpl->texId);
		if (tex == nullptr) cimpl->ctx->getTexturesManager()->getTexture(0);

		GameObject* obj = cimpl->gameObject;

		glm::mat4 modelMat = glm::mat4(1.0f);

		modelMat = glm::translate(modelMat, obj->getPosition());
		modelMat = glm::rotate(modelMat, obj->getRotation().x, glm::vec3(1, 0, 0));
		modelMat = glm::rotate(modelMat, obj->getRotation().y, glm::vec3(0, 1, 0));
		modelMat = glm::rotate(modelMat, obj->getRotation().z, glm::vec3(0, 0, 1));
		modelMat = glm::scale(modelMat, obj->getScale());

		Shader* worldShader = cimpl->ctx->getShadersManager()->getShader(SHADER_TYPE::WORLD);
		worldShader->activate();

		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMat)));

		worldShader->setMat3("normalMatrix", normalMatrix);
		worldShader->setMat4("model", modelMat);

		worldShader->setInt("isLight", obj->getComponent<Components::Light>() != nullptr || !lightingEnabled);

		worldShader->setVec4("color", glm::vec4(mimpl->color, 1));

		worldShader->setVec3(std::string("materials[0].ambientColor").c_str(), scene->getLightSettings().ambientLight);
		worldShader->setVec3(std::string("materials[0].diffuseColor").c_str(), glm::vec3(1.0f));
		worldShader->setVec3(std::string("materials[0].specularColor").c_str(), glm::vec3(1.0f));
		worldShader->setFloat(std::string("materials[0].specularComponent").c_str(), 64);

		auto materials = cimpl->ctx->getModelsManager()->getMaterials();

		for (int i = 0; i < mimpl->meshMaterialsCount; i++) {
			uint32_t matId = mimpl->materials[i];
			const Material& mat = materials[matId];

			std::string idStr = std::to_string(i);

			worldShader->setVec3(std::string("materials[" + idStr + "].ambientColor").c_str(), mat.ambientColor);
			worldShader->setVec3(std::string("materials[" + idStr + "].diffuseColor").c_str(), mat.diffuseColor);
			worldShader->setVec3(std::string("materials[" + idStr + "].specularColor").c_str(), mat.specularColor);
			worldShader->setFloat(std::string("materials[" + idStr + "].specularComponent").c_str(), mat.specularComponent);
		}

		if (tex) tex->activate();
		model->render();
	}
}
