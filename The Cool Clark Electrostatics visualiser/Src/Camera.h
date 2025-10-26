#pragma once
#include <glm/glm.hpp>

class Camera {
public:
	glm::vec3 camPos;
	glm::vec3 targetPos;
	glm::vec3 camFront;
	glm::vec3 camRight;
	glm::vec3 camUp;
	glm::vec3 worldUp;

	float camRad = 1;

	float x_rot;
	float y_rot;

	float x_rot_sen;
	float y_rot_sen;
	float speedX;
	float speedY;

	float lastX = 0;
	float lastY = 0;

	Camera(glm::vec3 camPos = glm::vec3(0), glm::vec3 targetPos = glm::vec3(0), glm::vec3 worldUp = glm::vec3(0, 1, 0), float x_rot = 90, float y_rot = 0,
		float x_rot_sen = 1, float y_rot_sen = 1, float speedX = 0.002, float speedY = 0.002);
	glm::mat4 GetViewMatrix();
	void HandleMouseInput(float xpos, float ypos, bool isShift);
	void HandleScroll(float xoffset, float yoffset);
private:
	void CalculateVectors();
};