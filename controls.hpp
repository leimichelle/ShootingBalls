#ifndef CONTROLS_HPP
#define CONTROLS_HPP
// Include GLFW
#include <glfw3.h>

void computeMatricesFromInputs();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
glm::vec3 getlightPos();
void computeLightPosFromInputs();
bool get_hasShot();
void set_hasShot(bool flag);
void computeShooting(glm::vec3& ray_origin, glm::vec3& ray_direction);
void mouseCallBack(GLFWwindow *window, int but, int act, int mods);
#endif