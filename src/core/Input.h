#ifndef INPUT_H
#define INPUT_H

// Teclado e mouse. Input::update() e chamado uma vez por quadro e guarda o
// estado anterior, para saber quando uma tecla/botao acabou de ser apertado.

#include <GLFW/glfw3.h>

class Input {
public:
    static void init(GLFWwindow* window);
    static void update();

    static bool isKeyDown(int key);
    static bool wasKeyPressed(int key);

    static bool isMouseDown(int button);
    static bool wasMousePressed(int button);
    static bool wasMouseReleased(int button);

    // Movimento do mouse (em pixels) e da roda desde o quadro anterior.
    static double mouseDeltaX();
    static double mouseDeltaY();
    static double scrollDelta();

    // Posicao atual do cursor (pixels, origem no canto superior esquerdo).
    static double mouseX();
    static double mouseY();

private:
    static GLFWwindow* s_window;

    static bool s_keys[GLFW_KEY_LAST + 1];
    static bool s_previousKeys[GLFW_KEY_LAST + 1];
    static bool s_buttons[GLFW_MOUSE_BUTTON_LAST + 1];
    static bool s_previousButtons[GLFW_MOUSE_BUTTON_LAST + 1];

    static double s_mouseX, s_mouseY;
    static double s_deltaX, s_deltaY;
    static double s_scrollAccumulated, s_scroll;
    static bool s_firstMouse;

    static void onScroll(GLFWwindow* window, double offsetX, double offsetY);
};

#endif
