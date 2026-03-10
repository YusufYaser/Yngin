// I know this code isn't the best but I'll improve it soon
//
// I just quickly add code to test new features without
// checking the code style

#include <stdio.h>
#include <fstream>
#include <sstream>
#include <format>
#include <Yngin/Yngin.h>
#include <glm/glm.hpp>

using namespace Yngin;

int main() {
	Yngin::init();

	ContextSettings settings = {};
	settings.windowSettings = {
		.title = "Yngin Demo"
	};

	Context* ctx = createContext(settings);

	printf("Context: %p\n", ctx);

	if (ctx == nullptr || ctx->getStatus() == CONTEXT_STATUS::FAILED_TO_INIT) {
		printf("Failed to create context\n");
		return 1;
	}

	ModelsManager* modelsMgr = ctx->getModelsManager();
	ScenesManager* scenesManager = ctx->getScenesManager();
	Scene* scene = scenesManager->createScene();
	Scene* scene2 = scenesManager->createScene();
	GameObjectsManager* gameObjMgr = scene->getGameObjectsManager();
	UI::UIManager* uiMgr = scene->getUIManager();
	UI::UIManager* globalUiMgr = ctx->getGlobalUIManager();

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

	defaultCamera->setPosition({ 10, 0, 0 });
	defaultCamera->lookAt(glm::vec3());
	defaultCamera->setFov(90.0f);

	TexturesManager* texMgr = ctx->getTexturesManager();

	// https://freestylized.com/skybox/sky_36/
	Texture* skyboxTex = texMgr->createTexture("skybox.png", {
		.wrap = TEXTURE_WRAP::CLAMP,
		.filterMin = TEXTURE_FILTER::NEAREST,
		.filterMag = TEXTURE_FILTER::NEAREST,
		});

	scene->setSkyboxTexture(skyboxTex);

	Texture* githubTex = texMgr->createTexture("github.png");

	TextureData texData{};
	texData.width = 2;
	texData.height = 2;
	texData.numCh = 1;
	texData.bytes = "\xff\x80\x80\xff";
	TextureSettings texSettings{};
	texSettings.wrap = TEXTURE_WRAP::REPEAT;
	texSettings.filterMin = TEXTURE_FILTER::NEAREST;
	texSettings.filterMag = TEXTURE_FILTER::NEAREST;
	Texture* tex = texMgr->createTexture(texData, texSettings);

	TextureData whiteTexData = {
		.width = 1,
		.height = 1,
		.numCh = 1,
		.bytes = "\xff"
	};
	Texture* whiteTex = texMgr->createTexture(whiteTexData);

	// https://github.com/shannpersand/comic-shanns/blob/master/v2/comic%20shanns%202.ttf
	Texture* glyphTex = texMgr->createTexture("glyph.png", {
		.wrap = TEXTURE_WRAP::CLAMP,
		.filterMin = TEXTURE_FILTER::LINEAR_MIPMAP_LINEAR,
		.filterMag = TEXTURE_FILTER::LINEAR,
		});

	Texture* buttonText = texMgr->createTexture("button.png");

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
	wall->setPosition({ 0, 0, -5.0f });

	{
		GameObject* light = gameObjMgr->getRootGameObject()->createChild();
		Components::Mesh* mesh = light->createComponent<Components::Mesh>();
		mesh->setModel(cubeModel);
		mesh->setTexture(whiteTex);
		light->setPosition({ 0, 0, 2 });

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
		light->setPosition({ 0, 3.0f, -0.5f });

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
		light->setPosition({ 3.0f, 0, 0.5f });

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
	int currentScene = 0;

	int textShown = 0;
	std::string testText;

	for (int c = 0; c < 256; c++) {
		if (c % 16 == 0) testText += '\n';
		if (c == '\n' || c == '\r') {
			testText += ' ';
			continue;
		}
		testText += c;
	}

	UI::Text* text = globalUiMgr->getRootElement()->createChild<UI::Text>();
	text->setGlyph(glyphTex);
	text->setText("");
	text->setTextSize(24);


	UI::UIElement* github = uiMgr->getRootElement()->createChild<UI::UIElement>();

	UI::Image* githubImg = github->createChild<UI::Image>();
	githubImg->setTexture(githubTex->getId());
	githubImg->setPosition({ 0, 48, 1.0f, -24 });
	githubImg->setSize({ 0, 64, 0, 64 });
	githubImg->setPivot({ 0.5, 1 });

	UI::Text* githubText = github->createChild<UI::Text>();
	githubText->setGlyph(glyphTex);
	githubText->setText(__TIMESTAMP__);
	githubText->setTextSize(12);
	githubText->setPosition({ 0, 48, 1.0f, -6 });
	githubText->setPivot({ 0.5, 1 });
	githubText->setText("GitHub");

	UI::Text* scene2Text = scene2->getUIManager()->getRootElement()->createChild<UI::Text>();
	scene2Text->setText("Scene 2");
	scene2Text->setPosition({ .5f, 0, .5f, 0 });
	scene2Text->setPivot({ .5f, .5f });
	scene2Text->setGlyph(glyphTex);

	scene2->setSkyboxTexture(skyboxTex);

	scene2->getGameObjectsManager()->getRootGameObject()->createComponent<Components::Mesh>()->setModel(cubeModel);

	glm::ivec2 oldMousePos = {};

	UI::Button* button = globalUiMgr->getRootElement()->createChild<UI::Button>();
	button->getTextElement()->setGlyph(glyphTex);
	button->getTextElement()->setText("Switch Scene");
	button->getImage()->setTexture(buttonText);
	button->setPivot({ 1, 1 });
	button->setPosition({ 1, -16, 1, -16 });

	while (ctx->getStatus() == CONTEXT_STATUS::RUNNING) {
		ctx->makeCurrent();

		if (input->isKeyJustPressed(Yngin::KEY::F11) || (input->isKeyPressed(Yngin::KEY::RALT) && input->isKeyJustPressed(Yngin::KEY::ENTER))) {
			window->setFullscreen(!window->isFullscreen());
		}

		if (input->isKeyJustPressed(Yngin::KEY::NUM_1)) {
			textShown = (textShown + 1) % 2;
		}

		if (button->isClicked()) {
			currentScene = (currentScene + 1) % 2;
		}

		if (textShown == 0) {
			glm::vec3 pos = defaultCamera->getPosition();

			text->setText(std::format("Yngin Demo\nFPS: {}\nPos: {}, {}, {}",
				round(1 / ctx->getDeltaTime()),
				round(pos.x),
				round(pos.y),
				round(pos.z)
			));
		} else if (textShown == 1) {
			text->setText(testText);
		}

		if (currentScene == 0) {
			scene->activate();
			defaultCamera = scene->getCamerasManager()->getCamera(0);
		} else if (currentScene == 1) {
			scene2->activate();
			defaultCamera = scene2->getCamerasManager()->getCamera(0);
		}

		window->setCursorLocked(input->isMousePressed(MOUSE_BUTTON::RIGHT));

		if (!tween->isActive(tweenId)) {
			cycle = (cycle + 1) % 4;
			tweenId = tween->tweenPos(obj, glm::vec3(glm::vec2(cycle <= 1 ? 0.5f : -0.5f, ((cycle + 3) % 4) <= 1 ? 0.5f : -0.5f) * 3.0f, -2.0f), tweenSettings);
		}

		if (input->isKeyJustPressed(Yngin::KEY::SPACE)) {
			tween->setPaused(tweenId, !tween->isPaused(tweenId));
		}

		if (githubImg->isClicked()) {
			system("start https://github.com/YusufYaser/Yngin");
		}

		if (githubImg->isHeld()) {
			githubImg->setColor(glm::vec4(0.5f));
		} else if (githubImg->isHovered()) {
			githubImg->setColor(glm::vec4(0.75f));
		} else {
			githubImg->setColor(glm::vec4(1.0f));
		}

		double delta = ctx->getDeltaTime();

		if (input->isMouseJustPressed(MOUSE_BUTTON::RIGHT)) {
			oldMousePos = input->getMousePos(true);
		}

		if (input->isMousePressed(MOUSE_BUTTON::RIGHT)) {
			glm::vec3 o = defaultCamera->getOrientation();
			glm::ivec2 m = input->getMousePos(true) - oldMousePos;
			oldMousePos = input->getMousePos(true);

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

			defaultCamera->setPosition(defaultCamera->getPosition() + change * (float)delta * speed);
		}

		ctx->update();
	}

	Yngin::terminate();

	return 0;
}
