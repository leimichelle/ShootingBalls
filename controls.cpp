// Include GLFW
#include <glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

bool hasShot = false;
bool hasChangedView = false;
bool get_hasShot() {
    return hasShot;
}
void set_hasShot(bool flag) {
    hasShot = flag;
}
glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

//Initial light position
glm::vec3 lightPos = glm::vec3(0,4,-3);
// Initial position : on +Z
glm::vec3 position = glm::vec3( 0, 0, 6 );

double old_xpos, old_ypos, new_xpos, new_ypos;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}

glm::vec3 getlightPos() {
    return lightPos;
}

glm::vec3 getcamPos() {
    return position;
}

// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.002f;

//slow down the change in light position.
float movelightspeed = 0.08;



void computeMatricesFromInputs(){

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	//glfwSetCursorPos(window, 1024/2, 768/2);

	// Compute new orientation
    if (hasChangedView) {
        horizontalAngle += mouseSpeed * float(xpos - old_xpos);
        verticalAngle   += mouseSpeed * float(ypos - old_ypos);
        old_xpos = xpos;
        old_ypos = ypos;
    }

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle), 
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f/2.0f), 
		0,
		cos(horizontalAngle - 3.14f/2.0f)
	);
	
	// Up vector
	glm::vec3 up = glm::cross( right, direction );

	// Move forward
	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
		position += direction * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
		position -= direction * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
		position -= right * deltaTime * speed;
	}

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix       = glm::lookAt(
								position,           // Camera is here
								position+direction, // and looks here : at the same position, plus "direction"
								up                  // Head is up (set to 0,-1,0 to look upside-down)
						   );

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}

void computeLightPosFromInputs() {
    //Light's heigh, y value, is fixed (to the ceiling), but we can move it around on the ceiling
    // glfwGetTime is called only once, the first time this function is called
    static double lastTime = glfwGetTime();
    
    // Compute time difference between current and last frame
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);
    // Move forward
    if (glfwGetKey( window,  GLFW_KEY_W ) == GLFW_PRESS){
        lightPos[2] -= deltaTime*movelightspeed;
    }
    // Move backward
    if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
        lightPos[2] += deltaTime*movelightspeed;
    }
    // Strafe right
    if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS){
        lightPos[0] -= deltaTime*movelightspeed;
    }
    // Strafe left
    if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
        lightPos[0] += deltaTime*movelightspeed;
    }
    
    // Strafe left
    if (glfwGetKey( window, GLFW_KEY_R ) == GLFW_PRESS){
        lightPos = glm::vec3(0,4,-3);
    }
    
}

void ScreenPosToWorldRay(
                         int mouseX, int mouseY,             // Mouse position, in pixels, from bottom-left corner of the window
                         int screenWidth, int screenHeight,  // Window size, in pixels
                         glm::mat4 ViewMatrix,               // Camera position and orientation
                         glm::mat4 ProjectionMatrix,         // Camera parameters (ratio, field of view, near and far planes)
                         glm::vec3& out_origin,              // Ouput : Origin of the ray. /!\ Starts at the near plane, so if you want the ray to start at the camera's position instead, ignore this.
                         glm::vec3& out_direction            // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
){
    
    // The ray Start and End positions, in Normalized Device Coordinates
    glm::vec4 lRayStart_NDC(
                            ((float)mouseX/(float)screenWidth  - 0.5f) * 2.0f, // [0,1024] -> [-1,1]
                            ((float)mouseY/(float)screenHeight - 0.5f) * 2.0f, // [0, 768] -> [-1,1]
                            -1.0, // The near plane maps to Z=-1 in Normalized Device Coordinates
                            1.0f
                            );
    glm::vec4 lRayEnd_NDC(
                          /*((float)mouseX/(float)screenWidth  - 0.5f) * 2.0f,
                          ((float)mouseY/(float)screenHeight - 0.5f) * 2.0f,*/
                          0.0,
                          0.0,
                          0.0,
                          1.0f
                          );
    
    
    // The Projection matrix goes from Camera Space to NDC.
    // So inverse(ProjectionMatrix) goes from NDC to Camera Space.
    /*glm::mat4 InverseProjectionMatrix = glm::inverse(ProjectionMatrix);
    
    // The View Matrix goes from World Space to Camera Space.
    // So inverse(ViewMatrix) goes from Camera Space to World Space.
    glm::mat4 InverseViewMatrix = glm::inverse(ViewMatrix);
    
    glm::vec4 lRayStart_camera = InverseProjectionMatrix * lRayStart_NDC;    lRayStart_camera/=lRayStart_camera.w;
    glm::vec4 lRayStart_world  = InverseViewMatrix       * lRayStart_camera; lRayStart_world /=lRayStart_world .w;
    glm::vec4 lRayEnd_camera   = InverseProjectionMatrix * lRayEnd_NDC;      lRayEnd_camera  /=lRayEnd_camera  .w;
    glm::vec4 lRayEnd_world    = InverseViewMatrix       * lRayEnd_camera;   lRayEnd_world   /=lRayEnd_world   .w;*/
    
    
    // Faster way (just one inverse)
    glm::mat4 M = glm::inverse(ProjectionMatrix * ViewMatrix);
    glm::vec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world/=lRayStart_world.w;
    glm::vec4 lRayEnd_world   = M * lRayEnd_NDC  ; lRayEnd_world  /=lRayEnd_world.w;
    
    
    glm::vec3 lRayDir_world(lRayEnd_world - lRayStart_world);
    //lRayDir_world = glm::normalize(lRayDir_world);
    
    
    out_origin = glm::vec3(lRayStart_world);
    out_direction = glm::normalize(lRayDir_world);
}

//return cursor position x,y if user has left clicked the mouse to indicate a shooting
void computeShooting(glm::vec3& ray_origin, glm::vec3& ray_direction){
    double xpos,ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    //The y-coordincate of the screen increases DOWNWARD!! Great! Took me a while to debug
    xpos = 1024. - xpos;
    ScreenPosToWorldRay(
                        xpos, ypos,
                        1024, 768,
                        ViewMatrix,
                        ProjectionMatrix,
                        ray_origin,
                        ray_direction
                        );

}

void mouseCallBack(GLFWwindow *window, int but, int act, int mods){
    if (but==GLFW_MOUSE_BUTTON_LEFT && act==GLFW_RELEASE){
        hasShot = true;
    }
    else if (but==GLFW_MOUSE_BUTTON_RIGHT && act==GLFW_PRESS){
        glfwGetCursorPos(window, &old_xpos, &old_ypos);
        hasChangedView = true;
    }
    else if (but==GLFW_MOUSE_BUTTON_RIGHT && act==GLFW_RELEASE){
        hasChangedView = false;
    }
}