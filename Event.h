#ifndef EVENT_H
#define EVENT_H
#include "GLFW/glfw3.h"
#include <array>
#include <unordered_map>

struct KeyInfo
{
    int key;
    int action;
};

class KeyTracker {
    std::unordered_map<int, bool> prevState;
public:
    // Call this once per frame AFTER all key checks
    void update(auto* event, std::initializer_list<int> keys) {
        for (int key : keys)
            prevState[key] = event->getKeyPressed(key);
    }

    bool justPressed(auto* event, int key) {
        return event->getKeyPressed(key) && !prevState[key];
    }

    bool justReleased(auto* event, int key) {
        return !event->getKeyPressed(key) && prevState[key];
    }
};

class Event
{
private:
    GLFWwindow &window;
    KeyInfo lastKeyPressedInfo;
    std::array<bool, GLFW_KEY_LAST + 1> keyDown{};
    int mouseButtonDown = 0; // bitfield for mouse buttons
    int mouseButtonAction = 0; // GLFW_PRESS, GLFW_RELEASE, or GLFW_REPEAT

public:

    double dt;

    double mouseX, mouseY;
    Event(GLFWwindow &window);
    static void keyPressedCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    void eventLoop();
    bool getKeyPressed(int key);
    bool getMouseButtonPressed(int button);
    ~Event();
};

#endif