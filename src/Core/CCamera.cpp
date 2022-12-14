#include "Common.h"
#include "CCamera.h"

CCamera::CCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
	: Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM), MovementMultiplier(1.0f)
{
	//std::cout << "CAM CREATE A" << std::endl;
	Position = position;
	WorldUp = up;
	Yaw = yaw;
	Pitch = pitch;
	updateCameraVectors();
}

CCamera::CCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
	: Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM), MovementMultiplier(1.0f)
{
	//std::cout << "CAM CREATE B" << std::endl;
	Position = glm::vec3(posX, posY, posZ);
	WorldUp = glm::vec3(upX, upY, upZ);
	Yaw = yaw;
	Pitch = pitch;
	updateCameraVectors();
}

glm::mat4 CCamera::GetViewMatrix()
{
	return glm::lookAt(Position, Position + Front, Up);
}

void CCamera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
	//std::cout << "CAM KEYBOARD " << direction << " " << deltaTime << std::endl;
	float velocity = MovementSpeed * deltaTime * MovementMultiplier;
	if (direction == FORWARD)
		Position += Front * velocity;
	if (direction == BACKWARD)
		Position -= Front * velocity;
	if (direction == LEFT)
		Position -= Right * velocity;
	if (direction == RIGHT)
		Position += Right * velocity;
}

void CCamera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
	//std::cout << "CAM MOUSE " << xoffset << " " << yoffset << std::endl;
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;

	Yaw += xoffset;
	Pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;
	}

	// Update Front, Right and Up Vectors using the updated Euler angles
	updateCameraVectors();
}

void CCamera::ProcessMouseScroll(float yoffset)
{
	//std::cout << "CAM SCROLL " << yoffset << std::endl;
	if (Zoom >= 1.0f && Zoom <= 90.0f)
		Zoom -= yoffset;
	if (Zoom <= 1.0f)
		Zoom = 1.0f;
	if (Zoom >= 90.0f)
		Zoom = 90.0f;
}

void CCamera::Reset(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
{
	//std::cout << "CAM RESET" << std::endl;
	Position = position;
	WorldUp = up;
	Yaw = yaw;
	Pitch = pitch;
	updateCameraVectors();
}

void CCamera::updateCameraVectors()
{
	// Calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);
	// Also re-calculate the Right and Up vector
	Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	Up = glm::normalize(glm::cross(Right, Front));
}
