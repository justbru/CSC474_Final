/*
CPE/CSC 474 Lab base code Eckhardt/Dahl
based on CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/
#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "WindowManager.h"
#include "ParticleSystem.h"

#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <glad/glad.h>
#include "SmartTexture.h"
#include "GLSL.h"
#include "Program.h"
#include "WindowManager.h"
#include "Shape.h"
#include "skmesh.h"
#include "ParticleSystem.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// assimp
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/vector3.h"
#include "assimp/scene.h"
#include <assimp/mesh.h>

using namespace std;
using namespace glm;
using namespace Assimp;

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime = glfwGetTime();
	double difference = actualtime - lasttime;
	lasttime = actualtime;
	return difference;
}

class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = glm::vec3(0, 0, 0);
		rot = glm::vec3(0, 0, 0);
	}

	glm::mat4 process(double ftime)
	{
		double speed = 0.0f;

		if (w == 1)
		{
			speed = 10 * ftime;
		}
		else if (s == 1)
		{
			speed = -10 * ftime;
		}
		double yangle = 0;
		if (a == 1)
			yangle = -3 * ftime;
		else if (d == 1)
			yangle = 3 * ftime;
		rot.y += (float)yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed, 1);
		dir = dir * R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R * T;
	}
};

camera mycam;

class Application : public EventCallbacks
{
public:
	WindowManager *windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> psky, skinProg, prog, particleProg;

	// skinnedMesh
	SkinnedMesh skmesh;
	// textures
	shared_ptr<SmartTexture> skyTex, rockTex, waterTex;

	shared_ptr<ParticleSystem> particleEmitter;

	// shapes
	shared_ptr<Shape> skyShape, rock, water;

	// values for diver rotation
	float diverRotationSpeed = 0.f;
	float diverRotationTheta = 0.f;
	float diverTakeOffAngleTimeTheta = 0.f;
	bool diverLeanBack = true;
	float diverRot = 0.0f;
	float initialXVelocity = 0.0f;
	float initialYVelocity = 0.15f;
	float gravityAccel = -0.001f;
	float diverYVelocity = 0.1f;

	vec3 diverPos = vec3(0.2, -0.7, -5.5);
	bool takeoff = false;
	bool crouching = false;

	int currentAnimation = 6;
	int currentLevel = 1;

	float activeParticleEmitter = false;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		else if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		else if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		else if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		else if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
		if (key == GLFW_KEY_R && action == GLFW_RELEASE)
		{
			// update start pos based on current level
			switch (currentLevel)
			{
			case 1:
				diverPos = vec3(0.2, -0.7, -5.5);
				break;
			case 2:
				diverPos = vec3(-1, 0.5, -5.5);
				break;
			case 3:
				diverPos = vec3(-0.7, 1.4, -5.5);
				break;
			case 4:
				diverPos = vec3(-1.5, 2.2, -6.5);
				break;
			}

			diverTakeOffAngleTimeTheta = 0.0f;
			diverRotationSpeed = 0.f;
			diverRotationTheta = 0.f;
			diverTakeOffAngleTimeTheta = 0.f;
			diverLeanBack = true;
			diverRot = 0.0f;
			initialXVelocity = 0.0f;
			initialYVelocity = 0.15f;
			gravityAccel = -0.001f;
			diverYVelocity = 0.1f;
			takeoff = false;
			crouching = false;
			skmesh.setCurrentAnimation(6);
			currentAnimation = 6;
			activeParticleEmitter = false;
		}

		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		{
			if (!skmesh.isBlending)
			{
				if (currentAnimation == 6)
				{
					crouching = true;
					diverTakeOffAngleTimeTheta = 0.0f;
					skmesh.SetNextAnimation(3);
					currentAnimation = 3;
				}
				else if (currentAnimation == 3)
				{
					skmesh.SetNextAnimation(4);
					currentAnimation = 4;
				}
				else if (currentAnimation == 4)
				{
					skmesh.SetNextAnimation(7);
					currentAnimation = 7;
				}
				else if (currentAnimation == 7)
				{
					skmesh.SetNextAnimation(4);
					currentAnimation = 4;
				}
			}
		}
		if (key == GLFW_KEY_U && action == GLFW_PRESS)
		{
			currentLevel = currentLevel > 3 ? 1 : currentLevel + 1;
		}

		if (key == GLFW_KEY_1 && action == GLFW_RELEASE)
		{
			skmesh.SetNextAnimation(0);
		}
		if (key == GLFW_KEY_2 && action == GLFW_RELEASE)
		{
			skmesh.SetNextAnimation(1);
		}
		if (key == GLFW_KEY_3 && action == GLFW_RELEASE)
		{
			skmesh.SetNextAnimation(2);
		}
		if (key == GLFW_KEY_4 && action == GLFW_RELEASE)
		{
			crouching = true;
			diverTakeOffAngleTimeTheta = 0.0f;
			skmesh.SetNextAnimation(3);
		}
		if (key == GLFW_KEY_5 && action == GLFW_RELEASE)
		{
			skmesh.SetNextAnimation(4);
		}
		if (key == GLFW_KEY_6 && action == GLFW_RELEASE)
		{
			skmesh.SetNextAnimation(5);
		}
		if (key == GLFW_KEY_7 && action == GLFW_RELEASE)
		{
			skmesh.SetNextAnimation(6);
		}
		if (key == GLFW_KEY_8 && action == GLFW_RELEASE)
		{
			skmesh.SetNextAnimation(7);
		}
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
	}

	// if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		// get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}
	/*Note that any gl calls must always happen after a GL state is initialized */

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom(const std::string &resourceDirectory)
	{
		if (!skmesh.LoadMesh(resourceDirectory + "/riggedDiver7.fbx"))
		{
			printf("Mesh load failed\n");
			return;
		}

		// set initial animation to the idle animation
		skmesh.setCurrentAnimation(6);
		currentAnimation = 6;

		// Initialize mesh.
		skyShape = make_shared<Shape>();
		skyShape->loadMesh(resourceDirectory + "/sphere.obj");
		skyShape->resize();
		skyShape->init();

		water = make_shared<Shape>();
		water->loadMesh(resourceDirectory + "/cube.obj");
		water->resize();
		water->init();

		// Initialize rock
		rock = make_shared<Shape>();
		rock->loadMesh(resourceDirectory + "/CavePlatform1_Obj.obj");
		rock->resize();
		rock->init();

		// sky texture
		auto strSky = resourceDirectory + "/sky.jpg";
		skyTex = SmartTexture::loadTexture(strSky, true);
		if (!skyTex)
			cerr << "error: texture " << strSky << " not found" << endl;

		// sky texture
		auto strRock = resourceDirectory + "/rock2.jpg";
		rockTex = SmartTexture::loadTexture(strRock, false);
		if (!rockTex)
			cerr << "error: texture " << strRock << " not found" << endl;

		auto strWater = resourceDirectory + "/water.jpeg";
		waterTex = SmartTexture::loadTexture(strWater, false);
		if (!waterTex)
			cerr << "error: texture " << strWater << " not found" << endl;
	}

	// General OGL initialization - set OGL state here
	void init(const std::string &resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		// glDisable(GL_DEPTH_TEST);
		//  Initialize the GLSL program.
		psky = std::make_shared<Program>();
		psky->setVerbose(true);
		psky->setShaderNames(resourceDirectory + "/skyvertex.glsl", resourceDirectory + "/skyfrag.glsl");
		if (!psky->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}

		psky->addUniform("P");
		psky->addUniform("V");
		psky->addUniform("M");
		psky->addUniform("tex");
		psky->addUniform("camPos");
		psky->addAttribute("vertPos");
		psky->addAttribute("vertNor");
		psky->addAttribute("vertTex");

		skinProg = std::make_shared<Program>();
		skinProg->setVerbose(true);
		skinProg->setShaderNames(resourceDirectory + "/skinning_vert.glsl", resourceDirectory + "/skinning_frag.glsl");
		if (!skinProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}

		skinProg->addUniform("P");
		skinProg->addUniform("V");
		skinProg->addUniform("M");
		skinProg->addUniform("tex");
		skinProg->addUniform("camPos");
		skinProg->addAttribute("vertPos");
		skinProg->addAttribute("vertNor");
		skinProg->addAttribute("vertTex");
		skinProg->addAttribute("BoneIDs");
		skinProg->addAttribute("Weights");

		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}

		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("flip");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("MatShine");
		prog->addUniform("lightPos");

		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");

		particleProg = std::make_shared<Program>();
		particleProg->setVerbose(true);
		particleProg->setShaderNames(resourceDirectory + "/particle_vert.glsl", resourceDirectory + "/particle_frag.glsl");
		if (!particleProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		particleProg->addUniform("P");
		particleProg->addUniform("V");
		particleProg->addUniform("M");
		particleProg->addAttribute("vertPos");

		particleEmitter = make_shared<ParticleSystem>();
		particleEmitter->createParticleSystem(vec3(2, 2, -6), 0.1, 50);
		particleEmitter->init();
	}

	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		double frametime = get_last_elapsed_time();
		static double totaltime = 0;
		totaltime += frametime;
		float animSpeed = 1.f;

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now

		glm::mat4 V, M, P; // View, Model and Perspective matrix
		V = mycam.process(frametime);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);
		if (width < height)
		{
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect, -2.0f, 100.0f);
		}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); // so much type casting... GLM metods are quite funny ones
		auto sangle = -3.1415926f / 2.0f;
		glm::mat4 RotateXSky = glm::rotate(glm::mat4(1.0f), sangle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::vec3 camp = -mycam.pos;
		glm::mat4 TransSky = glm::translate(glm::mat4(1.0f), camp);
		glm::mat4 SSky = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

		M = TransSky * RotateXSky * SSky;

		// Draw the sky using GLSL.
		psky->bind();
		GLuint texLoc = glGetUniformLocation(psky->pid, "tex");
		skyTex->bind(texLoc);
		glUniformMatrix4fv(psky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(psky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(psky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(psky->getUniform("camPos"), 1, &mycam.pos[0]);

		glDisable(GL_DEPTH_TEST);
		skyShape->draw(psky, false);
		glEnable(GL_DEPTH_TEST);
		skyTex->unbind();
		psky->unbind();

		// // draw water
		// psky->bind();
		// glUniformMatrix4fv(psky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		// glUniformMatrix4fv(psky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		// glUniformMatrix4fv(psky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		// glUniform3fv(psky->getUniform("camPos"), 1, &mycam.pos[0]);

		// glDisable(GL_DEPTH_TEST);
		// skyShape->draw(psky, false);
		// glEnable(GL_DEPTH_TEST);
		// skyTex->unbind();
		// psky->unbind();

		prog->bind();
		glUniform3f(prog->getUniform("MatAmb"), 0.1, 0.1, 0.1);
		glUniform3f(prog->getUniform("MatDif"), 0.05, 0.36, 0.85);
		glUniform3f(prog->getUniform("MatSpec"), 0.5, 0.5, 0.5);
		glUniform1f(prog->getUniform("MatShine"), 5.0);

		glUniform3f(prog->getUniform("lightPos"), 10.f, 20.0, 2.0);

		glm::mat4 Trans = glm::translate(glm::mat4(1.0f), vec3(0, -2, 0));
		glm::mat4 Scale = glm::scale(glm::mat4(1.0f), glm::vec3(300.f, 0.01f, 300.f));
		glm::mat4 RotX = glm::rotate(glm::mat4(1.0f), 0.f, vec3(0, 1, 0));

		M = Trans * RotX * Scale;
		texLoc = glGetUniformLocation(prog->pid, "tex");
		waterTex->bind(texLoc);
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		// glUniform3fv(psky->getUniform("camPos"), 1, &mycam.pos[0]);

		water->draw(prog, false);
		prog->unbind();

		// Load rock obj
		prog->bind();

		// Add a gray color
		// glUniform3f(prog->getUniform("MatAmb"), 0.1, 0.1, 0.1);
		// glUniform3f(prog->getUniform("MatDif"), 0.4745, 0.4471, 0.4431);
		// glUniform3f(prog->getUniform("MatSpec"), 0.1, 0.1, 0.1);
		// glUniform1f(prog->getUniform("MatShine"), 2.0);

		glUniform3f(prog->getUniform("lightPos"), 10.f, 20.0, 2.0);
		glUniform1f(prog->getUniform("MatShine"), 1.0);

		Trans = glm::translate(glm::mat4(1.0f), vec3(-2, -0.5, -7));
		Scale = glm::scale(glm::mat4(1.0f), glm::vec3(3.f, 3.f, 3.f));
		RotX = glm::rotate(glm::mat4(1.0f), 0.5f, vec3(0, 1, 0));

		M = Trans * RotX * Scale;
		texLoc = glGetUniformLocation(prog->pid, "tex");
		rockTex->bind(texLoc);
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		// glUniform3fv(psky->getUniform("camPos"), 1, &mycam.pos[0]);

		rock->draw(prog, false);
		prog->unbind();

		/*
			Use all the below code later for animating the diver
		*/

		// draw the skinned mesh
		skinProg->bind();
		texLoc = glGetUniformLocation(skinProg->pid, "tex");
		sangle = -3.1415926f / 2.0f;

		if (takeoff)
		{
			if (initialXVelocity > 0.15)
				initialXVelocity -= 0.001;
			if (diverPos.y > -1.4)
				diverPos.x += initialXVelocity * 0.1f;
			else
				diverPos.x += initialXVelocity * 0.05f;

			diverYVelocity = diverYVelocity <= -0.15f ? -0.15f : (diverYVelocity + gravityAccel);
			diverPos.y += diverYVelocity * 0.1f;
		}

		Trans = glm::translate(glm::mat4(1.0f), diverPos);
		RotX = glm::rotate(glm::mat4(1.0f), -diverRot, vec3(1, 0, 0));
		Scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.0014f, 0.0014f, 0.0014f));
		glm::mat4 RotY = glm::rotate(glm::mat4(1.0f), -3.1415f / 2.f, vec3(0, 1, 0));
		glm::mat4 TransOffset = glm::translate(glm::mat4(1.0f), vec3(0, -0.62, 0));

		if ((skmesh.getCurrentAnimation() != 3 && skmesh.getCurrentAnimation() != 6) || (skmesh.getCurrentAnimation() == 3 && skmesh.getNextAnimation() == 4))
			M = Trans * RotY * RotX * TransOffset * Scale;
		else
			M = TransOffset * Trans * RotY * RotX * Scale;

		glUniform3fv(skinProg->getUniform("camPos"), 1, &mycam.pos[0]);
		glUniformMatrix4fv(skinProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(skinProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(skinProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);

		if (skmesh.isBlending)
			animSpeed = 1.5f;
		else
			animSpeed = 1.f;

		skmesh.setBoneTransformations(skinProg->pid, frametime * 1.f);
		skmesh.Render(texLoc);
		skinProg->unbind();

		if (diverTakeOffAngleTimeTheta > 3.f && crouching)
		{
			diverLeanBack = !diverLeanBack;
			diverTakeOffAngleTimeTheta = 0.f;
		}

		if (diverPos.y > -1.9)
		{
			switch (skmesh.getCurrentAnimation())
			{
			case 3:
				if (skmesh.getNextAnimation() != -1)
				{
					diverRotationSpeed = 0.01f;
					takeoff = true;
				}
				else
				{
					initialXVelocity = diverRot;
					if (diverLeanBack)
						diverRotationSpeed = 0.0008f;
					else
						diverRotationSpeed = -0.0008f;
				}
				break;
			case 4:
				if (skmesh.getNextAnimation() != -1)
					diverRotationSpeed = 0.06f;
				else
					diverRotationSpeed = 0.02f;
				break;
			case 7:
				if (skmesh.getNextAnimation() != -1)
					diverRotationSpeed = 0.03f;
				else
					diverRotationSpeed = 0.06f;
				break;
			default:
				diverRotationSpeed = 0.0f;
				break;
			}
		}
		else
		{
			if (!activeParticleEmitter)
			{
				float rotationVal = fmod(diverRot, 3.1415f) < (3.1415f / 2.f) ? fmod(diverRot, 3.1415f) : 3.1415f - fmod(diverRot, 3.1415f);
				float splashRadius = rotationVal / 10.f;
				int totalParticles = 500 * (rotationVal / (3.1415f / 2.f));

				if (skmesh.getCurrentAnimation() == 7){
					totalParticles = 1000;
					splashRadius = 0.1;
				}
				// 0.1 is a large splash radius, 0.01 is small, 50 is small amount of particles, 200 is large
				particleEmitter->createParticleSystem(diverPos, splashRadius, totalParticles);
				activeParticleEmitter = true;
			}
			diverRotationSpeed = 0.0f;
		}

		if (activeParticleEmitter)
		{
			particleProg->bind();

			M = glm::mat4(1);
			glUniformMatrix4fv(particleProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(particleProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniformMatrix4fv(particleProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);

			particleEmitter->updateParticles();
			particleEmitter->draw();
			// rock->draw(particleProg, false);

			particleProg->unbind();

			// exit(1);
		}

		diverRot += diverRotationSpeed;
		diverTakeOffAngleTimeTheta += frametime;
	}
};

//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	std::string missingTexture = "missing.jpg";

	SkinnedMesh::setResourceDir(resourceDir);
	SkinnedMesh::setDefaultTexture(missingTexture);

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager *windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
