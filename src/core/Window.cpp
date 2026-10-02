#include "Window.h"
#include <iostream>

Window::Window(int width, int height, const std::string &title)
    : m_width(width), m_height(height), m_title(title), m_window(nullptr) {}

Window::~Window()
{
    if (m_window)
    {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

bool Window::init()
{
    if (!glfwInit())
    {
        std::cerr << "Falha ao inicializar o GLFW!" << std::endl;
        return false;
    }

    // Configuração para uso do OpenGL 2.0 (Pipeline Fixo)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!m_window)
    {
        std::cerr << "Falha ao criar a janela GLFW!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // Ativa V-Sync

    // Configurações padrão iniciais do OpenGL 2.0
    glEnable(GL_DEPTH_TEST); // Habilita o teste de profundidade (Buffer de Z)
    glDepthFunc(GL_LESS);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightPosition[] = {
        4.0f,
        6.0f,
        4.0f,
        1.0f};

    GLfloat lightAmbient[] = {
        0.25f,
        0.25f,
        0.25f,
        1.0f};

    GLfloat lightDiffuse[] = {
        0.8f,
        0.8f,
        0.8f,
        1.0f};

    glLightfv(
        GL_LIGHT0,
        GL_POSITION,
        lightPosition);

    glLightfv(
        GL_LIGHT0,
        GL_AMBIENT,
        lightAmbient);

    glLightfv(
        GL_LIGHT0,
        GL_DIFFUSE,
        lightDiffuse);

    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(
        GL_FRONT_AND_BACK,
        GL_AMBIENT_AND_DIFFUSE);

    // Configura a Viewport inicial
    glViewport(0, 0, m_width, m_height);

    return true;
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(m_window);
}

void Window::swapBuffers()
{
    glfwSwapBuffers(m_window);
}

void Window::pollEvents()
{
    glfwPollEvents();
}