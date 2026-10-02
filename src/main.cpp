// Ponto de entrada (loop principal)

#include <windows.h>
#include "core/Window.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <GL/glu.h>  

void setupProjection(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Configura projeção perspectiva simples (3D)
    float aspect = (float)width / (float)height;
    // fov = 45.0, aspect, zNear = 0.1, zFar = 100.0
    double fov = 45.0;
    double zNear = 0.1;
    double zFar = 100.0;
    double top = zNear * tan(fov * 0.5 * 3.14159265358979323846 / 180.0);
    double right = top * aspect;
    glFrustum(-right, right, -top, top, zNear, zFar);

    glMatrixMode(GL_MODELVIEW);
}

int main() {
    Window window(1280, 720, "Rota das Aguas - Uma Aventura Matematica");

    if (!window.init()) {
        return -1;
    }

    setupProjection(window.getWidth(), window.getHeight());

    // Loop principal da aplicação
    while (!window.shouldClose()) {
        // Clear de Cor e Profundidade
        glClearColor(0.2f, 0.4f, 0.6f, 1.0f); // Cor de fundo azul/água
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Reset da Matriz de Visualização
        glLoadIdentity();

        // Câmera posicionada temporariamente olhando para a origem (Etapa Protótipo)
        gluLookAt(0.0, 3.0, 5.0,  // Posição da Câmera
                  0.0, 0.0, 0.0,  // Ponto para onde olha
                  0.0, 1.0, 0.0); // Vetor UP

        // --- INÍCIO RENDERIZAÇÃO DE TESTE (Exemplo: Embarcação Mundaú simplificada) ---
        glPushMatrix();
            glColor3f(0.6f, 0.3f, 0.1f); // Cor madeira
            glBegin(GL_TRIANGLES);
                glVertex3f(-0.5f, 0.0f,  0.0f);
                glVertex3f( 0.5f, 0.0f,  0.0f);
                glVertex3f( 0.0f, 0.5f, -0.5f);
            glEnd();
        glPopMatrix();
        // --- FIM RENDERIZAÇÃO ---

        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}