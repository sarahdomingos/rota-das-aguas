#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>

class Input {
public:
    static void update(GLFWwindow* window);

    static bool isKeyDown(int key);

private:
    static GLFWwindow* s_window;
};

#endif