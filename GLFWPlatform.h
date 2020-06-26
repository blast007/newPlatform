#pragma once

#include "BzfPlatform.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

struct GLFWMonitor : public BzfMonitor
{
    GLFWmonitor* monitor;
};

class GLFWWindow;
class GLFWJoystick;

class GLFWPlatform : public BzfPlatform
{
public:
    GLFWPlatform();
    ~GLFWPlatform();

    BzfWindow* createWindow(int width, int height, BzfMonitor *monitor = nullptr, int positionX = -1, int positionY = -1);
    BzfWindow* createWindow(BzfResolution resolution, BzfMonitor *monitor = nullptr);

    // Audio
    BzfAudio *getAudio();

    // Joysticks / Game Controllers
    BzfJoystick *getJoystick();

    bool isGameRunning() const;

    // Timers
    double getGameTime() const;

    // Monitors
    BzfMonitor* getPrimaryMonitor() const;
    std::vector<BzfMonitor*> getMonitors() const;
    BzfResolution getCurrentResolution(BzfMonitor* monitor = nullptr) const;
    std::vector<BzfResolution> getResolutions(BzfMonitor* monitor = nullptr) const;

    // OpenGL Attributes
    // Set requested OpenGL profile and version
    void GLSetVersion(BzfGLProfile profile, unsigned short majorVersion, unsigned short minorVersion) const;
    // Set minumum color depth
    void GLSetRGBA(unsigned short red, unsigned short green, unsigned short blue, unsigned short alpha) const;

    // Events
    // This will poll for events and call any set callbacks
    void pollEvents();

    // Callback triggers
    static void callResizeCallback(GLFWwindow* window, int width, int height);
    static void callMoveCallback(GLFWwindow* window, int xpos, int ypos);
    static void callKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void callTextCallback(GLFWwindow* window, unsigned int codepoint);
    static void callCursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void callMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void callScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    void startTextInput();
    void stopTextInput();
    bool isTextInput();

private:
    static GLFWPlatform *platform;
    std::vector<GLFWWindow*> windows;
    GLFWJoystick *joystick;
    double startTime;
    bool inTextInputMode;
    bool joystickButtonPressed[BZF_JOY_LAST_BUTTON];
#if (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 3) || GLFW_VERSION_MAJOR > 3
    BzfJoyHatDirection joystickHatDirection[BZF_JOY_LAST_HAT];
#endif

    static BzfKey keyFromGLFW(int key);
    static int modsFromGLFW(int glfwMods);

    static void error_callback(int error, const char* description);
};

class GLFWWindow : public BzfWindow
{
public:
    GLFWWindow(GLFWPlatform *platform, int width, int height, GLFWMonitor *monitor = nullptr, int positionX = -1,
               int positionY = -1);
    GLFWWindow(GLFWPlatform *platform, BzfResolution resolution, GLFWMonitor *monitor = nullptr);
    ~GLFWWindow();

    // Fullscreen/Windowed
    bool isFullscreen() const;
    bool setVerticalSync(bool sync) const;
    bool getWindowSize(int &width, int &height) const;
    bool setWindowed(int width, int height, BzfMonitor* monitor = nullptr, int x = -1, int y = -1);
    bool setFullscreen(BzfResolution resolution, BzfMonitor* monitor = nullptr);
    void iconify() const;
    void setMinSize(int width, int height);
    void setTitle(const char *title);
    void setIcon(BzfIcon *icon);

    // Mouse
    void setMouseRelative(bool relative);
    void setMousePosition(double x, double y);
    bool supportsMouseConfinement();
    bool setConfineMouse(BzfMouseConfinement mode, double x1 = 0, double y1 = 0, double x2 = 0, double y2 = 0);
    BzfMouseConfinement getConfineMouse();

    // Drawing/context
    void makeContextCurrent() const;
    void swapBuffers() const;

    // Gamma control
    void setGamma(float gamma);
    float getGamma();
    bool hasGammaControl() const;

    // GLFW window specific methods
    bool shouldClose() const;
    GLFWwindow *getWindow() const;
    GLFWPlatform *getPlatform() const;

private:
    GLFWPlatform *platform;
    GLFWwindow *window;
    bool fullscreen;
    float gamma;

    void assignCallbacks();
};

class GLFWJoystick : public BzfJoystick
{
public:
    GLFWJoystick();
    ~GLFWJoystick();

    std::vector<BzfJoystickInfo*> getJoysticks();

    bool openDevice(int id);
    void closeDevice();

    // Joystick information
    const char *getName();
    int getNumAxes();
    int getNumHats();
    int getNumButtons();
    bool isRumbleSupported();

    // Rumble feedback
    void rumble(float strength, unsigned int duration);

    float getAxis(int axis);

    // GLFWJoystick specific methods
    int getJoystickID() const;
private:
    int joystickID;
    //SDL_Joystick *device;
    //SDL_Haptic *haptic;
    //bool rumbleSupported;
};





