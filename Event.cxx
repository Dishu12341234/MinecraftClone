#include "Event.h"
#include <iostream>
#include <chrono>

Event::Event(GLFWwindow &window) : window{window}
{
    glfwSetKeyCallback(&window, Event::keyPressedCallback);
    glfwSetCursorPosCallback(&window, Event::cursorPositionCallback);
    glfwSetMouseButtonCallback(&window, Event::mouseButtonCallback);

    if (glfwRawMouseMotionSupported())
    {
        // glfwSetInputMode(&window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        // std::cout << "RawInput" << std::endl;
    }
}

void Event::keyPressedCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{

    if (action == GLFW_REPEAT)
        return;
    Event *event = static_cast<Event *>(glfwGetWindowUserPointer(window));
    if (!event)
        return;

    event->lastKeyPressedInfo.key = key;
    event->lastKeyPressedInfo.action = action;

    event->keyDown[key] = (action == GLFW_PRESS);
}
void Event::cursorPositionCallback(GLFWwindow *window, double xpos, double ypos)
{
    Event *event = static_cast<Event *>(glfwGetWindowUserPointer(window));
    if (!event)
        return;

    // std::cout << "MouseCB" << time(0) << std::endl;

    event->mouseX = xpos;
    event->mouseY = ypos;

    std::cout << "--------" << std::endl;
    std::cout << xpos << std::endl;
    std::cout << ypos << std::endl;
    std::cout << "--------" << std::endl;
}
// TODO cursor callback this wont work
void Event::eventLoop()
{
    static auto startingTime = std::chrono::high_resolution_clock::now();
    static auto lastTime = startingTime;

    
    auto currentTime = std::chrono::high_resolution_clock::now();
    timeElapsed_ms = std::chrono::duration<double, std::milli>(currentTime - startingTime).count();

    auto delta = currentTime - lastTime;
    lastTime = currentTime;

    dt = std::chrono::duration<double, std::milli>(delta).count();
    // std::cout << "Δt = " << dt << std::endl;
    std::cout << "FPS: " << 1000.f / dt << std::endl;
    glfwPollEvents();
}

bool Event::getKeyPressed(int key)
{
    return keyDown[key];
}

const unsigned long long Event::getTimeElapsed_ms()
{
    return timeElapsed_ms;
}

Event::~Event()
{
}

void Event::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    Event *event = static_cast<Event *>(glfwGetWindowUserPointer(window));
    if (!event)
        return;

    event->mouseButtonDown = button;
    event->mouseButtonAction = action;
}

bool Event::getMouseButtonPressed(int button)
{
    if(button == mouseButtonDown)
        return (mouseButtonAction == GLFW_PRESS);

    return false;
}
