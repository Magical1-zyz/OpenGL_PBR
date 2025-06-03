// Camera class implementation
#include "Camera.h"


namespace core {
    // Constructor with position and orientation parameters
    Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position; // Set camera position
        WorldUp = up;       // Set world up vector
        Yaw = yaw;         // Set yaw angle
        Pitch = pitch;     // Set pitch angle
        updateCameraVectors(); // Update camera vectors based on initial parameters
    }

    // Constructor with individual position and orientation components
    Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ); // Set camera position
        WorldUp = glm::vec3(upX, upY, upZ);     // Set world up vector
        Yaw = yaw;                               // Set yaw angle
        Pitch = pitch;                           // Set pitch angle
        updateCameraVectors();                   // Update camera vectors based on initial parameters
    }

    // Returns the view matrix for the camera
    glm::mat4 Camera::GetViewMatrix() const
    {
        return glm::lookAt(Position, Position + Front, Up); // Create view matrix
    }

    // Processes keyboard input for camera movement
    void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime; // Calculate movement velocity
        if (direction == FORWARD)
            Position += Front * velocity; // Move forward
        if (direction == BACKWARD)
            Position -= Front * velocity; // Move backward
        if (direction == LEFT)
            Position -= Right * velocity; // Move left
        if (direction == RIGHT)
            Position += Right * velocity; // Move right
    }

    // Processes mouse movement for camera orientation
    void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
    {
        xoffset *= MouseSensitivity; // Adjust x offset based on sensitivity
        yoffset *= MouseSensitivity; // Adjust y offset based on sensitivity

        Yaw += xoffset; // Update yaw angle
        Pitch += yoffset; // Update pitch angle

        // Constrain pitch angle to avoid flipping
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f; // Limit pitch to 89 degrees
            if (Pitch < -89.0f)
                Pitch = -89.0f; // Limit pitch to -89 degrees
        }

        updateCameraVectors(); // Update camera vectors based on new angles
    }

    // Processes mouse scroll input for zooming
    void Camera::ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset; // Adjust zoom level
        if (Zoom < 1.0f)
            Zoom = 1.0f; // Limit zoom to minimum
        if (Zoom > 45.0f)
            Zoom = 45.0f; // Limit zoom to maximum
    }

    // Updates the camera's front, right, and up vectors based on yaw and pitch
    void Camera::updateCameraVectors()
    {
        glm::vec3 front; // Vector representing the front direction
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch)); // Calculate front x
        front.y = sin(glm::radians(Pitch)); // Calculate front y
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch)); // Calculate front z
        Front = glm::normalize(front); // Normalize front vector
        Right = glm::normalize(glm::cross(Front, WorldUp)); // Calculate right vector
        Up = glm::normalize(glm::cross(Right, Front)); // Calculate up vector
    }
}