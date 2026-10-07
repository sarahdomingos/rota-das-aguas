#ifndef WINDOW_H
#define WINDOW_H

#include <GLFW/glfw3.h>
#include <string>

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    bool init();
    bool shouldClose() const;
    void close();
    void swapBuffers();
    void pollEvents();

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    float getAspect() const;
    GLFWwindow* getNativeWindow() const { return m_window; }

private:
    int m_width;
    int m_height;
    std::string m_title;
    GLFWwindow* m_window;

    static void onFramebufferResize(GLFWwindow* window, int width, int height);
};

#endif
