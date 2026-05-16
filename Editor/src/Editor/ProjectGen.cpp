#include "Editor.h"
#include <filesystem>
#include <Yngin/Yngin.h>
#include "Cube_Model.h"
#include "DefaultScripts.h"
#include <fstream>

using namespace Yngin;
namespace fs = std::filesystem;

bool Editor::generateNewProject(std::string path) {
	if (!fs::exists(path) || !fs::is_empty(path)) return false;

	fs::path oldCwd = fs::current_path();
	fs::current_path(path);

	fs::create_directory("temp");
	fs::create_directory("bin");
	fs::create_directory("data");
	fs::create_directory("data/scenes");

	Context* ctx = new Context();


	Scene* activeScene = ctx->getScenesManager()->createScene(0, true);

	Model* cubeModel = ctx->getModelsManager()->createModel(cubeModelData);
	cubeModel->meta.setMeta("Editor.Name", "Cube");

	GameObject* defaultCube = activeScene->getGameObjectsManager()->getRootGameObject()->createChild();
	Components::Mesh* defaultCubeMesh = defaultCube->createComponent<Components::Mesh>();
	defaultCubeMesh->setModel(cubeModel);
	defaultCubeMesh->setTexture(2);
	defaultCube->meta.setMeta("Editor.Name", "Cube");

	fs::current_path(oldCwd);
	Texture* skyboxTex = ctx->getTexturesManager()->createTexture("assets/default_skybox.png");
	fs::current_path(path);

	skyboxTex->meta.setMeta("Editor.Name", "Skybox");

	activeScene->setSkyboxTexture(skyboxTex);

	Camera* editorCamera = activeScene->getCamerasManager()->getCamera(0);
	editorCamera->setPosition(glm::vec3(2.0f));
	editorCamera->lookAt(glm::vec3());



	std::map<uint32_t, EditorScript> scripts;
	int nextScriptId = 0;
	for (auto& script : defaultScripts) {
		scripts[nextScriptId++] = EditorScript{
			.name = script.name,
			.scene = script.scene,
			.code = script.code
		};
	}

	Material* defaultMat = ctx->getMaterialsManager()->createMaterial(0, true);
	defaultMat->meta.setMeta("Editor.Name", "Default Material");

	defaultMat->setAmbientColor(glm::vec3(1.0f));
	defaultMat->setDiffuseColor(glm::vec3(1.0f));
	defaultMat->setSpecularColor(glm::vec3(1.0f));
	defaultMat->setSpecularComponent(64.0f);

	bool success = saveContext(ctx, scripts);

	fs::current_path(oldCwd);

	delete ctx;
	ctx = nullptr;
	return success;
}
