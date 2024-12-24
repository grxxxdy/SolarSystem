#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <cmath>

class Camera
{
public:
	float cameraSpeed, speedMod = 1.0f;
	glm::vec3 cameraPos, startPos;
	glm::vec3 cameraTarget;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	float pitch;
	float yaw;
	float cameraZoom;
	glm::mat4 projection;
	glm::mat4 view;

	Camera(float speed, float zoom, glm::vec3 pos, glm::vec3 target, glm::vec3 front, glm::vec3 up, glm::mat4 projectionMat)
	{
		cameraSpeed = speed;
		cameraPos = pos;
		cameraTarget = target;
		cameraFront = front;
		cameraUp = up;
		cameraZoom = zoom;

		pitch = 0.0f;
		yaw = 90.0f;

		startPos = pos;

		projection = projectionMat;

		updateView();
	}

	void updateView()
	{
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	}

	void moveUp(float deltaTime)
	{
		cameraPos += cameraSpeed * speedMod * cameraFront * deltaTime;
	}

	void moveDown(float deltaTime)
	{
		cameraPos -= cameraSpeed * speedMod * cameraFront * deltaTime;
	}

	void moveLeft(float deltaTime)
	{
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * speedMod * deltaTime;
	}

	void moveRight(float deltaTime)
	{
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * speedMod * deltaTime;
	}

	void speedUp()
	{
		speedMod += 0.1f;

		if (speedMod > 10.0f) speedMod = 10.0f;
	}

	void speedDown()
	{
		speedMod -= 0.1f;
		if (speedMod < 0.1f) speedMod = 0.1f;
	}

	void resetSpeed()
	{
		speedMod = 1.0f;
	}

	void resetPos()
	{
		cameraPos = startPos;
	}

	void rotateCamera(float yawIncr, float pitchIncr)
	{
		yaw += yawIncr;
		pitch += pitchIncr;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 direction;

		direction.y = sin(glm::radians(pitch)) * cameraZoom;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)) * cameraZoom;
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch)) * cameraZoom;

		cameraFront = glm::normalize(direction);
	}
};

#endif