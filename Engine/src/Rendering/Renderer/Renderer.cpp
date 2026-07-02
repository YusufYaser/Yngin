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
#include <glm/gtx/component_wise.hpp>
#include <limits>

#define LOGGER_NAME Renderer
#include "../../Internal/Logger.h"

namespace Yngin::Rendering {
	Renderer::Renderer(Context* ctx) {
		impl = std::make_unique<Impl>();
		impl->ctx = ctx;

		GLint maxBlockSize = 0;
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxBlockSize);

		size_t maxUnitSize = std::max(sizeof(InstanceVertexOffset), sizeof(InstanceFragmentOffset));

		impl->maxInstances = static_cast<size_t>(maxBlockSize) / maxUnitSize;
		impl->maxInstances = std::min(impl->maxInstances, std::numeric_limits<GLsizeiptr>::max() / sizeof(InstanceVertexOffset));

		DEBUG("Max instances supported by the system: %lld", impl->maxInstances);

		impl->ssboSize = SSBO_GROW_UNIT;

		glGenBuffers(1, &impl->vertexSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, impl->vertexSSBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, impl->vertexSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, impl->ssboSize * sizeof(InstanceVertexOffset), nullptr, GL_DYNAMIC_DRAW);

		glGenBuffers(1, &impl->fragSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, impl->fragSSBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, impl->fragSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, impl->ssboSize * sizeof(InstanceFragmentOffset), nullptr, GL_DYNAMIC_DRAW);

		glGenBuffers(1, &impl->lightsSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, impl->lightsSSBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, impl->lightsSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ShaderLightsSSBOData), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);




		glGenFramebuffers(1, &impl->FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, impl->FBO);

		//
		glGenTextures(1, &impl->colorsTex);
		glBindTexture(GL_TEXTURE_2D, impl->colorsTex);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, impl->colorsTex, 0);

		//
		glGenRenderbuffers(1, &impl->RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, impl->RBO);

		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, impl->RBO);

		//
		glGenTextures(1, &impl->IDsTex);
		glBindTexture(GL_TEXTURE_2D, impl->IDsTex);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 800, 600, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, impl->IDsTex, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);



		// Shadows

		glGenFramebuffers(1, &impl->shadowFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, impl->shadowFBO);

		glGenTextures(1, &impl->shadowMapsTex);
		glBindTexture(GL_TEXTURE_2D_ARRAY, impl->shadowMapsTex);

		glTexImage3D(
			GL_TEXTURE_2D_ARRAY,
			0,
			GL_DEPTH_COMPONENT16,
			SHADOW_MAP_WIDTH,
			SHADOW_MAP_HEIGHT,
			MAX_DIRECTIONAL_LIGHTS,
			0,
			GL_DEPTH_COMPONENT,
			GL_FLOAT,
			0
		);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		static const glm::vec4 white = glm::vec4(1.0f);
		glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(white));

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	Renderer::~Renderer() {
		glDeleteBuffers(1, &impl->vertexSSBO);
		impl->vertexSSBO = 0;
		glDeleteBuffers(1, &impl->fragSSBO);
		impl->fragSSBO = 0;
		glDeleteBuffers(1, &impl->lightsSSBO);
		impl->lightsSSBO = 0;

		glDeleteFramebuffers(1, &impl->FBO);
		impl->FBO = 0;
		glDeleteFramebuffers(1, &impl->RBO);
		impl->RBO = 0;
		glDeleteTextures(1, &impl->colorsTex);
		impl->colorsTex = 0;
		glDeleteTextures(1, &impl->IDsTex);
		impl->IDsTex = 0;
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

	size_t Renderer::getSubmeshesRendered() const {
		return impl->sceneSubmeshesRendered;
	}

	size_t Renderer::getSceneLightsCount() const {
		return impl->sceneLights;
	}

	uint32_t Renderer::getGameObjectId(glm::ivec2 pixel) {
		if (pixel.x < 0 || pixel.y < 0 || pixel.x >= impl->renderedViewportSize.x || pixel.y >= impl->renderedViewportSize.y) return 0;

		GLuint readId = 0;

		glBindFramebuffer(GL_FRAMEBUFFER, impl->FBO);
		glReadBuffer(GL_COLOR_ATTACHMENT1);

		glReadPixels(
			pixel.x, impl->renderedViewportSize.y - pixel.y, 1, 1,
			GL_RED_INTEGER,
			GL_UNSIGNED_INT,
			&readId
		);

		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return readId;
	}

	void Renderer::Impl::setFrustumPlanes(glm::mat4 viewProjection) {
		for (int i = 0; i < 6; i++) {
			Plane& p = frustumPlanes[i];

			int row = i / 2;

			for (int j = 0; j < 3; j++) {
				p.normal[j] = viewProjection[j][3] + viewProjection[j][row] * (i % 2 ? 1 : -1);
			}
			p.distance = viewProjection[3][3] + viewProjection[3][row] * (i % 2 ? 1 : -1);

			float length = glm::length(p.normal);
			p.normal /= length;
			p.distance /= length;
		}
	}

	void Renderer::Impl::render(Scene* scene) {
		ctx->makeCurrent();

		glm::vec3 camPos = scene->impl->camerasManager->getBlendedCamera()->getPosition();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = scene->impl->camerasManager->getBlendedCamera()->impl->getMergedProjection();
		glm::mat4 view = scene->impl->camerasManager->getBlendedCamera()->impl->getView();
		glm::mat4 viewProjection = proj * view;

		// This is a temporary workaround to fix objects hidden by frustum culling not casting shadows
		// This will be fixed in the future
		if (!lightingEnabled) {
			setFrustumPlanes(viewProjection);
		}

		glm::ivec2 viewportPos = ctx->getViewportPos();
		glm::ivec2 viewportSize = ctx->getViewportSize();
		renderedViewportSize = viewportSize;

		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		//
		glBindTexture(GL_TEXTURE_2D, colorsTex);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, viewportSize.x, viewportSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorsTex, 0);

		//
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);

		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, viewportSize.x, viewportSize.y);

		//

		glBindTexture(GL_TEXTURE_2D, IDsTex);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, viewportSize.x, viewportSize.y, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glViewport(0, 0, viewportSize.x, viewportSize.y);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Model* skybox = ctx->getInternalModelsManager()->getModel(INTERNAL_MODEL_SKYBOX_ID);
		Texture* skyboxTex = ctx->getTexturesManager()->getTexture(scene->impl->skyboxTexId);
		if (skyboxTex && skybox) {
			Shader* skyboxShader = ctx->getShadersManager()->getShader(SHADER_TYPE::SKYBOX);
			skyboxShader->activate();

			skyboxShader->setMat4("projection", scene->impl->camerasManager->getBlendedCamera()->impl->getPerspectiveProjection());
			skyboxShader->setMat4("view", glm::mat4(glm::mat3(view)));

			skyboxTex->activate();
			glDepthMask(GL_FALSE);
			skybox->impl->render();
			glDepthMask(GL_TRUE);
		}

		preparingInstances = true;
		sceneSubmeshesRendered = 0;
		// this function name isn't really accurate
		// this is just to prepare the instances
		render(scene->impl->gameObjectsManager->getRootGameObject(), -1);
		preparingInstances = false;

		Shader* depthShader = ctx->getShadersManager()->getShader(SHADER_TYPE::DEPTH);
		depthShader->activate();

		sceneLights = 0;
		if (lightingEnabled) {
			// register lights
			ShaderLightsSSBOData* lights = new ShaderLightsSSBOData();

			glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
			glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

			for (auto& [id, obj] : scene->impl->gameObjectsManager->impl->gameObjects) {
				if (lights->pointLightsCount < MAX_POINT_LIGHTS) {
					glm::vec3 delta = obj->impl->pos - camPos;
					float distSq = glm::dot(delta, delta);

					if (distSq <= renderDistance * renderDistance) {
						Components::PointLight* pointLight = obj->getComponent<Components::PointLight>();

						if (pointLight) {
							ShaderPointLight shaderLight{};

							shaderLight.position = obj->getPosition();
							shaderLight.color = pointLight->getColor();
							shaderLight.distance = pointLight->getDistance();
							shaderLight.intensity = pointLight->getIntensity();

							lights->pointLights[lights->pointLightsCount] = shaderLight;

							lights->pointLightsCount++;
						}
					}
				}

				if (lights->directionalLightsCount < MAX_DIRECTIONAL_LIGHTS) {
					Components::DirectionalLight* directionalLight = obj->getComponent<Components::DirectionalLight>();

					if (directionalLight) {
						ShaderDirectionalLight shaderLight{};
						shaderLight.color = directionalLight->getColor();
						shaderLight.intensity = directionalLight->getIntensity();

						glm::vec3 rads = obj->getRotation();

						glm::vec3 direction;
						direction.x = cos(rads.y) * cos(rads.x);
						direction.y = sin(rads.y) * cos(rads.x);
						direction.z = sin(rads.x);

						shaderLight.direction = glm::normalize(direction);

						float halfBoxSize = 100.0f;
						float unitsPerTexel = (halfBoxSize * 2.0f) / SHADOW_MAP_WIDTH;

						glm::vec3 center = camPos;
						center.x = std::floor(center.x / unitsPerTexel) * unitsPerTexel;
						center.y = std::floor(center.y / unitsPerTexel) * unitsPerTexel;
						center.z = std::floor(center.z / unitsPerTexel) * unitsPerTexel;

						glm::mat4 proj = glm::ortho(-halfBoxSize, halfBoxSize, -halfBoxSize, halfBoxSize, 0.1f, 1000.0f);
						glm::mat4 view = glm::lookAt(center - direction * 300.0f, center, { 0, 0, 1 });

						glm::mat4 lightVP = proj * view;

						shaderLight.viewProjection = lightVP;
						shaderLight.index = lights->directionalLightsCount++;

						lights->directionalLights[shaderLight.index] = shaderLight;

						depthShader->setMat4("viewProjection", lightVP);

						glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowMapsTex, 0, shaderLight.index);
						glClear(GL_DEPTH_BUFFER_BIT);

						for (auto& [meshTexPair, data] : instancesPrep) {
							renderSubmeshInstanced(meshTexPair.first, meshTexPair.second, data);
						}
					}
				}


				if (lights->pointLightsCount >= MAX_POINT_LIGHTS && lights->directionalLightsCount >= MAX_DIRECTIONAL_LIGHTS) {
					break;
				}
			}
			glBindFramebuffer(GL_FRAMEBUFFER, FBO);
			glViewport(0, 0, viewportSize.x, viewportSize.y);

			sceneLights = lights->pointLightsCount + lights->directionalLightsCount;

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsSSBO);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ShaderLightsSSBOData), lights);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			delete lights;
			lights = nullptr;
		}

		depthShader->setMat4("viewProjection", viewProjection);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		for (auto& [meshTexPair, data] : instancesPrep) {
			renderSubmeshInstanced(meshTexPair.first, meshTexPair.second, data);
		}


		Shader* worldShader = ctx->getShadersManager()->getShader(SHADER_TYPE::WORLD);
		worldShader->activate();

		worldShader->setVec3("scene.ambientLight", scene->impl->lightSettings.ambientLight);
		worldShader->setVec3("cameraPos", camPos);
		worldShader->setMat4("viewProjection", viewProjection);
		worldShader->setInt("instancing", true);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		GLenum colors[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, colors);

		for (auto& [meshTexPair, data] : instancesPrep) {
			renderSubmeshInstanced(meshTexPair.first, meshTexPair.second, data);
		}
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glm::ivec2 windowSize = ctx->getWindow()->getSize();

		glViewport(viewportPos.x, windowSize.y - viewportPos.y - viewportSize.y, viewportSize.x, viewportSize.y);

		glBlitFramebuffer(
			0, 0, viewportSize.x, viewportSize.y,
			viewportPos.x, windowSize.y - viewportPos.y - viewportSize.y, viewportPos.x + viewportSize.x, windowSize.y - viewportPos.y,
			GL_COLOR_BUFFER_BIT, GL_NEAREST
		);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		worldShader->setInt("instancing", false);
		instancesPrep.clear();

		render(scene->impl->uiManager->getRootElement(), -1);
	}

	void Renderer::Impl::render(GameObject* gameObject, int renderChildren) {
		glm::vec3 camPos = gameObject->impl->scene->impl->camerasManager->getBlendedCamera()->getPosition();
		glm::vec3 delta = gameObject->impl->pos - camPos;
		float distSq = glm::dot(delta, delta);

		if (distSq <= renderDistance * renderDistance) {
			Components::Mesh* mesh = gameObject->getComponent<Components::Mesh>();
			if (mesh) render(mesh);
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

		GameObject* obj = cimpl->gameObject;
		uint32_t objId = obj->getId();

		static const glm::mat4 i(1.0f);

		glm::vec3 pos = obj->getPosition();
		glm::vec3 rot = obj->getRotation();
		glm::vec3 scale = obj->getScale();

		if (obj->impl->updateMatrices) {
			obj->impl->modelMatrix =
				glm::translate(i, pos) *
				glm::yawPitchRoll(rot.y, rot.x, rot.z) *
				glm::scale(i, scale);

			obj->impl->normalMatrix = glm::transpose(glm::inverse(glm::mat3(obj->impl->modelMatrix)));

			obj->impl->updateMatrices = false;
		}

		Shader* worldShader = cimpl->ctx->getShadersManager()->getShader(SHADER_TYPE::WORLD);
		worldShader->activate();

		bool isLight = !lightingEnabled || obj->hasComponent<Components::PointLight>() || obj->hasComponent<Components::DirectionalLight>();

		if (!preparingInstances) {
			worldShader->setMat4("uModel", obj->impl->modelMatrix);
			worldShader->setMat3("uNormalMatrix", obj->impl->normalMatrix);

			worldShader->setInt("uIsLight", isLight);

			worldShader->setVec4("uColor", glm::vec4(mimpl->color, 1));

			if (mimpl->meshMaterialsCount == 0) {
				worldShader->setVec3("uMaterial.ambientColor", scene->getLightSettings().ambientLight);
				worldShader->setVec3("uMaterial.diffuseColor", glm::vec3(1.0f));
				worldShader->setVec3("uMaterial.specularColor", glm::vec3(1.0f));
				worldShader->setFloat("uMaterial.specularComponent", 64);
			}

			Texture* tex = cimpl->ctx->getTexturesManager()->getTexture(mimpl->texId);
			if (tex == nullptr) tex = cimpl->ctx->getTexturesManager()->getTexture(2);
			if (tex) tex->activate();
			model->impl->renderWithMaterials(mimpl->materials);
			sceneSubmeshesRendered += model->impl->submeshes.size();
		} else {
			for (auto& submesh : model->impl->submeshes) {
				// This is a temporary workaround to fix objects hidden by frustum culling not casting shadows
				// This will be fixed in the future
				if (!lightingEnabled) {
					float radius = submesh->radius * glm::compMax(scale);

					bool showing = true;
					glm::vec3 center = glm::mat3(obj->impl->modelMatrix) * submesh->center + glm::vec3(obj->impl->modelMatrix[3]);

					for (int i = 0; i < 6; i++) {
						if (glm::dot(frustumPlanes[i].normal, center) + frustumPlanes[i].distance < -radius) {
							showing = false;
							break;
						}
					}
					if (!showing) continue;
				}

				InstancePrepData& data = instancesPrep[{submesh.get(), mimpl->texId}];
				if (data.vOffsets.capacity() == 0) {
					data.vOffsets.reserve(512);
					data.fOffsets.reserve(512);
				}

				int instance = data.instances;
				if (instance < maxInstances) {
					InstanceVertexOffset vOffset{};
					InstanceFragmentOffset fOffset{};

					vOffset.model = obj->impl->modelMatrix;
					vOffset.normalMatrix = obj->impl->normalMatrix;

					fOffset.objectId = objId;
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

					data.instances++;

					sceneSubmeshesRendered++;
				}
			}
		}
	}

	void Renderer::Impl::renderSubmeshInstanced(InternalSubmesh* submesh, uint32_t texId, const InstancePrepData& data) {
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

		if (data.instances > ssboSize) {
			// Resize by 64
			size_t inc = ((data.instances - ssboSize + (SSBO_GROW_UNIT - 1)) & ~(SSBO_GROW_UNIT - 1));
			ssboSize += inc;
#ifdef _DEBUG
			size_t oldSSBOSize = ssboSize - inc;
			DEBUG("Increased SSBO instances size from %lld to %lld (+%lld)", oldSSBOSize, ssboSize, inc);
#endif

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

		Texture* tex = ctx->getTexturesManager()->getTexture(texId);
		if (tex == nullptr) tex = ctx->getTexturesManager()->getTexture(2);
		tex->activate();

		glBindVertexArray(submesh->VAO);

		glDrawElementsInstanced(GL_TRIANGLES, submesh->indicesCount, GL_UNSIGNED_INT, 0, data.instances);
		glBindVertexArray(0);
	}
}
