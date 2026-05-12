#include <Yngin/Rendering/Renderer.h>
#include "Renderer_Internal.h"
#include <Yngin/Core/Scenes.h>
#include <Yngin/Core/Models.h>
#include <Yngin/Core/Materials.h>
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
#include "../../Core/Models/Models_Internal.h"
#include <Yngin/Core/GameObject.h>
#include "../../Components/Components_Internal.h"
#include <Yngin/Components/Component.h>
#include <Yngin/Components/Mesh.h>
#include "../../UI/Elements/UI_Elements_Internal.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

#define MAX_LIGHTS 32

namespace Yngin::Rendering {
	Renderer::Renderer(Context* ctx) {
		impl = std::make_unique<Impl>();
		impl->ctx = ctx;

		GLint maxBlockSize = 0;
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxBlockSize);

		size_t maxUnitSize = std::max(sizeof(InstanceVertexOffset), sizeof(InstanceFragmentOffset));

		impl->maxInstances = static_cast<size_t>(maxBlockSize) / maxUnitSize;

		impl->ssboSize = SSBO_GROW_UNIT;

		glGenBuffers(1, &impl->vertexSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, impl->vertexSSBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, impl->vertexSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, impl->ssboSize * sizeof(InstanceVertexOffset), nullptr, GL_DYNAMIC_DRAW);

		glGenBuffers(1, &impl->fragSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, impl->fragSSBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, impl->fragSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, impl->ssboSize * sizeof(InstanceFragmentOffset), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	Renderer::~Renderer() {
		glDeleteBuffers(1, &impl->vertexSSBO);
		impl->vertexSSBO = 0;
		glDeleteBuffers(1, &impl->fragSSBO);
		impl->fragSSBO = 0;
	}

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
			skybox->impl->render();
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

		preparingInstances = true;
		worldShader->setInt("instancing", true);
		render(scene->impl->gameObjectsManager->getRootGameObject(), -1);
		preparingInstances = false;
		for (auto& [meshTexPair, data] : instancesPrep) {
			uint32_t* a = new uint32_t[256];
			memset(a, 0, sizeof(a));
			renderSubmeshInstanced(meshTexPair.first, meshTexPair.second, data, a);
			delete[] a;
			a = nullptr;
		}
		worldShader->setInt("instancing", false);
		instancesPrep.clear();

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
		if (tex == nullptr) tex = cimpl->ctx->getTexturesManager()->getTexture(2);

		GameObject* obj = cimpl->gameObject;

		static const glm::mat4 i(1.0f);

		glm::vec3 rot = obj->getRotation();

		glm::mat4 modelMat =
			glm::translate(i, obj->getPosition()) *
			glm::yawPitchRoll(rot.y, rot.x, rot.z) *
			glm::scale(i, obj->getScale());

		Shader* worldShader = cimpl->ctx->getShadersManager()->getShader(SHADER_TYPE::WORLD);
		worldShader->activate();

		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMat)));

		bool isLight = obj->getComponent<Components::Light>() != nullptr || !lightingEnabled;

		if (!preparingInstances) {
			worldShader->setMat4("uModel", modelMat);
			worldShader->setMat3("uNormalMatrix", normalMatrix);

			worldShader->setInt("uIsLight", isLight);

			worldShader->setVec4("uColor", glm::vec4(mimpl->color, 1));

			if (mimpl->meshMaterialsCount == 0) {
				worldShader->setVec3("uMaterial.ambientColor", scene->getLightSettings().ambientLight);
				worldShader->setVec3("uMaterial.diffuseColor", glm::vec3(1.0f));
				worldShader->setVec3("uMaterial.specularColor", glm::vec3(1.0f));
				worldShader->setFloat("uMaterial.specularComponent", 64);
			}

			if (tex) tex->activate();
			model->impl->renderWithMaterials(mimpl->materials);
		} else {
			for (auto& submesh : model->impl->submeshes) {
				InstancePrepData& data = instancesPrep[{submesh.get(), tex}];

				int instance = data.instances;
				if (instance < maxInstances) {
					InstanceVertexOffset vOffset{};
					InstanceFragmentOffset fOffset{};

					vOffset.model = modelMat;
					vOffset.normalMatrix = normalMatrix;
					fOffset.isLight = isLight;
					fOffset.color = glm::vec4(mimpl->color, 1);
					if (submesh->matId < 256) {
						Material* mat = ctx->getMaterialsManager()->getMaterial(mimpl->materials[submesh->matId]);
						if (mat) {
							fOffset.material.ambientColor = mat->getAmbientColor();
							fOffset.material.diffuseColor = mat->getDiffuseColor();
							fOffset.material.specularColor = mat->getSpecularColor();
							fOffset.material.specularComponent = mat->getSpecularComponent();
						}
					}

					data.vOffsets.push_back(vOffset);
					data.fOffsets.push_back(fOffset);
				}

				data.instances++;

				if (data.instances >= maxInstances) {
					renderSubmeshInstanced(submesh.get(), tex, data, mimpl->materials);
					instancesPrep.erase({ submesh.get(), tex });
				}
			}
		}
	}

	void Renderer::Impl::renderSubmeshInstanced(InternalSubmesh* submesh, Texture* tex, InstancePrepData& data, const uint32_t materialsMap[256]) {
		ctx->makeCurrent();

		Model* model = submesh->model;
		const ModelData& modelData = model->getModelData();

		if (modelData.frontFace == MODEL_FRONT_FACE::NONE) {
			glDisable(GL_CULL_FACE);
		} else {
			glEnable(GL_CULL_FACE);
			if (modelData.frontFace == MODEL_FRONT_FACE::CW) {
				glFrontFace(GL_CW);
			} else {
				glFrontFace(GL_CCW);
			}
		}

		Shader* worldShader = ctx->getShadersManager()->getShader(SHADER_TYPE::WORLD);
		worldShader->activate();

		if (data.instances > ssboSize) {
			// Resize by 64
			ssboSize += ((data.instances - ssboSize + (SSBO_GROW_UNIT - 1)) & ~(SSBO_GROW_UNIT - 1));

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);
			glBufferData(GL_SHADER_STORAGE_BUFFER, ssboSize * sizeof(InstanceVertexOffset), nullptr, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, fragSSBO);
			glBufferData(GL_SHADER_STORAGE_BUFFER, ssboSize * sizeof(InstanceFragmentOffset), nullptr, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, data.vOffsets.size() * sizeof(InstanceVertexOffset), data.vOffsets.data());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, fragSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, data.fOffsets.size() * sizeof(InstanceFragmentOffset), data.fOffsets.data());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		tex->activate();

		glBindVertexArray(submesh->VAO);

		glDrawElementsInstanced(GL_TRIANGLES, submesh->indicesCount, GL_UNSIGNED_INT, 0, data.instances);
		glBindVertexArray(0);
	}
}
