// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}


// Initial position : on +Z
glm::vec3 position = glm::vec3( 0, 5, 5 );
// Initial horizontal angle : toward -Z
float horizontalAngle = glm::pi<float>();
// Initial vertical angle : none
float verticalAngle = - glm::sqrt(2) / 2.0;//glm::pi<float>();
// Initial Field of View
float initialFoV = 45.0f;

float speed = 5.0f; // 3 units / second
float turnSpeed = 0.02f;



void computeMatricesFromInputs(float deltaTime) {
	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	int width, height;
	glfwGetWindowSize(window, &width, &height);
    
    if (glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS){
        speed = 15.0f;
        turnSpeed = 0.04f;
    }
    else {
        speed = 5.0f;
        turnSpeed = 0.02f;
    }
	

	// Cam rotations:
    if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
        verticalAngle += turnSpeed;
    }
    if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
        verticalAngle -= turnSpeed;
    }
    if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
        horizontalAngle -= turnSpeed;
    }
    if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
        horizontalAngle += turnSpeed;
    }

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle), 
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - glm::pi<float>()/2.0f),
		0,
		cos(horizontalAngle - glm::pi<float>()/2.0f)
	);
	
	// Up vector
	glm::vec3 up = glm::cross( right, direction );

	// Move forward
	if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS){
		position += direction * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
		position -= direction * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS){
		position -= right * deltaTime * speed;
	}

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix       = glm::lookAt(
	    position,           // Camera is here
	    position+direction, // and looks here : at the same position, plus "direction"
	    up                  // Head is up (set to 0,-1,0 to look upside-down)
	);
}