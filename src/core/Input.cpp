#include "Input.h"

GLFWwindow* Input::s_window = nullptr;

bool Input::s_keys[GLFW_KEY_LAST + 1] = {};
bool Input::s_previousKeys[GLFW_KEY_LAST + 1] = {};
bool Input::s_buttons[GLFW_MOUSE_BUTTON_LAST + 1] = {};
bool Input::s_previousButtons[GLFW_MOUSE_BUTTON_LAST + 1] = {};

double Input::s_mouseX = 0.0;
double Input::s_mouseY = 0.0;
double Input::s_deltaX = 0.0;
double Input::s_deltaY = 0.0;
double Input::s_scrollAccumulated = 0.0;
double Input::s_scroll = 0.0;
bool Input::s_firstMouse = true;

void Input::init(GLFWwindow* window)
{
    s_window = window;
    glfwSetScrollCallback(window, onScroll);
}

void Input::onScroll(GLFWwindow*, double, double offsetY)
{
    s_scrollAccumulated += offsetY;
}

void Input::update()
{
    if (s_window == nullptr) {
        return;
    }

    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key) {
        s_previousKeys[key] = s_keys[key];
        s_keys[key] = glfwGetKey(s_window, key) == GLFW_PRESS;
    }

    for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; ++button) {
        s_previousButtons[button] = s_buttons[button];
        s_buttons[button] = glfwGetMouseButton(s_window, button) == GLFW_PRESS;
    }

    double x = 0.0;
    double y = 0.0;
    glfwGetCursorPos(s_window, &x, &y);

    if (s_firstMouse) {
        s_mouseX = x;
        s_mouseY = y;
        s_firstMouse = false;
    }

    s_deltaX = x - s_mouseX;
    s_deltaY = y - s_mouseY;
    s_mouseX = x;
    s_mouseY = y;

    s_scroll = s_scrollAccumulated;
    s_scrollAccumulated = 0.0;
}

bool Input::isKeyDown(int key)
{
    return key >= 0 && key <= GLFW_KEY_LAST && s_keys[key];
}

bool Input::wasKeyPressed(int key)
{
    return key >= 0 && key <= GLFW_KEY_LAST && s_keys[key] && !s_previousKeys[key];
}

bool Input::isMouseDown(int button)
{
    return button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST && s_buttons[button];
}

bool Input::wasMousePressed(int button)
{
    return isMouseDown(button) && !s_previousButtons[button];
}

bool Input::wasMouseReleased(int button)
{
    return button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST && !s_buttons[button] && s_previousButtons[button];
}

double Input::mouseDeltaX()
{
    return s_deltaX;
}

double Input::mouseDeltaY()
{
    return s_deltaY;
}

double Input::scrollDelta()
{
    return s_scroll;
}

double Input::mouseX()
{
    return s_mouseX;
}

double Input::mouseY()
{
    return s_mouseY;
}
