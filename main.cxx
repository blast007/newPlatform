#include <iostream>
#include <fstream>
#include <string>
#include <string.h>

#include <GL/glew.h>

#include "PlatformFactory.h"
#include "GLHelloWorld.h"
#include "bzicon.h"

#define MESSAGE_LEN 1024

class MyCallbacks
{
public:
    void key(BzfPlatform* platform, BzfWindow* window, BzfKey key, BzfKeyAction action, int mods)
    {
        printf("Key event %d (%s), action %d and modifiers %d\n", key, getKeyName(key), action, mods);

        if (action == BZF_KEY_PRESSED && key == BZF_KEY_F12)
            exit(EXIT_SUCCESS);
        else if (action == BZF_KEY_RELEASED)
        {
            if (platform->isTextInput())
            {
                if (key == BZF_KEY_ENTER)
                {
                    printf("Finishing text input: %s\n", message);
                    platform->stopTextInput();
                    memset(message, 0, MESSAGE_LEN);
                }
                if (key == BZF_KEY_ESCAPE)
                {
                    printf("Canceling text input\n");
                    platform->stopTextInput();
                    memset(message, 0, MESSAGE_LEN);
                }
            }
            else
            {
                if (key == BZF_KEY_N)
                {
                    printf("Starting text input\n");
                    platform->startTextInput();
                }
                else if (key == BZF_KEY_X)
                {
                    int width, height;
                    window->getWindowSize(width, height);
                    window->setMousePosition(width / 2, height / 2);
                }
                else if (key == BZF_KEY_C)
                {
                    BzfMouseConfinement currentMode = window->getConfineMouse();
                    if (currentMode == BZF_MOUSE_CONFINED_NONE)
                        window->setConfineMouse(BZF_MOUSE_CONFINED_WINDOW);
                    else if (currentMode == BZF_MOUSE_CONFINED_WINDOW)
                    {
                        int width, height;
                        window->getWindowSize(width, height);
                        double centerX = width / 2;
                        double centerY = height / 2;
                        window->setConfineMouse(BZF_MOUSE_CONFINED_BOX, centerX - 20, centerY - 20, centerX + 20, centerY + 20);
                    }
                    else
                        window->setConfineMouse(BZF_MOUSE_CONFINED_NONE);
                }
                else if (key == BZF_KEY_M)
                {
                    useMouse = !useMouse;
                    printf("Input set to %s\n", useMouse?"mouse":"joystick");
                }
                else if (key == BZF_KEY_T)
                    printf("Current game time (in seconds): %f\n", platform->getGameTime());
                else if (key == BZF_KEY_F4)
                    window->iconify();
                else if (key == BZF_KEY_G)
                {
                    if (window->getGamma() < 1.0f)
                        window->setGamma(1.0f);
                    else
                        window->setGamma(0.5f);
                    printf("Gamma set for %p: %f\n", static_cast<void*>(window), window->getGamma());
                }
            }
        }
    }

    void text(BzfPlatform* /*platform*/, BzfWindow* /*window*/, char text[32])
    {
        if (strlen(message) + strlen(text) < MESSAGE_LEN)
        {
            strcat(message, text);
            printf("Text input (%lu now, added %lu): %s\n", strlen(message), strlen(text), text);
        }
        else
            printf("Text input (%lu now, ignored %lu because buffer is full): %s\n", strlen(message), strlen(text), text);
    }

    bool usingMouse()
    {
        return useMouse;
    }

    void cursorPos(BzfPlatform* /*platform*/, BzfWindow* window, double x, double y)
    {
        mouseX = x;
        mouseY = y;

        if (leftMouseButtonDown && useMouse)
        {
            int width, height;
            window->getWindowSize(width, height);
            window->makeContextCurrent();
            static_cast<GLHelloWorld*>(window->getUserPointer())->setPosition(mouseX, height-mouseY, mouseClickX,
                    height-mouseClickY);
        }
    }

    void mouseButton(BzfPlatform* /*platform*/, BzfWindow* window, BzfMouseButton button, BzfButtonAction action, int mods)
    {
        printf("Mouse %s action %d mods %d\n", getMouseButtonName(button), action, mods);

        if (button == 1 && useMouse)
        {
            leftMouseButtonDown = (action == 1);
            if (leftMouseButtonDown)
            {
                mouseClickX = mouseX;
                mouseClickY = mouseY;
            }
            else
            {
                int width, height;
                window->getWindowSize(width, height);
                double centerX = width / 2;
                double centerY = height / 2;
                window->makeContextCurrent();
                static_cast<GLHelloWorld*>(window->getUserPointer())->setPosition(centerX, centerY, centerX, centerY);
            }
        }
    }

    static void joystickButton(BzfPlatform* platform, BzfWindow* /*window*/, BzfJoyButton button, BzfButtonAction action)
    {
        printf("Joystick %s %s\n", getJoystickButtonName(button), (action == BZF_BUTTON_PRESSED)?"pressed":"released");
        if (action == BZF_BUTTON_PRESSED)
        {
            auto joystick = platform->getJoystick();
            if (button == 0)
                joystick->rumble(0.1f, 500);
            else if (button == 1)
                joystick->rumble(0.2f, 500);
            else if (button == 2)
                joystick->rumble(0.3f, 500);
            else if (button == 3)
                joystick->rumble(0.4f, 500);
            else if (button == 4)
                joystick->rumble(0.5f, 500);
            else if (button == 5)
                joystick->rumble(0.1f, 200);
            else if (button == 6)
                joystick->rumble(0.2f, 200);
            else if (button == 7)
                joystick->rumble(0.3f, 200);
            else if (button == 8)
                joystick->rumble(0.4f, 200);
            else if (button == 9)
                joystick->rumble(0.5f, 200);
        }
    }

    static void joystickHat(BzfPlatform* /*platform*/, BzfWindow* /*window*/, BzfJoyHat hat, BzfJoyHatDirection direction)
    {
        printf("Joystick %s %s\n", getJoystickHatName(hat), getJoystickHatDirectionName(direction));
    }
private:
    char message[MESSAGE_LEN] = {0};
    double mouseX = 0, mouseY = 0;
    double mouseClickX = 0, mouseClickY = 0;
    bool useMouse = true;
    bool leftMouseButtonDown = false;
};

void scroll_callback(BzfPlatform* /*platform*/, BzfWindow* /*window*/, double x, double y)
{
    printf("Scroll position: %.1lf %.1lf\n", x, y);
}

void resize_callback(BzfPlatform* /*platform*/, BzfWindow* window, int width, int height)
{
    window->makeContextCurrent();
    static_cast<GLHelloWorld*>(window->getUserPointer())->resize(width, height);
}

int main()
{
    // Get the platform factory
    BzfPlatform* platform = PlatformFactory::get();

    BzfAudio* audio = platform->getAudio();
    if (audio != nullptr)
    {
        auto audioDevices = audio->getAudioDevices();
        printf("Audio devices:\n");
        for (auto &audioDevice : audioDevices)
            printf(" * %s\n", audioDevice);
    }

    BzfJoystick* joystick = platform->getJoystick();
    if (joystick != nullptr)
    {
        auto joysticks = joystick->getJoysticks();
        printf("\nJoystick devices:\n");
        for (auto &joystick : joysticks)
            printf(" * %s - %s (%d axes, %d hats, %d buttons, Rumble %s Supported, %s)\n", joystick->guid, joystick->name,
                   joystick->axes, joystick->hats, joystick->buttons, joystick->isRumbleSupported?"Is":"Is Not",
                   joystick->isGameController?"Game Controller Interface":"Joystick Interface");
        printf("\n");
        if (joysticks.size() > 0)
            if (!joystick->openDevice(0))
                printf("Failed to open joystick\n");
    }

    // Set OpenGL attributes
#ifdef USE_GLES2
    platform->GLSetVersion(BzfGLES, 2, 0);
#else
    platform->GLSetVersion(BzfGLCore, 3, 2);
#endif
    // Request 8 bits per channel
    platform->GLSetRGBA(8, 8, 8, 8);

    // Get all monitors
    auto monitors = platform->getMonitors();

    printf("\nNumber of monitors: %ld\n\n", monitors.size());
    // Dump the supported modes for each monitor
    for (auto &monitor : monitors)
    {
        auto resolutions = platform->getResolutions(monitor);
        printf("Resolutions for monitor %s (%ld):\n", monitor->name.c_str(), resolutions.size());
        for (auto &resolution : resolutions)
            printf(" * %dx%d@%dHz\n", resolution.width, resolution.height, resolution.refreshRate);
        printf("\n");
    }

    BzfResolution resolution;

    // Windowed mode
    bool windowed = true;

    // Create a window on each monitor at the current desktop resolution
    std::vector<BzfWindow*> windows;

    if (monitors.size() >= 1 || windowed)
    {
        BzfWindow* window;
        if (windowed)
            window = platform->createWindow(800, 600, nullptr, 40, 40);
        else
        {
            resolution = platform->getCurrentResolution(monitors.at(0));
            window = platform->createWindow(resolution, monitors.at(0));
        }
        window->setMinSize(512, 384);
        window->setTitle("Mss3WN");
        window->setIcon(&icon);

        // From: https://www.shadertoy.com/view/Mss3WN
        window->makeContextCurrent();
        int width, height;
        window->getWindowSize(width, height);
        window->setUserPointer(new GLHelloWorld("Mss3WN.frag", width, height));

        windows.push_back(window);
    }

    if (monitors.size() >= 2 || windowed)
    {
        BzfWindow* window;
        if (windowed)
            window = platform->createWindow(800, 600, nullptr, 860, 40);
        else
        {
            resolution = platform->getCurrentResolution(monitors.at(1));
            window = platform->createWindow(resolution, monitors.at(1));
        }
        window->setMinSize(640, 480);
        window->setTitle("ldfGWn");

        // From: https://www.shadertoy.com/view/ldfGWn
        window->makeContextCurrent();
        int width, height;
        window->getWindowSize(width, height);
        window->setUserPointer(new GLHelloWorld("ldfGWn.frag", width, height));

        windows.push_back(window);
    }

    // Callbacks
    using namespace std::placeholders;
    MyCallbacks *callbacks = new MyCallbacks;
    // Two examples of non-static methods
    platform->setKeyCallback(std::bind(&MyCallbacks::key, callbacks, _1, _2, _3, _4, _5));
    platform->setTextCallback(std::bind(&MyCallbacks::text, callbacks, _1, _2, _3));
    platform->setCursorPosCallback(std::bind(&MyCallbacks::cursorPos, callbacks, _1, _2, _3, _4));
    platform->setMouseButtonCallback(std::bind(&MyCallbacks::mouseButton, callbacks, _1, _2, _3, _4, _5));
    // Example of a static method
    platform->setJoystickButtonCallback(&MyCallbacks::joystickButton);
    platform->setJoystickHatCallback(&MyCallbacks::joystickHat);
    // Function callbacks
    platform->setScrollCallback(scroll_callback);
    platform->addResizeCallback(resize_callback);

    while (platform->isGameRunning())
    {
        platform->pollEvents();

        for (auto &window : windows)
        {
            window->makeContextCurrent();
            auto hw = static_cast<GLHelloWorld*>(window->getUserPointer());
            if (!callbacks->usingMouse())
            {
                int width, height;
                window->getWindowSize(width, height);
                double centerX = width / 2;
                double centerY = height / 2;
                double scaleX = 1 / centerX;
                double scaleY = 1 / centerY;

                hw->setPosition(centerX + (joystick->getAxis(0) / scaleX), centerY - (joystick->getAxis(1) / scaleY), centerX, centerY);
            }
            hw->drawFrame(platform->getGameTime());
            window->swapBuffers();
        }
    }

    // Delete test programs
    for (auto &window : windows)
    {
        GLHelloWorld* hw = static_cast<GLHelloWorld*>(window->getUserPointer());
        delete hw;
        window->setUserPointer(nullptr);
    }

    // Delete platform factory
    delete platform;
    platform = nullptr;

    // We out!
    return 0;
}
