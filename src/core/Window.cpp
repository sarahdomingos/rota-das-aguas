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

    // OpenGL 2.0: primeira versao com shaders GLSL (1.10)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_SAMPLES, 4); // Antisserrilhamento, se o driver suportar

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!m_window)
    {
        std::cerr << "Falha ao criar a janela GLFW!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // Ativa V-Sync

    // Mantem o tamanho atualizado quando a janela for redimensionada
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, onFramebufferResize);

    glfwGetFramebufferSize(m_window, &m_width, &m_height);
    glViewport(0, 0, m_width, m_height);

    return true;
}

void Window::onFramebufferResize(GLFWwindow *window, int width, int height)
{
    Window *self = static_cast<Window *>(glfwGetWindowUserPointer(window));

    if (self)
    {
        self->m_width = width;
        self->m_height = height;
    }

    glViewport(0, 0, width, height);
}

float Window::getAspect() const
{
    if (m_height <= 0)
    {
        return 1.0f;
    }

    return static_cast<float>(m_width) / static_cast<float>(m_height);
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(m_window);
}

void Window::close()
{
    glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

void Window::swapBuffers()
{
    glfwSwapBuffers(m_window);
}

void Window::pollEvents()
{
    glfwPollEvents();
}
