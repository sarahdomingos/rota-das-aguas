// Ponto de entrada (loop principal)

#include <windows.h>

#include "core/Window.h"
#include "core/Input.h"
#include "graphics/Renderer.h"

#include <GLFW/glfw3.h>
#include <cmath>
#include <GL/glu.h>

void setupProjection(int width, int height)
{
    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    float aspect =
        static_cast<float>(width) /
        static_cast<float>(height);

    double fov = 45.0;
    double zNear = 0.1;
    double zFar = 100.0;

    double top =
        zNear *
        tan(
            fov *
            0.5 *
            3.14159265358979323846 /
            180.0
        );

    double right = top * aspect;

    glFrustum(
        -right,
        right,
        -top,
        top,
        zNear,
        zFar
    );

    glMatrixMode(GL_MODELVIEW);
}

void processInput(
    Renderer& renderer,
    float deltaTime)
{
    Transform& boat =
        renderer.getBoatTransform();

    const float movementSpeed =
        2.0f * deltaTime;

    const float rotationSpeed =
        90.0f * deltaTime;

    const float scaleFactor =
        1.0f + 0.5f * deltaTime;

    // ------------------------------------------
    // Movimento
    // ------------------------------------------

    if (Input::isKeyDown(GLFW_KEY_UP)) {

        boat.translate(
            0.0f,
            0.0f,
            -movementSpeed
        );
    }

    if (Input::isKeyDown(GLFW_KEY_DOWN)) {

        boat.translate(
            0.0f,
            0.0f,
            movementSpeed
        );
    }

    if (Input::isKeyDown(GLFW_KEY_LEFT)) {

        boat.translate(
            -movementSpeed,
            0.0f,
            0.0f
        );
    }

    if (Input::isKeyDown(GLFW_KEY_RIGHT)) {

        boat.translate(
            movementSpeed,
            0.0f,
            0.0f
        );
    }

    // ------------------------------------------
    // Rotação
    // ------------------------------------------

    if (Input::isKeyDown(GLFW_KEY_Q)) {

        boat.rotate(
            0.0f,
            rotationSpeed,
            0.0f
        );
    }

    if (Input::isKeyDown(GLFW_KEY_E)) {

        boat.rotate(
            0.0f,
            -rotationSpeed,
            0.0f
        );
    }

    // ------------------------------------------
    // Escala
    // ------------------------------------------

    if (Input::isKeyDown(GLFW_KEY_Z)) {

        boat.scale(
            1.0f / scaleFactor,
            1.0f / scaleFactor,
            1.0f / scaleFactor
        );
    }

    if (Input::isKeyDown(GLFW_KEY_X)) {

        boat.scale(
            scaleFactor,
            scaleFactor,
            scaleFactor
        );
    }
}

int main()
{
    Window window(
        1280,
        720,
        "Rota das Aguas - Uma Aventura Matematica"
    );

    if (!window.init()) {
        return -1;
    }

    setupProjection(
        window.getWidth(),
        window.getHeight()
    );

    Renderer renderer;

    if (!renderer.loadBoat()) {
        return -1;
    }

    Input::update(
        window.getNativeWindow()
    );

    double previousTime =
        glfwGetTime();

    while (!window.shouldClose()) {

        // ------------------------------------------
        // Delta time
        // ------------------------------------------

        double currentTime =
            glfwGetTime();

        float deltaTime =
            static_cast<float>(
                currentTime - previousTime
            );

        previousTime = currentTime;

        // Evita movimentos gigantescos caso
        // a aplicação congele por algum motivo.
        if (deltaTime > 0.1f) {
            deltaTime = 0.1f;
        }

        // ------------------------------------------
        // Entrada
        // ------------------------------------------

        processInput(
            renderer,
            deltaTime
        );

        // ------------------------------------------
        // Limpa a tela
        // ------------------------------------------

        glClearColor(
            0.2f,
            0.4f,
            0.6f,
            1.0f
        );

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT
        );

        // ------------------------------------------
        // Câmera
        // ------------------------------------------

        glMatrixMode(GL_MODELVIEW);

        glLoadIdentity();

        gluLookAt(
            0.0, 1.5, 5.0,
            0.0, 0.0, 0.0,
            0.0, 1.0, 0.0
        );

        // ------------------------------------------
        // Renderização
        // ------------------------------------------

        renderer.render();

        // ------------------------------------------
        // Atualização
        // ------------------------------------------

        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}