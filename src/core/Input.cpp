#include "Input.h"

GLFWwindow* Input::s_window = nullptr;

void Input::update(GLFWwindow* window)
{
    s_window = window;
}

bool Input::isKeyDown(int key)
{
    if (s_window == nullptr) {
        return false;
    }

    return glfwGetKey(
        s_window,
        key
    ) == GLFW_PRESS;
}