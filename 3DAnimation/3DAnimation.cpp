#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "animator.h"
#include "model_animation.h"
#include "Player.h"

#include <iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

std::map<std::string, std::string> player1Anims = {
	{"idle", "Object/Vegas/Idle.dae"},
	{"walk", "Object/Vegas/WalkForward.dae"},
	{"run", "Object/Vegas/WalkBack.dae"},
	{"punch", "Object/Vegas/Punching.dae"},
	{"kick", "Object/Vegas/Kick.dae"}
};

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(false);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader ourShader("anim_model.vs", "anim_model.fs");
	Player player1("Object/Vegas/Big Vegas.dae", glm::vec3(0.0f, -0.4f, 0.0f), glm::vec3(1.0f, 2.0f, 1.0f), player1Anims);


	// load models
	// -----------
	// idle 3.3, walk 2.06, run 0.83, punch 1.03, kick 1.6
	
	enum AnimState charState = IDLE;
	float blendAmount = 0.0f;
	float blendRate = 0.055f;

	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);
		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
			player1.Action("idle");  // No blending when switching to idle
		}
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
			player1.Action("walk");  // Blend into walking
		}
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
			player1.Action("punch");  // Blend into punching
		}
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
			player1.Action("kick");  // Blend into kicking
		}
		
		
		switch (player1.getState()) {
		case IDLE:
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
				blendAmount = 0.0f;
				player1.Action("idle", "walk", blendAmount,false);
				charState = IDLE_WALK;
			}
			else if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
				blendAmount = 0.0f;
				player1.Action("idle", "punch", blendAmount,false);
				charState = IDLE_PUNCH;
			}
			else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
				blendAmount = 0.0f;
				player1.Action("idle", "kick", blendAmount,false);
				charState = IDLE_KICK;
			}
			player1.changeState(charState);
			printf("idle \n");
			break;
		case IDLE_WALK:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			player1.Action("idle", "walk", blendAmount,true);
			if (blendAmount > 0.9f) {
				blendAmount = 0.0f;
				float startTime = 2;
				player1.Action("walk", NULL, blendAmount,false);
				charState = WALK;
			}
			player1.changeState(charState);
			printf("idle_walk \n");
			break;
		case WALK:
			player1.Action("walk", NULL, blendAmount,true);
			if (glfwGetKey(window, GLFW_KEY_UP) != GLFW_PRESS) {
				charState = WALK_IDLE;
			}
			player1.changeState(charState);
			printf("walking\n");
			break;
		case WALK_IDLE:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			player1.Action("walk", "idle", blendAmount,true);
			if (blendAmount > 0.9f) {
				blendAmount = 0.0f;
				player1.Action("idle", NULL, blendAmount,false);
				charState = IDLE;
			}
			player1.changeState(charState);
			printf("walk_idle \n");
			break;
		case IDLE_PUNCH:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			player1.Action("idle", "punch", blendAmount,true);
			if (blendAmount > 0.9f) {
				blendAmount = 0.0f;
				player1.Action("punch", NULL, blendAmount,false);
				charState = PUNCH_IDLE;
			}
			player1.changeState(charState);
			printf("idle_punch\n");
			break;
		case PUNCH_IDLE:
			if (player1.getCurrentTime() > 0.7 * player1.getAnimationDuration("punch")) {
				blendAmount += blendRate;
				blendAmount = fmod(blendAmount, 1.0f);
				player1.Action("punch", "idle", blendAmount,true);
				if (blendAmount > 0.9f) {
					blendAmount = 0.0f;
					player1.Action("idle", NULL, blendAmount,false);
					charState = IDLE;
				}
				player1.changeState(charState);
				printf("punch_idle \n");
			}
			else {
				player1.changeState(charState);
				printf("punching \n");
			}
			break;
		case IDLE_KICK:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			player1.Action("idle", "kick", blendAmount,true);
			if (blendAmount > 0.9f) {
				blendAmount = 0.0f;
				player1.Action("kick", NULL, blendAmount,false);
				charState = KICK_IDLE;
			}
			player1.changeState(charState);
			printf("idle_kick\n");
			break;
		case KICK_IDLE:
			if (player1.getCurrentTime() > 0.7f * player1.getAnimationDuration("kick")) {
				blendAmount += blendRate;
				blendAmount = fmod(blendAmount, 1.0f);
				player1.Action("kick", "idle", blendAmount,true);
				if (blendAmount > 0.9f) {
					blendAmount = 0.0f;
					player1.Action("idle", NULL, blendAmount,false);
					charState = IDLE;
				}
				player1.changeState(charState);
				printf("kick_idle \n");
			}
			else {
				player1.changeState(charState);
				printf("kicking \n");
			}
			break;
		}



		

		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		ourShader.use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		auto transforms = player1.GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i)
			ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);


		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -0.4f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(.5f, .5f, .5f));	// it's a bit too big for our scene, so scale it down
		ourShader.setMat4("model", model);
		player1.draw(ourShader);
		player1.updateAnimation(deltaTime);

		//glm::mat4 model2 = glm::mat4(1.0f);
		//model2 = glm::translate(model2, glm::vec3(0.0f, -0.4f, 0.0f)); // translate it down so it's at the center of the scene
		//model2 = glm::scale(model2, glm::vec3(.5f, .5f, .5f));	// it's a bit too big for our scene, so scale it down
		//ourShader.setMat4("model", model2);
		//guitarModel.Draw(ourShader);



		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

bool checkCollision(const BoxCollider& a, const BoxCollider& b) {
	// Check if boxes overlap on all axes
	bool xCollide = abs(a.center.x - b.center.x) * 2 < (a.size.x + b.size.x);
	bool yCollide = abs(a.center.y - b.center.y) * 2 < (a.size.y + b.size.y);
	bool zCollide = abs(a.center.z - b.center.z) * 2 < (a.size.z + b.size.z);
	return xCollide && yCollide && zCollide;
}

void updateGame() {

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
