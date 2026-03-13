#ifndef EVENT_H
#define EVENT_H
#include "GLFW/glfw3.h"
#include <array>

struct KeyInfo
{
    int key;
    int action;
};

class Event
{
private:
    GLFWwindow &window;
    KeyInfo lastKeyPressedInfo;
    std::array<bool, GLFW_KEY_LAST + 1> keyDown{};

public:

    double dt;

    double mouseX, mouseY;
    Event(GLFWwindow &window);
    static void keyPressedCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    void eventLoop();
    bool getKeyPressed(int key);
    ~Event();
};

#endif