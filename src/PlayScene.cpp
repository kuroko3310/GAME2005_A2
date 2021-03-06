#include "PlayScene.h"
#include "Game.h"
#include "EventManager.h"

// required for IMGUI
#include "imgui.h"
#include "imgui_sdl.h"
#include "Renderer.h"
#include "Util.h"

const float PI = 3.14159265359;

PlayScene::PlayScene()
{
	PlayScene::start();
}

PlayScene::~PlayScene()
= default;

void PlayScene::draw()
{
	TextureManager::Instance()->draw("background", 400, 300, 0, 255, true);
	
	drawDisplayList();
	
	if (EventManager::Instance().isIMGUIActive())
	{
		GUI_Function();
	}
	SDL_SetRenderDrawColor(Renderer::Instance()->getRenderer(), 255, 255, 255, 255);

	
}

void PlayScene::update()
{
	updateDisplayList();

}

void PlayScene::clean()
{
	removeAllChildren();
}

void PlayScene::handleEvents()
{
	EventManager::Instance().update();

	// handle player movement with GameController
	if (SDL_NumJoysticks() > 0)
	{
		if (EventManager::Instance().getGameController(0) != nullptr)
		{
			const auto deadZone = 10000;
			if (EventManager::Instance().getGameController(0)->LEFT_STICK_X > deadZone)
			{
				m_pPlayer->setAnimationState(PLAYER_RUN_RIGHT);
				m_playerFacingRight = true;
			}
			else if (EventManager::Instance().getGameController(0)->LEFT_STICK_X < -deadZone)
			{
				m_pPlayer->setAnimationState(PLAYER_RUN_LEFT);
				m_playerFacingRight = false;
			}
			else
			{
				if (m_playerFacingRight)
				{
					m_pPlayer->setAnimationState(PLAYER_IDLE_RIGHT);
				}
				else
				{
					m_pPlayer->setAnimationState(PLAYER_IDLE_LEFT);
				}
			}
		}
	}


	// handle player movement if no Game Controllers found
	if (SDL_NumJoysticks() < 1)
	{
		if (EventManager::Instance().isKeyDown(SDL_SCANCODE_A))
		{
			m_pPlayer->setAnimationState(PLAYER_RUN_LEFT);
			m_playerFacingRight = false;
		}
		else if (EventManager::Instance().isKeyDown(SDL_SCANCODE_D))
		{
			m_pPlayer->setAnimationState(PLAYER_RUN_RIGHT);
			m_playerFacingRight = true;
		}
		else
		{
			if (m_playerFacingRight)
			{
				m_pPlayer->setAnimationState(PLAYER_IDLE_RIGHT);
			}
			else
			{
				m_pPlayer->setAnimationState(PLAYER_IDLE_LEFT);
			}
		}
	}
	

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_ESCAPE))
	{
		TheGame::Instance()->quit();
	}

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_1))
	{
		TheGame::Instance()->changeSceneState(START_SCENE);
	}

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_2))
	{
		TheGame::Instance()->changeSceneState(END_SCENE);
	}
}

void PlayScene::start()
{
	// Background
	TextureManager::Instance()->load("../Assets/textures/bg.png", "background");


	// Set GUI Title
	m_guiTitle = "Play Scene";


	// Plane Sprite
	m_pPlaneSprite = new Plane();
	addChild(m_pPlaneSprite);


	// Player Sprite
	m_pPlayer = new Player();
	addChild(m_pPlayer);
	m_playerFacingRight = true;

	// Ball Sprite
	m_pBall = new Target();
	addChild(m_pBall);

	// Back Button
	m_pBackButton = new Button("../Assets/textures/backButton.png", "backButton", BACK_BUTTON);
	m_pBackButton->getTransform()->position = glm::vec2(100.0f, 50.0f);
	m_pBackButton->addEventListener(CLICK, [&]()-> void
	{
		m_pBackButton->setActive(false);
		TheGame::Instance()->changeSceneState(START_SCENE);
	});

	m_pBackButton->addEventListener(MOUSE_OVER, [&]()->void
	{
		m_pBackButton->setAlpha(128);
	});

	m_pBackButton->addEventListener(MOUSE_OUT, [&]()->void
	{
		m_pBackButton->setAlpha(255);
	});
	addChild(m_pBackButton);

	// Next Button
	m_pNextButton = new Button("../Assets/textures/nextButton.png", "nextButton", NEXT_BUTTON);
	m_pNextButton->getTransform()->position = glm::vec2(700.0f, 50.0f);
	m_pNextButton->addEventListener(CLICK, [&]()-> void
	{
		m_pNextButton->setActive(false);
		TheGame::Instance()->changeSceneState(END_SCENE);
	});

	m_pNextButton->addEventListener(MOUSE_OVER, [&]()->void
	{
		m_pNextButton->setAlpha(128);
	});

	m_pNextButton->addEventListener(MOUSE_OUT, [&]()->void
	{
		m_pNextButton->setAlpha(255);
	});

	addChild(m_pNextButton);

	/* Instructions Label */
	const SDL_Color white = { 255, 255, 255, 255 };
	const SDL_Color blue = { 0, 0, 255, 255 };
	m_pInstructionsLabel = new Label("Press the backtick (`) character to toggle Debug View", "Consolas", 12, white);
	m_pInstructionsLabel->getTransform()->position = glm::vec2(Config::SCREEN_WIDTH * 0.5f, 550.0f);
	addChild(m_pInstructionsLabel);

	m_pInstructionsLabel = new Label("Scale = 5 meters per pixel", "Consolas", 12, white);
	m_pInstructionsLabel->getTransform()->position = glm::vec2(400, 30.0f);
	addChild(m_pInstructionsLabel);

	m_pdistanceLabel = new Label("Distance", "Consolas", 12, white, glm::vec2(400.0f, 45.0f));
	m_pdistanceLabel->setParent(this);
	addChild(m_pdistanceLabel);

	m_pVelocityLabel = new Label("Velocity", "Consolas", 12, white, glm::vec2(400.0f, 60.0f)); 
	m_pVelocityLabel->setParent(this);
	addChild(m_pVelocityLabel);

	m_pAccLabel = new Label("Acceleration", "Consolas", 12, white, glm::vec2(400.0f, 75.0f));
	m_pAccLabel->setParent(this);
	addChild(m_pAccLabel);

	m_pAngleLabel = new Label("Angle of the ramp", "Consolas", 12, white, glm::vec2(400.0f, 90.0f));
	m_pAngleLabel->setParent(this);
	addChild(m_pAngleLabel);

	// Fnet
	m_pFNetLabel = new Label("Net Force", "Consolas", 12, white, glm::vec2(400.0f, 105.0f));
	m_pFNetLabel->setParent(this);
	addChild(m_pFNetLabel);
	// FNormal
	m_pFNormalLabel = new Label("Normal Force", "Consolas", 12, white, glm::vec2(400.0f, 120.0f));
	m_pFNormalLabel->setParent(this);
	addChild(m_pFNormalLabel);
	// FGravity
	m_pFGravityLabel = new Label("Force of Gravity", "Consolas", 12, white, glm::vec2(400.0f, 135.0f));
	m_pFGravityLabel->setParent(this);
	addChild(m_pFGravityLabel);
	// FFriction
	m_pFFrictionLabel = new Label("Friction Force", "Consolas", 12, white, glm::vec2(400.0f, 150.0f));
	m_pFFrictionLabel->setParent(this);
	addChild(m_pFFrictionLabel);
}

void PlayScene::GUI_Function() const
{
	// Always open with a NewFrame
	ImGui::NewFrame();

	// See examples by uncommenting the following - also look at imgui_demo.cpp in the IMGUI filter
	//ImGui::ShowDemoWindow();
	
	ImGui::SetNextWindowPos(ImVec2(250,165)); // hardcoded the position of imgui
	ImGui::Begin("Physics Control", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar );

	

	ImGui::Separator();

	static bool gravcheck = false;
	if (ImGui::Checkbox("Gravity", &gravcheck))
	{
		m_pBall->isGravityEnabled = gravcheck;
	}
	
	// Mass
	static float mass = 1.0f;
	ImGui::SliderFloat("Mass in kg", &mass, 1.0f, 200.0f);

	// Ramp
	float xRampPos = m_pPlayer->getTransform()->position.x;
	float yRampPos = 465; //m_pPlayer->getTransform()->position.y;

	// Ramp width
	static float width = 0.0f;
	ImGui::SliderFloat("width", &width, 0.0f, 400.0f);
	Util::DrawLine(glm::vec2(xRampPos, yRampPos), glm::vec2(xRampPos + width, yRampPos ));

	// Ramp height
	static float height = 0.0f;
	ImGui::SliderFloat("height", &height, 0.0f, 300.0f);
	Util::DrawLine(glm::vec2(xRampPos, yRampPos ), glm::vec2(xRampPos, yRampPos - height ));

	// Ramp slope
	Util::DrawLine(glm::vec2(xRampPos, yRampPos - height), glm::vec2(xRampPos + width, yRampPos));

	// Floor friction
	static float coefficient = 0.1f;
	ImGui::SliderFloat("Kinetic Coefficient", &coefficient, 0.0f, 1.0f);
	float friction = coefficient * -9.8;
	

	
	if (m_pBall->getTransform()->position.y + 15 >= 465 && m_pBall->getRigidBody()->velocity.x > 1)
	{
		
		
		m_pFNormalLabel->setText("Normal Force = " + std::to_string(mass * 9.8));
		m_pFGravityLabel->setText("Force of Gravity = " + std::to_string(mass * -9.8));
		if (m_pBall->getRigidBody()->velocity.x < 1.5f)
		{
			m_pBall->getRigidBody()->acceleration = glm::vec2(0, 0);
			m_pBall->getRigidBody()->velocity.x = 0;
			m_pAccLabel->setText("Acceleration = " + std::to_string(m_pBall->getRigidBody()->acceleration.x));
			m_pFFrictionLabel->setText("Friction Force = " + std::to_string(mass * m_pBall->getRigidBody()->acceleration.x));
			m_pFNetLabel->setText("Net Force = " + std::to_string(mass * m_pBall->getRigidBody()->acceleration.x));
			
		}
		else
		{
			m_pBall->getRigidBody()->acceleration = glm::vec2(friction, 0);
			m_pAccLabel->setText("Acceleration = " + std::to_string(friction));
			m_pFFrictionLabel->setText("Friction Force = " + std::to_string(mass * friction));
			m_pFNetLabel->setText("Net Force = " + std::to_string(mass * friction));
		}

	}

	float angle = atan(height / width);
	// Slide
	if (ImGui::Button("Ramp"))
	{
		m_pBall->doThrow();
		m_pBall->throwPosition = glm::vec2(xRampPos, yRampPos - height - 15);
		m_pBall->getRigidBody()->acceleration = glm::vec2(9.8 * cos(angle), 9.8 * sin(angle));
		m_pAccLabel->setText("Acceleration = " + std::to_string((9.8 * sin(angle))));
		m_pFNetLabel->setText("Net Force = " + std::to_string(mass * (9.8 * sin(angle))));
		m_pFNormalLabel->setText("Normal Force = " + std::to_string(mass * (9.8 * cos(angle))));
		m_pFGravityLabel->setText("Force of Gravity = " + std::to_string(mass * -9.8));

		//Display angle
		if ((height / width)>(-2/PI)|| (height/ width) < (2 / PI))
		{
			float degAngle = (angle / PI) * 180;
			m_pAngleLabel->setText("Angle of the ramp = " + std::to_string(degAngle) + " degree");
		}
	}

	// Display distance
	std::string labelText = "";
	if (m_pBall->isColliding(m_pPlaneSprite)) {
		labelText = "HIT";
	}
	else {
		labelText = "Distance = " + std::to_string(m_pBall->getDistance(m_pPlaneSprite));
	}
	m_pdistanceLabel->setText(labelText);

	// Display velocity
	float x = m_pBall->getRigidBody()->velocity.x;
	float y = m_pBall->getRigidBody()->velocity.y;
	float magnitude = sqrt(x * x + y * y);
	m_pVelocityLabel->setText("Velocity = " + std::to_string((magnitude)));

	

	ImGui::End();
	// Don't Remove this
	ImGui::Render();
	
	ImGuiSDL::Render(ImGui::GetDrawData());
	ImGui::StyleColorsDark();
}
