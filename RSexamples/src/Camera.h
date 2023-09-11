#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define _USE_MATH_DEFINES
#include <math.h>

class Camera {
private:
	float fov = 45.f;
	float znear = 0.01f, zfar = 1000.0f;

	void updateViewMatrix();

public:
	enum CameraType { lookat, firstperson };
	CameraType type = CameraType::lookat;
	glm::vec3 rotation = glm::vec3();
	glm::vec3 position = glm::vec3();
	glm::vec4 viewPos = glm::vec4();

	float rotationSpeed = 1.0f;
	float movementSpeed = 1.0f;

	bool updated = false;
	bool flipY = false;

	struct
	{
		glm::mat4 perspective;
		glm::mat4 view;
	} matrices;

	struct
	{
		bool left = false;
		bool right = false;
		bool up = false;
		bool down = false;
	} keys;

	bool isAnimating() const;
	float getNearClip() const;
	float getFarClip() const;
	void setPerspective(float fov, float aspect, float znear, float zfar);
	void updateAspectRatio(float aspect);
	void setPosition(glm::vec3 position);
	void setRotation(glm::vec3 rotation);
	void rotate(glm::vec3 delta);
	void setTranslation(glm::vec3 translation);
	void translate(glm::vec3 delta);
	void setRotationSpeed(float rotationSpeed);
	void setMovementSpeed(float movementSpeed);
	void update(float deltaTime);
	bool updatePad(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime);

	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;
};

