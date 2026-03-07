// I know this code isn't the best but I'll improve it soon
//
// I just quickly add code to test new features without
// checking the code style

#include <stdio.h>
#include <fstream>
#include <sstream>
#include <Yngin/Yngin.h>
#include <glm/glm.hpp>

using namespace Yngin;

int main() {
	Yngin::init();

	ContextSettings settings = {};
	settings.windowSettings = {
		.title = "Yngin Demo App"
	};

	Context* ctx = createContext(settings);

	printf("Context: %p\n", ctx);

	ModelsManager* modelsMgr = ctx->getModelsManager();
	ScenesManager* scenesManager = ctx->getScenesManager();
	Scene* scene = scenesManager->createScene();
	GameObjectsManager* gameObjMgr = scene->getGameObjectsManager();

	Window* window = ctx->getWindow();
	InputSystem* input = ctx->getInputSystem();

	scenesManager->setActive(scene->getId());

	std::ifstream modelFile("test_model");

	if (!modelFile.is_open()) {
		printf("Test model not found\n");
		Yngin::terminate();
		return 1;
	}

	std::stringstream modelFileData;
	modelFileData << modelFile.rdbuf();

	modelFile.close();

	Model* model = modelsMgr->createModel(MODEL_FILE_TYPE::OBJ, modelFileData.str().c_str(), modelFileData.str().length());

	std::ifstream cubeModelFile("cube_model");

	if (!cubeModelFile.is_open()) {
		printf("Cube model not found\n");
		Yngin::terminate();
		return 1;
	}

	std::stringstream cubeModelFileData;
	cubeModelFileData << cubeModelFile.rdbuf();

	cubeModelFile.close();

	Model* cubeModel = modelsMgr->createModel(MODEL_FILE_TYPE::OBJ, cubeModelFileData.str().c_str(), cubeModelFileData.str().length());

	CamerasManager* camerasManager = scene->getCamerasManager();

	Camera* defaultCamera = camerasManager->getCamera(0);

	defaultCamera->setPos({ 10, 0, 0 });
	defaultCamera->lookAt(glm::vec3());
	defaultCamera->setFov(90.0f);

	TexturesManager* texMgr = ctx->getTexturesManager();

	// https://freestylized.com/skybox/sky_36/
	Texture* skyboxTex = texMgr->createTexture("skybox.png", {
		.wrap = TEXTURE_WRAP::CLAMP,
		.filter = TEXTURE_FILTER::NEAREST
		});

	scene->setSkyboxTexture(skyboxTex);

	Texture* githubTex = texMgr->createTexture("github.png", {
		.wrap = TEXTURE_WRAP::CLAMP,
		.filter = TEXTURE_FILTER::LINEAR
		});

	TextureData texData{};
	texData.width = 2;
	texData.height = 2;
	texData.numCh = 1;
	texData.bytes = "\xff\x80\x80\xff";
	TextureSettings texSettings{};
	texSettings.wrap = TEXTURE_WRAP::REPEAT;
	texSettings.filter = TEXTURE_FILTER::NEAREST;
	Texture* tex = texMgr->createTexture(texData, texSettings);

	TextureData whiteTexData = {
		.width = 1,
		.height = 1,
		.numCh = 1,
		.bytes = "\xff"
	};
	Texture* whiteTex = texMgr->createTexture(whiteTexData);

	GameObject* obj = gameObjMgr->getRootGameObject()->createChild();
	Components::Mesh* mesh = obj->createComponent<Components::Mesh>();
	obj->setRotation({ glm::radians(90.0f), 0, 0 });

	mesh->setModel(cubeModel->getId());
	mesh->setTexture(tex->getId());

	std::vector<Vertex> wallVertices;
	std::vector<uint32_t> wallIndices = { 0, 1, 2, 0, 2, 3 };

	wallVertices.push_back({ glm::vec3(+0.5f, +0.5f, 0.0f), glm::vec2(100.0f, 000.0f), glm::vec3(0, 0, 1.0f) });
	wallVertices.push_back({ glm::vec3(-0.5f, +0.5f, 0.0f), glm::vec2(000.0f, 000.0f), glm::vec3(0, 0, 1.0f) });
	wallVertices.push_back({ glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(000.0f, 100.0f), glm::vec3(0, 0, 1.0f) });
	wallVertices.push_back({ glm::vec3(+0.5f, -0.5f, 0.0f), glm::vec2(100.0f, 100.0f), glm::vec3(0, 0, 1.0f) });

	ModelData wallModelData = { wallVertices, wallIndices, MODEL_FRONT_FACE::NONE };
	Model* wallModel = modelsMgr->createModel(wallModelData);

	GameObject* wall = gameObjMgr->getRootGameObject()->createChild();
	Components::Mesh* wallMesh = wall->createComponent<Components::Mesh>();
	wallMesh->setModel(wallModel->getId());
	wallMesh->setTexture(tex->getId());
	wallMesh->setScale(glm::vec3(1, 1, 1) * 100.0f);
	wall->setPos({ 0, 0, -5.0f });

	{
		GameObject* light = gameObjMgr->getRootGameObject()->createChild();
		Components::Mesh* mesh = light->createComponent<Components::Mesh>();
		mesh->setModel(cubeModel);
		mesh->setTexture(whiteTex);
		light->setPos({ 0, 0, 2 });

		Components::Light* lightComp = light->createComponent<Components::Light>();
		lightComp->setIntensity(2.0f);
		lightComp->setDistance(32.0f);
		lightComp->setColor({ 1.0f, 0.8f, 0.45f });

		mesh->setColor(lightComp->getColor());
	}
	{
		GameObject* light = gameObjMgr->getRootGameObject()->createChild();
		Components::Mesh* mesh = light->createComponent<Components::Mesh>();
		mesh->setModel(cubeModel);
		mesh->setTexture(whiteTex);
		light->setPos({ 0, 3.0f, -0.5f });

		Components::Light* lightComp = light->createComponent<Components::Light>();
		lightComp->setColor({ 1.0f, 0, 0 });
		lightComp->setIntensity(10.0f);

		mesh->setColor(lightComp->getColor());
	}

	{
		GameObject* light = gameObjMgr->getRootGameObject()->createChild();
		Components::Mesh* mesh = light->createComponent<Components::Mesh>();
		mesh->setModel(cubeModel);
		mesh->setTexture(whiteTex);
		light->setPos({ 3.0f, 0, 0.5f });

		Components::Light* lightComp = light->createComponent<Components::Light>();
		lightComp->setColor({ 0, 0, 1.0f });
		lightComp->setIntensity(10.0f);

		mesh->setColor(lightComp->getColor());
	}

	ctx->setMaxFPS(120);

	Services::Tween* tween = ctx->getService<Services::Tween>();

	Services::TweenSettings tweenSettings = {
		.duration = 1.0f,
		.function = Services::TWEEN_FUNCTION::EASE_INOUT
	};

	int tweenId = 0;
	int cycle = 0;

	UI::Image* github = scene->getUIManager()->getRootElement()->createChild<UI::Image>();
	github->setTexture(githubTex->getId());
	github->setPos({ 0, 48, 1.0f, -48 });
	github->setSize({ 0, 64, 0, 64 });

	glm::ivec2 oldMousePos = {};

	while (ctx->getStatus() == CONTEXT_STATUS::RUNNING) {
		ctx->makeCurrent();

		window->setCursorLocked(input->isMousePressed(MOUSE_BUTTON::RIGHT));

		if (!tween->isActive(tweenId)) {
			cycle = (cycle + 1) % 4;
			tweenId = tween->tweenPos(obj, glm::vec3(glm::vec2(cycle <= 1 ? 0.5f : -0.5f, ((cycle + 3) % 4) <= 1 ? 0.5f : -0.5f) * 3.0f, -2.0f), tweenSettings);
		}

		if (input->isKeyJustPressed(Yngin::KEY::SPACE)) {
			tween->setPaused(tweenId, !tween->isPaused(tweenId));
		}

		if (github->isClicked()) {
			system("start https://github.com/YusufYaser/Yngin");
		}

		if (github->isHeld()) {
			github->setColor(glm::vec4(0.5f));
		} else if (github->isHovered()) {
			github->setColor(glm::vec4(0.75f));
		} else {
			github->setColor(glm::vec4(1.0f));
		}

		double delta = ctx->getDeltaTime();

		if (input->isMouseJustPressed(MOUSE_BUTTON::RIGHT)) {
			oldMousePos = input->getMousePos();
		}

		if (input->isMousePressed(MOUSE_BUTTON::RIGHT)) {
			glm::vec3 o = defaultCamera->getOrientation();
			glm::ivec2 m = input->getMousePos() - oldMousePos;
			oldMousePos = input->getMousePos();

			float senstivity = 0.002f;

			float yaw = atan2(o.x, o.y);
			float pitch = asin(o.z);

			o.x = cos(pitch) * sin(yaw + m.x * senstivity);
			o.y = cos(pitch) * cos(yaw + m.x * senstivity);
			o.z = sin(pitch - m.y * senstivity);

			defaultCamera->setOrientation(o);

			glm::vec3 forward = glm::normalize(o);
			glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 0, 1)));
			glm::vec3 realUp = glm::cross(forward, right);

			glm::vec3 change = {};

			if (input->isKeyPressed(Yngin::KEY::W)) {
				change += forward;
			}
			if (input->isKeyPressed(Yngin::KEY::S)) {
				change -= forward;
			}
			if (input->isKeyPressed(Yngin::KEY::D)) {
				change += right;
			}
			if (input->isKeyPressed(Yngin::KEY::A)) {
				change -= right;
			}
			if (input->isKeyPressed(Yngin::KEY::Q)) {
				change += realUp;
			}
			if (input->isKeyPressed(Yngin::KEY::E)) {
				change -= realUp;
			}

			float speed = 5.0f;
			if (input->isKeyPressed(Yngin::KEY::LSHIFT)) {
				speed *= 1.5f;
			}

			defaultCamera->setPos(defaultCamera->getPos() + change * (float)delta * speed);
		}

		ctx->update();
	}

	Yngin::terminate();

	return 0;
}
