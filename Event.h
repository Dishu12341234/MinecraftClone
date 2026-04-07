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
class Event
{
private:
    GLFWwindow &window;
    KeyInfo lastKeyPressedInfo;
    std::array<bool, GLFW_KEY_LAST + 1> keyDown{};
    int mouseButtonDown = 0; // bitfield for mouse buttons
    int mouseButtonAction = 0; // GLFW_PRESS, GLFW_RELEASE, or GLFW_REPEAT

    unsigned long long timeElapsed_ms = 0;//elapsed Time in millseconds
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

    const unsigned long long getTimeElapsed_ms();
    ~Event();
};



class KeyTracker {
    std::unordered_map<int, bool> prevState;
public:
    // Call this once per frame AFTER all key checks
    void update(Event* event, std::initializer_list<int> keys) {
        for (int key : keys)
            prevState[key] = event->getKeyPressed(key);
    }

    bool justPressed(Event* event, int key) {
        return event->getKeyPressed(key) && !prevState[key];
    }

    bool justReleased(Event* event, int key) {
        return !event->getKeyPressed(key) && prevState[key];
    }
};


#endif
