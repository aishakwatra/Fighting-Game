#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "animator.h"
#include "model_animation.h"

#include <iostream>

enum AnimStateP1 {
	P1_IDLE = 1,
	P1_IDLE_PUNCH,
	P1_PUNCH_IDLE,
	P1_IDLE_KICK,
	P1_KICK_IDLE,
	P1_IDLE_BLOCK,
	P1_BLOCK_IDLE,
	P1_IDLE_WALK_FRONT,
	P1_IDLE_WALK_BACK,
	P1_WALK_FRONT_IDLE,
	P1_WALK_BACK_IDLE,
	P1_WALK_FRONT,
	P1_WALK_BACK
};

enum AnimStateP2 {
	P2_IDLE = 1,
	P2_IDLE_PUNCH,
	P2_PUNCH_IDLE,
	P2_IDLE_KICK,
	P2_KICK_IDLE,
	P2_IDLE_BLOCK,
	P2_BLOCK_IDLE,
	P2_IDLE_WALK_FRONT,
	P2_IDLE_WALK_BACK,
	P2_WALK_FRONT_IDLE,
	P2_WALK_BACK_IDLE,
	P2_WALK_FRONT,
	P2_WALK_BACK
};

struct Capsule {
	glm::vec3 pointA;  // Top center of the capsule
	glm::vec3 pointB;  // Bottom center of the capsule
	float radius;
};

Capsule player1Capsule;
Capsule player2Capsule;

int player1Health = 150;
int player2Health = 150;



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void UpdateStateP1(GLFWwindow* window, Animator& animator, AnimStateP1& charState, float& blendAmount);
void UpdateStateP2(GLFWwindow* window, Animator& animator, AnimStateP2& charState, float& blendAmount);

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// camera
Camera camera(glm::vec3(-3.821124f, 0.010320f, 1.682272f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

ModelAnim player1;
Animation idleAnimationP1;
Animation walkFrontAnimationP1;
Animation walkBackAnimationP1;
Animation punchAnimationP1;
Animation kickAnimationP1;
Animation blockAnimationP1;

ModelAnim player2;
Animation idleAnimationP2;
Animation walkFrontAnimationP2;
Animation walkBackAnimationP2;
Animation punchAnimationP2;
Animation kickAnimationP2;
Animation blockAnimationP2;

glm::vec3 player1Position = glm::vec3(0.0f, -0.4f, 0.0f);
glm::vec3 player2Position = glm::vec3(0.0f, -0.4f, 3.0f);
float moveSpeed = 0.5f;
int P1punchDamage = 1;
int P1KickDamage = 2;
int P2punchDamage = 3;
int P2KickDamage = 4;
Animator player1_animator(&idleAnimationP1);
Animator player2_animator(&idleAnimationP2);
enum AnimStateP1 P1charState = P1_IDLE;
enum AnimStateP2 P2charState = P2_IDLE;
float blendAmountP1 = 0.0f;
float blendAmountP2 = 0.0f;
float blendRate = 0.055f;

void updateCapsules() {
	// Adjust these values based on the character's current pose and animation
	player1Capsule.pointA = player1Position + glm::vec3(0.0f, 1.2f, 0.0f); // example values
	player1Capsule.pointB = player1Position + glm::vec3(0.0f, 0.4f, 0.0f);
	player1Capsule.radius = 0.4f;

	player2Capsule.pointA = player2Position + glm::vec3(0.0f, 1.2f, 0.0f);
	player2Capsule.pointB = player2Position + glm::vec3(0.0f, 0.4f, 0.0f);
	player2Capsule.radius = 0.4f;
}

float segmentSegmentDistance(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& q1, const glm::vec3& q2) {
	glm::vec3 u = p2 - p1;
	glm::vec3 v = q2 - q1;
	glm::vec3 w = p1 - q1;
	float a = glm::dot(u, u); // always >= 0
	float b = glm::dot(u, v);
	float c = glm::dot(v, v); // always >= 0
	float d = glm::dot(u, w);
	float e = glm::dot(v, w);
	float D = a * c - b * b; // always >= 0
	float sc, sN, sD = D;    // sc = sN / sD, default sD = D >= 0
	float tc, tN, tD = D;    // tc = tN / tD, default tD = D >= 0

	// compute the line parameters of the two closest points
	if (D < 1e-8) { // the lines are almost parallel
		sN = 0.0; // force using point P0 on segment S1
		sD = 1.0; // to prevent possible division by 0.0 later
		tN = e;
		tD = c;
	}
	else { // get the closest points on the infinite lines
		sN = (b * e - c * d);
		tN = (a * e - b * d);
		if (sN < 0.0) { // sc < 0 => the s=0 edge is visible
			sN = 0.0;
			tN = e;
			tD = c;
		}
		else if (sN > sD) { // sc > 1  => the s=1 edge is visible
			sN = sD;
			tN = e + b;
			tD = c;
		}
	}

	if (tN < 0.0) { // tc < 0 => the t=0 edge is visible
		tN = 0.0;
		// recompute sc for this edge
		if (-d < 0.0)
			sN = 0.0;
		else if (-d > a)
			sN = sD;
		else {
			sN = -d;
			sD = a;
		}
	}
	else if (tN > tD) { // tc > 1  => the t=1 edge is visible
		tN = tD;
		// recompute sc for this edge
		if ((-d + b) < 0.0)
			sN = 0;
		else if ((-d + b) > a)
			sN = sD;
		else {
			sN = (-d + b);
			sD = a;
		}
	}
	// finally do the division to get sc and tc
	sc = (abs(sN) < 1e-8 ? 0.0 : sN / sD);
	tc = (abs(tN) < 1e-8 ? 0.0 : tN / tD);

	// get the difference of the two closest points
	glm::vec3 dP = w + (sc * u) - (tc * v); // =  S1(sc) - S2(tc)

	return glm::length(dP); // return the closest distance
}

bool checkCapsuleCollision(const Capsule& capsule1, const Capsule& capsule2) {
	float distance = segmentSegmentDistance(capsule1.pointA, capsule1.pointB, capsule2.pointA, capsule2.pointB);
	return distance < (capsule1.radius + capsule2.radius);
}

void handleCollisions() {
	if (checkCapsuleCollision(player1Capsule, player2Capsule)) {
		glm::vec3 collisionNormal = glm::normalize(player2Position - player1Position);
		float overlap = (player1Capsule.radius + player2Capsule.radius) - glm::distance(player1Capsule.pointB, player2Capsule.pointB);
		float pushFactor = 0.1f;

		// Push both players away from each other
		player1Position -= collisionNormal * (overlap * pushFactor) * 0.5f;
		player2Position += collisionNormal * (overlap * pushFactor) * 0.5f;

		// Handling for Player 1 attacking
		if (P1charState == P1_PUNCH_IDLE || P1charState == P1_KICK_IDLE) {
			int damage = 0;
			if (P1charState == P1_PUNCH_IDLE) {
				damage = P1punchDamage;
			}
			else if (P1charState == P1_KICK_IDLE) {
				damage = P1KickDamage;
			}

			// Check if Player 2 is currently blocking
			if (P2charState == P2_IDLE_BLOCK) {
				std::cout << "Player 2 blocked the attack!" << std::endl;
			}
			else {
				// Apply damage to Player 2 if not blocking
				player2Health -= damage;
				std::cout << "Player 2 hit! Health now: " << player2Health << std::endl;
				if (player2Health <= 0) {
					std::cout << "Player 2 has been defeated!" << std::endl;
				}
			}
		}

		// Handling for Player 2 attacking
		if (P2charState == P2_PUNCH_IDLE || P2charState == P2_KICK_IDLE) {
			int damage = 0;
			if (P2charState == P2_PUNCH_IDLE) {
				damage = P2punchDamage;
			}
			else if (P2charState == P2_KICK_IDLE) {
				damage = P2KickDamage;
			}

			// Check if Player 1 is currently blocking
			if (P1charState == P1_IDLE_BLOCK) {
				std::cout << "Player 1 blocked the attack!" << std::endl;
			}
			else {
				// Apply damage to Player 1 if not blocking
				player1Health -= damage;
				std::cout << "Player 1 hit! Health now: " << player1Health << std::endl;
				if (player1Health <= 0) {
					std::cout << "Player 1 has been defeated!" << std::endl;
				}
			}
		}
	}
}



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
	//glfwSetCursorPosCallback(window, mouse_callback);
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
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader ourShader("anim_model.vs", "anim_model.fs");

	// load models
	// -----------
	// idle 3.3, walk 2.06, run 0.83, punch 1.03, kick 1.6
	
	player1.loadModel("Object/Vegas/Big Vegas.dae");
	idleAnimationP1.loadAnimation("Object/Vegas/Idle.dae", &player1);
	walkFrontAnimationP1.loadAnimation("Object/Vegas/WalkForward.dae", &player1,1.8f);
	walkBackAnimationP1.loadAnimation("Object/Vegas/WalkBack.dae", &player1,1.5f);
	punchAnimationP1.loadAnimation("Object/Vegas/Punch Combo.dae", &player1,1.5f);
	kickAnimationP1.loadAnimation("Object/Vegas/Kicking.dae", &player1,1.5f);
	blockAnimationP1.loadAnimation("Object/Vegas/Bouncing Fight Idle.dae", &player1,1.2f);

	player2.loadModel("Object/Wrestler/Ch43_nonPBR.dae");
	idleAnimationP2.loadAnimation("Object/Wrestler/Idle.dae", &player2);
	walkFrontAnimationP2.loadAnimation("Object/Wrestler/Walking.dae", &player2, 1.5f);
	walkBackAnimationP2.loadAnimation("Object/Wrestler/Standing Walk Back.dae", &player2, 1.7f);
	punchAnimationP2.loadAnimation("Object/Wrestler/Cross Punch.dae", &player2, 1.7f);
	kickAnimationP2.loadAnimation("Object/Wrestler/Mma Kick.dae", &player2, 1.8f);
	blockAnimationP2.loadAnimation("Object/Wrestler/Center Block.dae", &player2, 1.5f);


	camera.Yaw = -1.000028f;
	camera.Pitch = 8.200005f;

	camera.updateCameraVectors();
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
		updateCapsules();
		handleCollisions();

		// input
		// -----
		//processInput(window);
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);


		UpdateStateP1(window, player1_animator, P1charState, blendAmountP1);
		UpdateStateP2(window, player2_animator, P2charState, blendAmountP2);


		player1_animator.UpdateAnimation(deltaTime);
		player2_animator.UpdateAnimation(deltaTime);

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

		// Before drawing player 1
		ourShader.use();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);
		glm::mat4 modelP1 = glm::mat4(1.0f);
		modelP1 = glm::translate(modelP1, player1Position);
		modelP1 = glm::scale(modelP1, glm::vec3(.55f, .55f, .55f));
		ourShader.setMat4("model", modelP1);

		auto transformsP1 = player1_animator.GetFinalBoneMatrices();
		for (int i = 0; i < transformsP1.size(); ++i) {
			ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transformsP1[i]);
		}
		player1.Draw(ourShader);

		// Before drawing player 2
		ourShader.use();  // Ensure shader is active when setting uniforms
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);
		glm::mat4 modelP2 = glm::mat4(1.0f);
		modelP2 = glm::translate(modelP2, player2Position);
		modelP2 = glm::rotate(modelP2, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate 180 degrees around the y-axis
		modelP2 = glm::scale(modelP2, glm::vec3(.6f, .6f, .6f));
		ourShader.setMat4("model", modelP2);

		auto transformsP2 = player2_animator.GetFinalBoneMatrices();
		for (int i = 0; i < transformsP2.size(); ++i) {
			ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transformsP2[i]);
		}
		player2.Draw(ourShader);

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
	

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		player1_animator.PlayAnimation(&idleAnimationP1, NULL, 0.0f, 0.0f, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		player1_animator.PlayAnimation(&walkFrontAnimationP1, NULL, 0.0f, 0.0f, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		player1_animator.PlayAnimation(&punchAnimationP1, NULL, 0.0f, 0.0f, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		player1_animator.PlayAnimation(&kickAnimationP1, NULL, 0.0f, 0.0f, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
		player1_animator.PlayAnimation(&walkBackAnimationP1, NULL, 0.0f, 0.0f, 0.0f);
	
	
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	printf("x: %f, y: %f, z: %f \n", camera.Position.x, camera.Position.y, camera.Position.z);
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}


void UpdateStateP1(GLFWwindow* window, Animator& animator, AnimStateP1& charState, float& blendAmount)
{
	switch (charState) {
	case P1_IDLE:
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			blendAmount = 0.0f;
			player1Position.z += moveSpeed * deltaTime;
			animator.PlayAnimation(&idleAnimationP1, &walkFrontAnimationP1, animator.m_CurrentTime, 0.0f, blendAmount);
			charState = P1_IDLE_WALK_FRONT;
		}
		else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			blendAmount = 0.0f;
			player1Position.z -= moveSpeed * deltaTime;
			animator.PlayAnimation(&idleAnimationP1, &walkBackAnimationP1, animator.m_CurrentTime, 0.0f, blendAmount);
			charState = P1_IDLE_WALK_BACK;
		}


		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			blendAmount = 0.0f;
			animator.PlayAnimation(&idleAnimationP1, &blockAnimationP1, animator.m_CurrentTime, 0.0f, blendAmount);
			charState = P1_IDLE_BLOCK;
		}
		if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
			blendAmount = 0.0f;
			animator.PlayAnimation(&idleAnimationP1, &punchAnimationP1, animator.m_CurrentTime, 0.0f, blendAmount);
			charState = P1_IDLE_PUNCH;
		}
		if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
			blendAmount = 0.0f;
			animator.PlayAnimation(&idleAnimationP1, &kickAnimationP1, animator.m_CurrentTime, 0.0f, blendAmount);
			charState = P1_IDLE_KICK;
		}
		//printf("idle \n");
		break;
	case P1_IDLE_WALK_FRONT:
		player1Position.z += moveSpeed * deltaTime;
		blendAmount += blendRate;
		blendAmount = fmod(blendAmount, 1.0f);
		animator.PlayAnimation(&idleAnimationP1, &walkFrontAnimationP1, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
		if (blendAmount > 0.9f) {
			blendAmount = 0.0f;
			float startTime = animator.m_CurrentTime2;
			animator.PlayAnimation(&walkFrontAnimationP1, NULL, startTime, 0.0f, blendAmount);
			charState = P1_WALK_FRONT;
		}
		//printf("idle_walk_front \n");
		break;
	case P1_IDLE_WALK_BACK:
		player1Position.z -= moveSpeed * deltaTime;
		blendAmount += blendRate;
		blendAmount = fmod(blendAmount, 1.0f);
		animator.PlayAnimation(&idleAnimationP1, &walkBackAnimationP1, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
		if (blendAmount > 0.9f) {
			blendAmount = 0.0f;
			float startTime = animator.m_CurrentTime2;
			animator.PlayAnimation(&walkBackAnimationP1, NULL, startTime, 0.0f, blendAmount);
			charState = P1_WALK_BACK;
		}
		//printf("idle_walk_back \n");
		break;
	case P1_WALK_FRONT:
		player1Position.z += moveSpeed * deltaTime;
		animator.PlayAnimation(&walkFrontAnimationP1, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
		if (glfwGetKey(window, GLFW_KEY_D) != GLFW_PRESS) {
			charState = P1_WALK_FRONT_IDLE;

		}
		//printf("walking_front\n");
		break;
	case P1_WALK_BACK:
		player1Position.z -= moveSpeed * deltaTime;
		animator.PlayAnimation(&walkBackAnimationP1, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
		if (glfwGetKey(window, GLFW_KEY_A) != GLFW_PRESS) {
			charState = P1_WALK_BACK_IDLE;

		}
		//printf("walking_back\n");
		break;
	case P1_WALK_FRONT_IDLE:
		blendAmount += blendRate;
		blendAmount = fmod(blendAmount, 1.0f);
		animator.PlayAnimation(&walkFrontAnimationP1, &idleAnimationP1, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
		if (blendAmount > 0.9f) {
			blendAmount = 0.0f;
			float startTime = animator.m_CurrentTime2;
			animator.PlayAnimation(&idleAnimationP1, NULL, startTime, 0.0f, blendAmount);
			charState = P1_IDLE;
		}
		//printf("walk_front_idle \n");
		break;
	case P1_WALK_BACK_IDLE:
		blendAmount += blendRate;
		blendAmount = fmod(blendAmount, 1.0f);
		animator.PlayAnimation(&walkBackAnimationP1, &idleAnimationP1, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
		if (blendAmount > 0.9f) {
			blendAmount = 0.0f;
			float startTime = animator.m_CurrentTime2;
			animator.PlayAnimation(&idleAnimationP1, NULL, startTime, 0.0f, blendAmount);
			charState = P1_IDLE;
		}
		//printf("walk_back_idle \n");
		break;
	case P1_IDLE_PUNCH:
		blendAmount += blendRate;
		blendAmount = fmod(blendAmount, 1.0f);
		animator.PlayAnimation(&idleAnimationP1, &punchAnimationP1, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
		if (blendAmount > 0.9f) {
			blendAmount = 0.0f;
			float startTime = animator.m_CurrentTime2;
			animator.PlayAnimation(&punchAnimationP1, NULL, startTime, 0.0f, blendAmount);
			charState = P1_PUNCH_IDLE;
		}
		//printf("idle_punch\n");
		break;
	case P1_PUNCH_IDLE:
		if (animator.m_CurrentTime > 0.6 * (punchAnimationP1.GetDuration() * 1.0f)) {
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&punchAnimationP1, &idleAnimationP1, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.8f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&idleAnimationP1, NULL, startTime, 0.0f, blendAmount);
				charState = P1_IDLE;
			}
			//printf("punch_idle \n");
		}
		else {
			// punching
			//printf("punching \n");
		}
		break;
	case P1_IDLE_KICK:
		blendAmount += blendRate;
		blendAmount = fmod(blendAmount, 1.0f);
		animator.PlayAnimation(&idleAnimationP1, &kickAnimationP1, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
		if (blendAmount > 0.9f) {
			blendAmount = 0.0f;
			float startTime = animator.m_CurrentTime2;
			animator.PlayAnimation(&kickAnimationP1, NULL, startTime, 0.0f, blendAmount);
			charState = P1_KICK_IDLE;
		}
		//printf("idle_kick\n");
		break;
	case P1_KICK_IDLE:
		if (animator.m_CurrentTime > 0.7f * kickAnimationP1.GetDuration()) {
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&kickAnimationP1, &idleAnimationP1, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.7f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&idleAnimationP1, NULL, startTime, 0.0f, blendAmount);
				charState = P1_IDLE;
			}
			//printf("kick_idle \n");
		}
		else {
			// punching
			//printf("kicking \n");
		}
		break;
	case P1_IDLE_BLOCK:
		blendAmount += blendRate;
		blendAmount = fmod(blendAmount, 1.0f);
		animator.PlayAnimation(&idleAnimationP1, &blockAnimationP1, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
		if (blendAmount > 0.9f) {
			blendAmount = 0.0f;
			float startTime = animator.m_CurrentTime2;
			animator.PlayAnimation(&blockAnimationP1, NULL, startTime, 0.0f, blendAmount);
			charState = P1_BLOCK_IDLE;
		}
		//printf("idle_block\n");
		break;
	case P1_BLOCK_IDLE:
		if (animator.m_CurrentTime > 0.7f * (blockAnimationP1.GetDuration() * 0.5f)) {
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&blockAnimationP1, &idleAnimationP1, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.9f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&idleAnimationP1, NULL, startTime, 0.0f, blendAmount);
				charState = P1_IDLE;
			}
			//printf("block_idle \n");
		}
		else {
			// punching
			//printf("blocking \n");
		}
		break;
	}
}


	void UpdateStateP2(GLFWwindow* window, Animator& animator, AnimStateP2& charState, float& blendAmount)
	{
		switch (charState) {
		case P2_IDLE:
			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
				blendAmount = 0.0f;
				player2Position.z -= moveSpeed * deltaTime;
				animator.PlayAnimation(&idleAnimationP2, &walkFrontAnimationP2, animator.m_CurrentTime, 0.0f, blendAmount);
				charState = P2_IDLE_WALK_FRONT;
			}
			else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
				blendAmount = 0.0f;
				player2Position.z += moveSpeed * deltaTime;
				animator.PlayAnimation(&idleAnimationP2, &walkBackAnimationP2, animator.m_CurrentTime, 0.0f, blendAmount);
				charState = P2_IDLE_WALK_BACK;
			}


			if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
				blendAmount = 0.0f;
				animator.PlayAnimation(&idleAnimationP2, &blockAnimationP2, animator.m_CurrentTime, 0.0f, blendAmount);
				charState = P2_IDLE_BLOCK;
			}
			if (glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS) {
				blendAmount = 0.0f;
				animator.PlayAnimation(&idleAnimationP2, &punchAnimationP2, animator.m_CurrentTime, 0.0f, blendAmount);
				charState = P2_IDLE_PUNCH;
			}
			if (glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS) {
				blendAmount = 0.0f;
				animator.PlayAnimation(&idleAnimationP2, &kickAnimationP2, animator.m_CurrentTime, 0.0f, blendAmount);
				charState = P2_IDLE_KICK;
			}
			//printf("idle \n");
			break;
		case  P2_IDLE_WALK_FRONT:
			player2Position.z -= moveSpeed * deltaTime;
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&idleAnimationP2, &walkFrontAnimationP2, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.9f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&walkFrontAnimationP2, NULL, startTime, 0.0f, blendAmount);
				charState = P2_WALK_FRONT;
			}
			//printf("idle_walk_front \n");
			break;
		case  P2_IDLE_WALK_BACK:
			player2Position.z += moveSpeed * deltaTime;
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&idleAnimationP2, &walkBackAnimationP2, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.8f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&walkBackAnimationP2, NULL, startTime, 0.0f, blendAmount);
				charState = P2_WALK_BACK;
			}
			//printf("idle_walk_back \n");
			break;
		case  P2_WALK_FRONT:
			player2Position.z -= moveSpeed * deltaTime;
			animator.PlayAnimation(&walkFrontAnimationP2, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (glfwGetKey(window, GLFW_KEY_LEFT) != GLFW_PRESS) {
				charState = P2_WALK_FRONT_IDLE;

			}
			//printf("walking_front\n");
			break;
		case  P2_WALK_BACK:
			player2Position.z += moveSpeed * deltaTime;
			animator.PlayAnimation(&walkBackAnimationP2, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (glfwGetKey(window, GLFW_KEY_RIGHT) != GLFW_PRESS) {
				charState = P2_WALK_BACK_IDLE;

			}
			//printf("walking_back\n");
			break;
		case  P2_WALK_FRONT_IDLE:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&walkFrontAnimationP2, &idleAnimationP2, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.8f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&idleAnimationP2, NULL, startTime, 0.0f, blendAmount);
				charState = P2_IDLE;
			}
			//printf("walk_front_idle \n");
			break;
		case  P2_WALK_BACK_IDLE:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&walkBackAnimationP2, &idleAnimationP2, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.9f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&idleAnimationP2, NULL, startTime, 0.0f, blendAmount);
				charState = P2_IDLE;
			}
			//printf("walk_back_idle \n");
			break;
		case  P2_IDLE_PUNCH:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&idleAnimationP2, &punchAnimationP2, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.8f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&punchAnimationP2, NULL, startTime, 0.0f, blendAmount);
				charState = P2_PUNCH_IDLE;
			}
			//printf("idle_punch\n");
			break;
		case  P2_PUNCH_IDLE:
			if (animator.m_CurrentTime > 0.65 * punchAnimationP2.GetDuration()) {
				blendAmount += blendRate;
				blendAmount = fmod(blendAmount, 1.0f);
				animator.PlayAnimation(&punchAnimationP2, &idleAnimationP2, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
				if (blendAmount > 0.7f) {
					blendAmount = 0.0f;
					float startTime = animator.m_CurrentTime2;
					animator.PlayAnimation(&idleAnimationP2, NULL, startTime, 0.0f, blendAmount);
					charState = P2_IDLE;
				}
				//printf("punch_idle \n");
			}
			else {
				// punching
				//printf("punching \n");
			}
			break;
		case  P2_IDLE_KICK:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&idleAnimationP2, &kickAnimationP2, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.9f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&kickAnimationP2, NULL, startTime, 0.0f, blendAmount);
				charState = P2_KICK_IDLE;
			}
			//printf("idle_kick\n");
			break;
		case  P2_KICK_IDLE:
			if (animator.m_CurrentTime > 0.6f * kickAnimationP2.GetDuration()) {
				blendAmount += blendRate;
				blendAmount = fmod(blendAmount, 1.0f);
				animator.PlayAnimation(&kickAnimationP2, &idleAnimationP2, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
				if (blendAmount > 0.7f) {
					blendAmount = 0.0f;
					float startTime = animator.m_CurrentTime2;
					animator.PlayAnimation(&idleAnimationP2, NULL, startTime, 0.0f, blendAmount);
					charState = P2_IDLE;
				}
				//printf("kick_idle \n");
			}
			else {
				// punching
				//printf("kicking \n");
			}
			break;
		case  P2_IDLE_BLOCK:
			blendAmount += blendRate;
			blendAmount = fmod(blendAmount, 1.0f);
			animator.PlayAnimation(&idleAnimationP2, &blockAnimationP2, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			if (blendAmount > 0.9f) {
				blendAmount = 0.0f;
				float startTime = animator.m_CurrentTime2;
				animator.PlayAnimation(&blockAnimationP2, NULL, startTime, 0.0f, blendAmount);
				charState = P2_BLOCK_IDLE;
			}
			//printf("idle_block\n");
			break;
		case  P2_BLOCK_IDLE:
			if (animator.m_CurrentTime > 0.7f * (blockAnimationP2.GetDuration() * 0.5f)) {
				blendAmount += blendRate;
				blendAmount = fmod(blendAmount, 1.0f);
				animator.PlayAnimation(&blockAnimationP2, &idleAnimationP2, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
				if (blendAmount > 0.9f) {
					blendAmount = 0.0f;
					float startTime = animator.m_CurrentTime2;
					animator.PlayAnimation(&idleAnimationP2, NULL, startTime, 0.0f, blendAmount);
					charState = P2_IDLE;
				}
				//printf("block_idle \n");
			}
			else {
				// punching
				//printf("blocking \n");
			}
			break;
		}
	}
	
	


