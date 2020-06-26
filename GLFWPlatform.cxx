#include "GLFWPlatform.h"
#include <stdio.h>
#include <iostream>
#include <string.h>

///////////////////////////////////////////////////////////
// Platform
///////////////////////////////////////////////////////////

GLFWPlatform *GLFWPlatform::platform = nullptr;

GLFWPlatform::GLFWPlatform() : joystick(nullptr), inTextInputMode(false)
{
#if (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 3) || GLFW_VERSION_MAJOR > 3
    // Do not include the joystick hats as buttons
    glfwInitHint(GLFW_JOYSTICK_HAT_BUTTONS, GLFW_FALSE);
#endif

    if (!glfwInit())
    {
        std::cerr << "Error: There was an error initializing GLFW." << std::endl;
        exit(-1);
    }

    int i;
    for (i = 0; i < BZF_JOY_LAST_BUTTON; ++i)
        joystickButtonPressed[i] = false;
#if (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 3) || GLFW_VERSION_MAJOR > 3
    for (i = 0; i < BZF_JOY_LAST_HAT; ++i)
        joystickHatDirection[i] = BZF_JOY_HAT_CENTERED;
#endif

    // Store a static pointer to ourself, for use in callbacks
    GLFWPlatform::platform = this;
#if (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 3) || GLFW_VERSION_MAJOR > 3
    // Use full resolution framebuffers on Retina displays
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
    // Use the discrete graphics card on systems with hybrid graphics
    glfwWindowHint(GLFW_COCOA_GRAPHICS_SWITCHING, GLFW_FALSE);
#endif

    // Error callback
    glfwSetErrorCallback(GLFWPlatform::error_callback);

#ifdef _DEBUG
	// For debugging, set the start time of the program to be 184 days in the past to catch issues that may occur with long running programs
	glfwSetTime(60.0 * 60.0 * 24.0 * 184.0);
#endif
}

GLFWPlatform::~GLFWPlatform()
{
    // Delete all the windows (Is this necessary?)
    for (auto window : windows)
        delete window;

    // Shut down GLFW
    glfwTerminate();
}

BzfWindow* GLFWPlatform::createWindow(int width, int height, BzfMonitor* monitor, int positionX, int positionY)
{
    GLFWWindow* window = new GLFWWindow(this, width, height, static_cast<GLFWMonitor*>(monitor), positionX, positionY);
    windows.push_back(window);
    return window;
}

BzfWindow* GLFWPlatform::createWindow(BzfResolution resolution, BzfMonitor *monitor)
{
    GLFWWindow* window = new GLFWWindow(this, resolution, static_cast<GLFWMonitor*>(monitor));
    windows.push_back(window);
    return window;
}

BzfAudio *GLFWPlatform::getAudio()
{
    return nullptr;
}

BzfJoystick *GLFWPlatform::getJoystick()
{
    if (joystick == nullptr)
        joystick = new GLFWJoystick();
    return joystick;
}

bool GLFWPlatform::isGameRunning() const
{
    glfwPollEvents();

    for (auto window : windows)
    {
        if (window->shouldClose())
            return false;
    }

    return true;
}

double GLFWPlatform::getGameTime() const
{
    return glfwGetTime();
}

BzfMonitor* GLFWPlatform::getPrimaryMonitor() const
{
    GLFWMonitor* monitor = new GLFWMonitor;
    monitor->monitor = glfwGetPrimaryMonitor();
    monitor->name = glfwGetMonitorName(monitor->monitor);
    return monitor;
}

std::vector<BzfMonitor*> GLFWPlatform::getMonitors() const
{
    std::vector<BzfMonitor*> monitors;
    int count;
    GLFWmonitor** glfwMonitors = glfwGetMonitors(&count);
    for (int i = 0; i < count; i++)
    {
        GLFWMonitor* monitor = new GLFWMonitor;
        monitor->name = glfwGetMonitorName(glfwMonitors[i]);
        monitor->monitor = glfwMonitors[i];
        monitors.push_back(monitor);
    }
    return monitors;
}

BzfResolution GLFWPlatform::getCurrentResolution(BzfMonitor* monitor) const
{
    BzfResolution resolution;
    GLFWmonitor* mon;
    if (monitor != nullptr && static_cast<GLFWMonitor*>(monitor)->monitor != nullptr)
        mon = static_cast<GLFWMonitor*>(monitor)->monitor;
    else
        mon = glfwGetPrimaryMonitor();
    const GLFWvidmode* currentMode = glfwGetVideoMode(mon);
    resolution.width = currentMode->width;
    resolution.height = currentMode->height;
    resolution.refreshRate = currentMode->refreshRate;
    return resolution;
}

std::vector<BzfResolution> GLFWPlatform::getResolutions(BzfMonitor* monitor) const
{
    std::vector<BzfResolution> resolutions;
    int count = 0;
    GLFWmonitor* mon;
    if (monitor != nullptr && static_cast<GLFWMonitor*>(monitor)->monitor != nullptr)
        mon = static_cast<GLFWMonitor*>(monitor)->monitor;
    else
        mon = glfwGetPrimaryMonitor();
    const GLFWvidmode* modes = glfwGetVideoModes(mon, &count);
    for (int i = 0; i < count; i++)
    {
        BzfResolution resolution;
        resolution.width = modes[i].width;
        resolution.height = modes[i].height;
        resolution.refreshRate = modes[i].refreshRate;
        resolutions.push_back(resolution);
    }

    return resolutions;
}

void GLFWPlatform::GLSetVersion(BzfGLProfile profile, unsigned short majorVersion, unsigned short minorVersion) const
{
    if (profile == BzfGLCore)
    {
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    }
    else if (profile == BzfGLES)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    }
    else
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, majorVersion);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minorVersion);
}

void GLFWPlatform::GLSetRGBA(unsigned short red, unsigned short green, unsigned short blue,
                             unsigned short /*alpha*/) const
{
    glfwWindowHint(GLFW_RED_BITS, red);
    glfwWindowHint(GLFW_GREEN_BITS, green);
    glfwWindowHint(GLFW_BLUE_BITS, blue);
}

void GLFWPlatform::pollEvents()
{
    // GLFW does not currently have an event system for joysticks, so you have to poll for the button state. This will
    // likely miss events, especially if this function is not called frequenly, such as if the main thread also
    // handles graphics.

    // GLFW 3.2 does not have a separate handling of hats and they are instead treated like buttons. By default, 3.3
    // also behaves this way for compatability, but we set a hint to change this behavior.

    if (joystick != nullptr && windows.size() > 0)
    {
        int joystickID = joystick->getJoystickID();

        if (glfwJoystickPresent(joystickID))
        {
            int count, i;
            if (joystickButtonCallback != nullptr)
            {
                // Get the button state
                const unsigned char* buttons = glfwGetJoystickButtons(joystickID, &count);
                BzfJoyButton button;

                for (i = 0; i < count && i < BZF_JOY_LAST_BUTTON; ++i)
                {
                    switch(i)
                    {
                    // *INDENT-OFF*
                    case 0: button = BZF_JOY_BUTTON_1; break;
                    case 1: button = BZF_JOY_BUTTON_2; break;
                    case 2: button = BZF_JOY_BUTTON_3; break;
                    case 3: button = BZF_JOY_BUTTON_4; break;
                    case 4: button = BZF_JOY_BUTTON_5; break;
                    case 5: button = BZF_JOY_BUTTON_6; break;
                    case 6: button = BZF_JOY_BUTTON_7; break;
                    case 7: button = BZF_JOY_BUTTON_8; break;
                    case 8: button = BZF_JOY_BUTTON_9; break;
                    case 9: button = BZF_JOY_BUTTON_10; break;
                    case 10: button = BZF_JOY_BUTTON_11; break;
                    case 11: button = BZF_JOY_BUTTON_12; break;
                    case 12: button = BZF_JOY_BUTTON_13; break;
                    case 13: button = BZF_JOY_BUTTON_14; break;
                    case 14: button = BZF_JOY_BUTTON_15; break;
                    case 15: button = BZF_JOY_BUTTON_16; break;
                    case 16: button = BZF_JOY_BUTTON_17; break;
                    case 17: button = BZF_JOY_BUTTON_18; break;
                    case 18: button = BZF_JOY_BUTTON_19; break;
                    case 19: button = BZF_JOY_BUTTON_20; break;
                    case 20: button = BZF_JOY_BUTTON_21; break;
                    case 21: button = BZF_JOY_BUTTON_22; break;
                    case 22: button = BZF_JOY_BUTTON_23; break;
                    case 23: button = BZF_JOY_BUTTON_24; break;
                    case 24: button = BZF_JOY_BUTTON_25; break;
                    case 25: button = BZF_JOY_BUTTON_26; break;
                    case 26: button = BZF_JOY_BUTTON_27; break;
                    case 27: button = BZF_JOY_BUTTON_28; break;
                    case 28: button = BZF_JOY_BUTTON_29; break;
                    case 29: button = BZF_JOY_BUTTON_30; break;
                    case 30: button = BZF_JOY_BUTTON_31; break;
                    case 31: button = BZF_JOY_BUTTON_32; break;
                    default: button = BZF_JOY_BUTTON_UNKNOWN; break;
                    // *INDENT-ON*
                    }
                    if (button == BZF_JOY_BUTTON_UNKNOWN)
                        continue;

                    if (!joystickButtonPressed[i] && buttons[i] == GLFW_PRESS)
                        joystickButtonCallback(this, windows.at(0), button, BZF_BUTTON_PRESSED);
                    else if (joystickButtonPressed[i] && buttons[i] != GLFW_PRESS)
                        joystickButtonCallback(this, windows.at(0), button, BZF_BUTTON_RELEASED);

                    joystickButtonPressed[i] = (buttons[i] == GLFW_PRESS);
                }
            }


#if (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 3) || GLFW_VERSION_MAJOR > 3
            if (joystickHatCallback != nullptr)
            {
                const unsigned char* hats = glfwGetJoystickHats(joystickID, &count);
                BzfJoyHat hat;
                BzfJoyHatDirection direction;
                for (i = 0; i < count&& i < BZF_JOY_LAST_HAT; ++i)
                {

                    switch(i)
                    {
                    // *INDENT-OFF*
                    case 0: hat = BZF_JOY_HAT_1; break;
                    case 1: hat = BZF_JOY_HAT_2; break;
                    case 2: hat = BZF_JOY_HAT_3; break;
                    case 3: hat = BZF_JOY_HAT_4; break;
                    case 4: hat = BZF_JOY_HAT_5; break;
                    case 5: hat = BZF_JOY_HAT_6; break;
                    case 6: hat = BZF_JOY_HAT_7; break;
                    case 7: hat = BZF_JOY_HAT_8; break;
                    default: hat = BZF_JOY_HAT_UNKNOWN; break;
                    // *INDENT-ON*
                    }
                    if (hat == BZF_JOY_HAT_UNKNOWN)
                        continue;

                    switch(hats[i])
                    {
                    // *INDENT-OFF*
                    case GLFW_HAT_LEFT_UP: direction = BZF_JOY_HAT_LEFTUP; break;
                    case GLFW_HAT_UP: direction = BZF_JOY_HAT_UP; break;
                    case GLFW_HAT_RIGHT_UP: direction = BZF_JOY_HAT_RIGHTUP; break;
                    case GLFW_HAT_LEFT: direction = BZF_JOY_HAT_LEFT; break;
                    case GLFW_HAT_RIGHT: direction = BZF_JOY_HAT_RIGHT; break;
                    case GLFW_HAT_LEFT_DOWN: direction = BZF_JOY_HAT_LEFTDOWN; break;
                    case GLFW_HAT_DOWN: direction = BZF_JOY_HAT_DOWN; break;
                    case GLFW_HAT_RIGHT_DOWN: direction = BZF_JOY_HAT_RIGHTDOWN;break;
                    default: direction = BZF_JOY_HAT_CENTERED; break;
                    // *INDENT-ON*
                    }

                    // Trigger the callback if
                    if (joystickHatDirection[i] != direction)
                        joystickHatCallback(this, windows.at(0), hat, direction);

                    joystickHatDirection[i] = direction;
                }
            }
#endif
        }
    }

    glfwPollEvents();
}

void GLFWPlatform::callResizeCallback(GLFWwindow* window, int width, int height)
{
    for (auto callback : platform->resizeCallbacks)
        callback(platform, static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window)), width, height);
}

void GLFWPlatform::callMoveCallback(GLFWwindow* window, int xpos, int ypos)
{
    for (auto callback : platform->moveCallbacks)
        callback(platform, static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window)), xpos, ypos);
}

void GLFWPlatform::callKeyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int mods)
{
    if (platform->keyCallback != nullptr)
    {
        BzfKeyAction kaction = BZF_KEY_RELEASED;
        if (action == GLFW_PRESS)
            kaction = BZF_KEY_PRESSED;
        else if (action == GLFW_REPEAT)
            kaction = BZF_KEY_REPEATED;

        platform->keyCallback(platform, static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window)), keyFromGLFW(key), kaction,
                              modsFromGLFW(mods));
    }
}

// Start of code ripped from Nuklear

/*
Copyright (c) 2017 Micha Mettke
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <assert.h>
#define NK_ASSERT(expr) assert(expr)

#define NK_BETWEEN(x, a, b) ((a) <= (x) && (x) < (b))
#define NK_LEN(a) (sizeof(a)/sizeof(a)[0])

#include <stdint.h>
#define NK_INT8 int8_t
#define NK_UINT8 uint8_t
#define NK_INT16 int16_t
#define NK_UINT16 uint16_t
#define NK_INT32 int32_t
#define NK_UINT32 uint32_t
#define NK_SIZE_TYPE uintptr_t
#define NK_POINTER_TYPE uintptr_t

typedef NK_INT8 nk_char;
typedef NK_UINT8 nk_uchar;
typedef NK_UINT8 nk_byte;
typedef NK_INT16 nk_short;
typedef NK_UINT16 nk_ushort;
typedef NK_INT32 nk_int;
typedef NK_UINT32 nk_uint;
typedef NK_SIZE_TYPE nk_size;
typedef NK_POINTER_TYPE nk_ptr;

#define NK_UTF_INVALID 0xFFFD /* internal invalid utf8 rune */
#define NK_UTF_SIZE 4

typedef nk_uint nk_rune;
typedef char nk_glyph[NK_UTF_SIZE];

const nk_byte nk_utfbyte[NK_UTF_SIZE+1] = {0x80, 0, 0xC0, 0xE0, 0xF0};
const nk_byte nk_utfmask[NK_UTF_SIZE+1] = {0xC0, 0x80, 0xE0, 0xF0, 0xF8};
const nk_uint nk_utfmin[NK_UTF_SIZE+1] = {0, 0, 0x80, 0x800, 0x10000};
const nk_uint nk_utfmax[NK_UTF_SIZE+1] = {0x10FFFF, 0x7F, 0x7FF, 0xFFFF, 0x10FFFF};

int
nk_utf_validate(nk_rune *u, int i)
{
    NK_ASSERT(u);
    if (!u) return 0;
    if (!NK_BETWEEN(*u, nk_utfmin[i], nk_utfmax[i]) ||
            NK_BETWEEN(*u, 0xD800, 0xDFFF))
        *u = NK_UTF_INVALID;
    for (i = 1; *u > nk_utfmax[i]; ++i);
    return i;
}
nk_rune nk_utf_decode_byte(char c, int *i)
{
    NK_ASSERT(i);
    if (!i) return 0;
    for(*i = 0; *i < (int)NK_LEN(nk_utfmask); ++(*i))
    {
        if (((nk_byte)c & nk_utfmask[*i]) == nk_utfbyte[*i])
            return (nk_byte)(c & ~nk_utfmask[*i]);
    }
    return 0;
}
int nk_utf_decode(const char *c, nk_rune *u, int clen)
{
    int i, j, len, type=0;
    nk_rune udecoded;

    NK_ASSERT(c);
    NK_ASSERT(u);

    if (!c || !u) return 0;
    if (!clen) return 0;
    *u = NK_UTF_INVALID;

    udecoded = nk_utf_decode_byte(c[0], &len);
    if (!NK_BETWEEN(len, 1, NK_UTF_SIZE))
        return 1;

    for (i = 1, j = 1; i < clen && j < len; ++i, ++j)
    {
        udecoded = (udecoded << 6) | nk_utf_decode_byte(c[i], &type);
        if (type != 0)
            return j;
    }
    if (j < len)
        return 0;
    *u = udecoded;
    nk_utf_validate(u, len);
    return len;
}
char nk_utf_encode_byte(nk_rune u, int i)
{
    return (char)((nk_utfbyte[i]) | ((nk_byte)u & ~nk_utfmask[i]));
}
int nk_utf_encode(nk_rune u, char *c, int clen)
{
    int len, i;
    len = nk_utf_validate(&u, 0);
    if (clen < len || !len || len > NK_UTF_SIZE)
        return 0;

    for (i = len - 1; i != 0; --i)
    {
        c[i] = nk_utf_encode_byte(u, 0);
        u >>= 6;
    }
    c[0] = nk_utf_encode_byte(u, len);
    return len;
}

void append_unicode(char *string, nk_rune unicode2, unsigned int maxSize)
{
    nk_glyph glyph;
    nk_utf_encode(unicode2, glyph, NK_UTF_SIZE);

    int len = 0;
    nk_rune unicode;

    len = nk_utf_decode(glyph, &unicode, NK_UTF_SIZE);
    if (len && ((strlen(string) + len) < maxSize))
    {
        nk_utf_encode(unicode, &string[strlen(string)],
                      maxSize - strlen(string));
    }
}

// End of code ripped from Nuklear

void GLFWPlatform::callTextCallback(GLFWwindow* window, unsigned int codepoint)
{
    if (platform->textCallback != nullptr)
    {
        char buffer[32] = {0};

        append_unicode(buffer, codepoint, 32);

        platform->textCallback(platform, static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window)), buffer);
    }

}

void GLFWPlatform::callCursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (platform->cursorPosCallback != nullptr)
        platform->cursorPosCallback(platform, static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window)), xpos, ypos);
}

void GLFWPlatform::callMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (platform->mouseButtonCallback != nullptr)
    {
        BzfMouseButton bzbutton;
        // GLFW starts the numbering at 0, and flips the middle and right values
        switch(button)
        {
        // *INDENT-OFF*
        case 0: bzbutton = BZF_MOUSE_LEFT; break;
        case 2: bzbutton = BZF_MOUSE_MIDDLE; break;
        case 1: bzbutton = BZF_MOUSE_RIGHT; break;
        case 3: bzbutton = BZF_MOUSE_4; break;
        case 4: bzbutton = BZF_MOUSE_5; break;
        case 5: bzbutton = BZF_MOUSE_6; break;
        case 6: bzbutton = BZF_MOUSE_7; break;
        case 7: bzbutton = BZF_MOUSE_8; break;
        default: bzbutton = BZF_MOUSE_UNKNOWN; break;
        // *INDENT-ON*
        }
        if (bzbutton == BZF_MOUSE_UNKNOWN)
            return;
        platform->mouseButtonCallback(platform, static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window)), bzbutton,
                                      (action == GLFW_PRESS)?BZF_BUTTON_PRESSED:BZF_BUTTON_RELEASED, modsFromGLFW(mods));
    }
}

void GLFWPlatform::callScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (platform->scrollCallback != nullptr)
        platform->scrollCallback(platform, static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window)), xoffset, yoffset);
}

void GLFWPlatform::startTextInput()
{
    inTextInputMode = true;

    for (auto window : windows)
        glfwSetCharCallback(window->getWindow(), GLFWPlatform::callTextCallback);
}

void GLFWPlatform::stopTextInput()
{
    for (auto window : windows)
        glfwSetCharCallback(window->getWindow(), nullptr);

    inTextInputMode = false;
}

bool GLFWPlatform::isTextInput()
{
    return inTextInputMode;
}

BzfKey GLFWPlatform::keyFromGLFW(int key)
{
    switch (key)
    {
    case GLFW_KEY_SPACE:
        return BZF_KEY_SPACE;
    case GLFW_KEY_APOSTROPHE:
        return BZF_KEY_APOSTROPHE;
    case GLFW_KEY_COMMA:
        return BZF_KEY_COMMA;
    case GLFW_KEY_MINUS:
        return BZF_KEY_MINUS;
    case GLFW_KEY_PERIOD:
        return BZF_KEY_PERIOD;
    case GLFW_KEY_SLASH:
        return BZF_KEY_SLASH;
    case GLFW_KEY_0:
        return BZF_KEY_0;
    case GLFW_KEY_1:
        return BZF_KEY_1;
    case GLFW_KEY_2:
        return BZF_KEY_2;
    case GLFW_KEY_3:
        return BZF_KEY_3;
    case GLFW_KEY_4:
        return BZF_KEY_4;
    case GLFW_KEY_5:
        return BZF_KEY_5;
    case GLFW_KEY_6:
        return BZF_KEY_6;
    case GLFW_KEY_7:
        return BZF_KEY_7;
    case GLFW_KEY_8:
        return BZF_KEY_8;
    case GLFW_KEY_9:
        return BZF_KEY_9;
    case GLFW_KEY_SEMICOLON:
        return BZF_KEY_SEMICOLON;
    case GLFW_KEY_EQUAL:
        return BZF_KEY_EQUAL;
    case GLFW_KEY_A:
        return BZF_KEY_A;
    case GLFW_KEY_B:
        return BZF_KEY_B;
    case GLFW_KEY_C:
        return BZF_KEY_C;
    case GLFW_KEY_D:
        return BZF_KEY_D;
    case GLFW_KEY_E:
        return BZF_KEY_E;
    case GLFW_KEY_F:
        return BZF_KEY_F;
    case GLFW_KEY_G:
        return BZF_KEY_G;
    case GLFW_KEY_H:
        return BZF_KEY_H;
    case GLFW_KEY_I:
        return BZF_KEY_I;
    case GLFW_KEY_J:
        return BZF_KEY_J;
    case GLFW_KEY_K:
        return BZF_KEY_K;
    case GLFW_KEY_L:
        return BZF_KEY_L;
    case GLFW_KEY_M:
        return BZF_KEY_M;
    case GLFW_KEY_N:
        return BZF_KEY_N;
    case GLFW_KEY_O:
        return BZF_KEY_O;
    case GLFW_KEY_P:
        return BZF_KEY_P;
    case GLFW_KEY_Q:
        return BZF_KEY_Q;
    case GLFW_KEY_R:
        return BZF_KEY_R;
    case GLFW_KEY_S:
        return BZF_KEY_S;
    case GLFW_KEY_T:
        return BZF_KEY_T;
    case GLFW_KEY_U:
        return BZF_KEY_U;
    case GLFW_KEY_V:
        return BZF_KEY_V;
    case GLFW_KEY_W:
        return BZF_KEY_W;
    case GLFW_KEY_X:
        return BZF_KEY_X;
    case GLFW_KEY_Y:
        return BZF_KEY_Y;
    case GLFW_KEY_Z:
        return BZF_KEY_Z;
    case GLFW_KEY_LEFT_BRACKET:
        return BZF_KEY_LEFT_BRACKET;
    case GLFW_KEY_BACKSLASH:
        return BZF_KEY_BACKSLASH;
    case GLFW_KEY_RIGHT_BRACKET:
        return BZF_KEY_RIGHT_BRACKET;
    case GLFW_KEY_GRAVE_ACCENT:
        return BZF_KEY_GRAVE_ACCENT;
    case GLFW_KEY_WORLD_1:
        return BZF_KEY_WORLD_1; // What are these?
    case GLFW_KEY_WORLD_2:
        return BZF_KEY_WORLD_2; // What are these?
    case GLFW_KEY_ESCAPE:
        return BZF_KEY_ESCAPE;
    case GLFW_KEY_ENTER:
        return BZF_KEY_ENTER;
    case GLFW_KEY_TAB:
        return BZF_KEY_TAB;
    case GLFW_KEY_BACKSPACE:
        return BZF_KEY_BACKSPACE;
    case GLFW_KEY_INSERT:
        return BZF_KEY_INSERT;
    case GLFW_KEY_DELETE:
        return BZF_KEY_DELETE;
    case GLFW_KEY_RIGHT:
        return BZF_KEY_RIGHT;
    case GLFW_KEY_LEFT:
        return BZF_KEY_LEFT;
    case GLFW_KEY_DOWN:
        return BZF_KEY_DOWN;
    case GLFW_KEY_UP:
        return BZF_KEY_UP;
    case GLFW_KEY_PAGE_UP:
        return BZF_KEY_PAGE_UP;
    case GLFW_KEY_PAGE_DOWN:
        return BZF_KEY_PAGE_DOWN;
    case GLFW_KEY_HOME:
        return BZF_KEY_HOME;
    case GLFW_KEY_END:
        return BZF_KEY_END;
    case GLFW_KEY_PAUSE:
        return BZF_KEY_PAUSE;
    case GLFW_KEY_F1:
        return BZF_KEY_F1;
    case GLFW_KEY_F2:
        return BZF_KEY_F2;
    case GLFW_KEY_F3:
        return BZF_KEY_F3;
    case GLFW_KEY_F4:
        return BZF_KEY_F4;
    case GLFW_KEY_F5:
        return BZF_KEY_F5;
    case GLFW_KEY_F6:
        return BZF_KEY_F6;
    case GLFW_KEY_F7:
        return BZF_KEY_F7;
    case GLFW_KEY_F8:
        return BZF_KEY_F8;
    case GLFW_KEY_F9:
        return BZF_KEY_F9;
    case GLFW_KEY_F10:
        return BZF_KEY_F10;
    case GLFW_KEY_F11:
        return BZF_KEY_F11;
    case GLFW_KEY_F12:
        return BZF_KEY_F12;
    case GLFW_KEY_F13:
        return BZF_KEY_F13;
    case GLFW_KEY_F14:
        return BZF_KEY_F14;
    case GLFW_KEY_F15:
        return BZF_KEY_F15;
    case GLFW_KEY_F16:
        return BZF_KEY_F16;
    case GLFW_KEY_F17:
        return BZF_KEY_F17;
    case GLFW_KEY_F18:
        return BZF_KEY_F18;
    case GLFW_KEY_F19:
        return BZF_KEY_F19;
    case GLFW_KEY_F20:
        return BZF_KEY_F20;
    case GLFW_KEY_F21:
        return BZF_KEY_F21;
    case GLFW_KEY_F22:
        return BZF_KEY_F22;
    case GLFW_KEY_F23:
        return BZF_KEY_F23;
    case GLFW_KEY_F24:
        return BZF_KEY_F24;
    case GLFW_KEY_F25:
        return BZF_KEY_F25;
    case GLFW_KEY_KP_0:
        return BZF_KEY_KP_0;
    case GLFW_KEY_KP_1:
        return BZF_KEY_KP_1;
    case GLFW_KEY_KP_2:
        return BZF_KEY_KP_2;
    case GLFW_KEY_KP_3:
        return BZF_KEY_KP_3;
    case GLFW_KEY_KP_4:
        return BZF_KEY_KP_4;
    case GLFW_KEY_KP_5:
        return BZF_KEY_KP_5;
    case GLFW_KEY_KP_6:
        return BZF_KEY_KP_6;
    case GLFW_KEY_KP_7:
        return BZF_KEY_KP_7;
    case GLFW_KEY_KP_8:
        return BZF_KEY_KP_8;
    case GLFW_KEY_KP_9:
        return BZF_KEY_KP_9;
    case GLFW_KEY_KP_DECIMAL:
        return BZF_KEY_KP_DECIMAL;
    case GLFW_KEY_KP_DIVIDE:
        return BZF_KEY_KP_DIVIDE;
    case GLFW_KEY_KP_MULTIPLY:
        return BZF_KEY_KP_MULTIPLY;
    case GLFW_KEY_KP_SUBTRACT:
        return BZF_KEY_KP_SUBTRACT;
    case GLFW_KEY_KP_ADD:
        return BZF_KEY_KP_ADD;
    case GLFW_KEY_KP_ENTER:
        return BZF_KEY_KP_ENTER;
    case GLFW_KEY_KP_EQUAL:
        return BZF_KEY_KP_EQUAL;
    case GLFW_KEY_LEFT_SHIFT:
        return BZF_KEY_LEFT_SHIFT;
    case GLFW_KEY_LEFT_CONTROL:
        return BZF_KEY_LEFT_CONTROL;
    case GLFW_KEY_LEFT_ALT:
        return BZF_KEY_LEFT_ALT;
    case GLFW_KEY_LEFT_SUPER:
        return BZF_KEY_LEFT_SUPER;
    case GLFW_KEY_RIGHT_SHIFT:
        return BZF_KEY_RIGHT_SHIFT;
    case GLFW_KEY_RIGHT_CONTROL:
        return BZF_KEY_RIGHT_CONTROL;
    case GLFW_KEY_RIGHT_ALT:
        return BZF_KEY_RIGHT_ALT;
    case GLFW_KEY_RIGHT_SUPER:
        return BZF_KEY_RIGHT_SUPER;
    case GLFW_KEY_MENU:
        return BZF_KEY_MENU;
    default:
        return BZF_KEY_UNKNOWN;
    }
}

int GLFWPlatform::modsFromGLFW(int glfwMods)
{
    int mods = 0;
    if (glfwMods & GLFW_MOD_SHIFT)
        mods = mods | BZF_MOD_SHIFT;
    if (glfwMods & GLFW_MOD_CONTROL)
        mods = mods | BZF_MOD_CTRL;
    if (glfwMods & GLFW_MOD_ALT)
        mods = mods | BZF_MOD_ALT;
    if (glfwMods & GLFW_MOD_SUPER)
        mods = mods | BZF_MOD_SUPER;
    return mods;
}

void GLFWPlatform::error_callback(int error, const char* description)
{
    std::cerr << "Error (" << error << "): " << description << std::endl;
}

///////////////////////////////////////////////////////////
// Window
///////////////////////////////////////////////////////////

GLFWWindow::GLFWWindow(GLFWPlatform *_platform, int width, int height, GLFWMonitor* _monitor, int positionX,
                       int positionY) : platform(_platform), fullscreen(false), gamma(1.0f)
{
    // Create the window
    window = glfwCreateWindow(width, height, "GLFWWindow", nullptr, nullptr);

    // Check if this worked
    if (!window)
    {
        std::cerr << "Error: Unable to create window/context." << std::endl;
        exit(-1);
    }

    // Determine which monitor the window should appear on
    GLFWmonitor* mon;
    if (_monitor == nullptr || _monitor->monitor == nullptr)
        mon = glfwGetPrimaryMonitor();
    else
        mon = _monitor->monitor;

    // Move the window to the requested location, with negative positions meaning centered
    int xOffset, yOffset;
    glfwGetMonitorPos(mon, &xOffset, &yOffset);
    const GLFWvidmode *vmode = glfwGetVideoMode(mon);
    if (positionX < 0)
        positionX = (vmode->width / 2) - (width / 2);
    else
        positionX += xOffset;

    if (positionY < 0)
        positionY = (vmode->height / 2) - (height / 2);
    else
        positionY += yOffset;

    glfwSetWindowPos(window, positionX, positionY);

    glfwSetWindowUserPointer(window, this);
    assignCallbacks();
}

GLFWWindow::GLFWWindow(GLFWPlatform *_platform, BzfResolution resolution, GLFWMonitor *_monitor) : platform(_platform),
    fullscreen(true), gamma(1.0f)
{
    // Refresh rate
    // TODO: Check on the special values for this
    glfwWindowHint(GLFW_REFRESH_RATE, resolution.refreshRate);

    // Determine which monitor to use
    GLFWmonitor* mon;
    if (_monitor == nullptr || _monitor->monitor == nullptr)
        mon = glfwGetPrimaryMonitor();
    else
        mon = _monitor->monitor;

    // Make fullscreen window
    fullscreen = true;
    window = glfwCreateWindow(resolution.width, resolution.height, "GLFWWindow", mon, nullptr);

    if (!window)
    {
        std::cerr << "Error: Unable to create window/context." << std::endl;
        exit(-1);
    }

    glfwSetWindowUserPointer(window, this);
    assignCallbacks();
}

GLFWWindow::~GLFWWindow()
{
    glfwDestroyWindow(window);
}

bool GLFWWindow::isFullscreen() const
{
    return fullscreen;
}

bool GLFWWindow::setVerticalSync(bool sync) const
{
    glfwSwapInterval(sync?1:0);
    return true;
}

bool GLFWWindow::getWindowSize(int &width, int &height) const
{
    glfwGetWindowSize(window, &width, &height);
    return true;
}

bool GLFWWindow::setWindowed(int width, int height, BzfMonitor* _monitor, int x, int y)
{
    // Determine which monitor the window should appear on
    GLFWmonitor* mon;
    if (_monitor == nullptr || static_cast<GLFWMonitor*>(_monitor)->monitor == nullptr)
        mon = glfwGetPrimaryMonitor();
    else
        mon = static_cast<GLFWMonitor*>(_monitor)->monitor;

    // Move the window to the requested location, with negative positions meaning centered
    int xOffset, yOffset;
    glfwGetMonitorPos(mon, &xOffset, &yOffset);
    const GLFWvidmode *vmode = glfwGetVideoMode(mon);
    if (x < 0)
        x = (vmode->width / 2) - (width / 2);
    else
        x += xOffset;

    if (y < 0)
        y = (vmode->height / 2) - (height / 2);
    else
        y += yOffset;

    // Switch to windowed mode
    fullscreen = false;
    glfwSetWindowMonitor(window, nullptr, x, y, width, height, GLFW_DONT_CARE);

    return true;
}

bool GLFWWindow::setFullscreen(BzfResolution resolution, BzfMonitor *monitor)
{
    fullscreen = true;
    glfwSetWindowMonitor(window, static_cast<GLFWMonitor*>(monitor)->monitor, 0, 0, resolution.width, resolution.height,
                         resolution.refreshRate);
    // Set the gamma, since we can't apply it while windowed
    setGamma(getGamma());
    return true;
}

void GLFWWindow::iconify() const
{
    glfwIconifyWindow(window);
}

void GLFWWindow::setMinSize(int width, int height)
{
    glfwSetWindowSizeLimits(window, width, height, GLFW_DONT_CARE, GLFW_DONT_CARE);
}

void GLFWWindow::setTitle(const char *title)
{
    glfwSetWindowTitle(window, title);
}

void GLFWWindow::setIcon(BzfIcon *icon)
{
    GLFWimage image[1];
    image[0].width = icon->width;
    image[0].height = icon->height;
    image[0].pixels = icon->pixel_data;
    glfwSetWindowIcon(window, 1, image);
}

void GLFWWindow::setMouseRelative(bool relative)
{
    glfwSetInputMode(window, GLFW_CURSOR, relative?GLFW_CURSOR_DISABLED:GLFW_CURSOR_NORMAL);
}

void GLFWWindow::setMousePosition(double x, double y)
{
    glfwSetCursorPos(window, x, y);
}

bool GLFWWindow::supportsMouseConfinement()
{
    return false;
}

bool GLFWWindow::setConfineMouse(BzfMouseConfinement /*mode*/, double /*x1*/, double /*y1*/, double /*x2*/,
                                 double /*y2*/)
{
    return false;
}

BzfMouseConfinement GLFWWindow::getConfineMouse()
{
    return BZF_MOUSE_CONFINED_NONE;
}

void GLFWWindow::makeContextCurrent() const
{
    glfwMakeContextCurrent(window);
}

void GLFWWindow::swapBuffers() const
{
    glfwSwapBuffers(window);
}

void GLFWWindow::setGamma(float _gamma)
{
    if (hasGammaControl())
    {
        gamma = _gamma;
        GLFWmonitor *monitor = glfwGetWindowMonitor(window);
        if (monitor != nullptr)
            glfwSetGamma(monitor, gamma);
    }
}

float GLFWWindow::getGamma()
{
    return gamma;
}

bool GLFWWindow::hasGammaControl() const
{
    // GLFW 3.2 on Linux with Xorg throws an "X Error of failed request:  BadMatch (invalid parameter attributes)" when
    // setting gamma. There may have been a problem on macOS as well.
#if (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 3) || GLFW_VERSION_MAJOR > 3
    return true;
#else
    return false;
#endif
}

bool GLFWWindow::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

GLFWwindow *GLFWWindow::getWindow() const
{
    return window;
}

GLFWPlatform *GLFWWindow::getPlatform() const
{
    return platform;
}

void GLFWWindow::assignCallbacks()
{
    glfwSetKeyCallback(window, GLFWPlatform::callKeyCallback);
    glfwSetCursorPosCallback(window, GLFWPlatform::callCursorPosCallback);
    glfwSetMouseButtonCallback(window, GLFWPlatform::callMouseButtonCallback);
    glfwSetScrollCallback(window, GLFWPlatform::callScrollCallback);
    glfwSetFramebufferSizeCallback(window, GLFWPlatform::callResizeCallback);
    glfwSetWindowPosCallback(window, GLFWPlatform::callMoveCallback);
}


///////////////////////////////////////////////////////////
// Joystick / Game Controller
///////////////////////////////////////////////////////////

GLFWJoystick::GLFWJoystick() : joystickID(GLFW_JOYSTICK_LAST)
{

}
GLFWJoystick::~GLFWJoystick()
{

}

std::vector<BzfJoystickInfo*> GLFWJoystick::getJoysticks()
{
    std::vector<BzfJoystickInfo*> joysticks;
    for (int i = 0; i < GLFW_JOYSTICK_LAST; ++i)
    {
        BzfJoystickInfo *joystick = new BzfJoystickInfo;
        if (glfwJoystickPresent(i))
        {
            joystick->id = i;
            // GLFW does not provide the GUID until 3.3
#if (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 3) || GLFW_VERSION_MAJOR > 3
            joystick->guid = glfwGetJoystickGUID(i);
#else
            joystick->guid = "";
#endif
            joystick->name = glfwGetJoystickName(i);
            glfwGetJoystickAxes(i, &joystick->axes);
#if (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 3) || GLFW_VERSION_MAJOR > 3
            glfwGetJoystickHats(i, &joystick->hats);
#else
            joystick->hats = 0;
#endif
            glfwGetJoystickButtons(i, &joystick->buttons);
            joystick->isRumbleSupported = false;
            joystick->isGameController = true; // TODO: Check if GLFW always uses the game controller interface
            joysticks.push_back(joystick);
        }
    }

    return joysticks;
}

bool GLFWJoystick::openDevice(int id)
{
    if (id <= GLFW_JOYSTICK_LAST && glfwJoystickPresent(id))
    {
        joystickID = id;
        return true;
    }
    else
    {
        joystickID = GLFW_JOYSTICK_LAST;
        return false;
    }
}

void GLFWJoystick::closeDevice()
{

}

// Joystick information
const char *GLFWJoystick::getName()
{
    return glfwGetJoystickName(joystickID);
}
int GLFWJoystick::getNumAxes()
{
    if (joystickID < 0 || !glfwJoystickPresent(joystickID))
        return 0;
    int count;
    glfwGetJoystickAxes(joystickID, &count);
    return count;
}

int GLFWJoystick::getNumHats()
{
    return 0;
}

int GLFWJoystick::getNumButtons()
{
    if (joystickID < 0 || !glfwJoystickPresent(joystickID))
        return 0;
    int count;
    glfwGetJoystickButtons(joystickID, &count);
    return count;
}

bool GLFWJoystick::isRumbleSupported()
{
    return false;
}

// Rumble feedback
void GLFWJoystick::rumble(float /*strength*/, unsigned int /*duration*/)
{
    return;
}

float GLFWJoystick::getAxis(int axis)
{
    int count;
    const float *axes = glfwGetJoystickAxes(joystickID, &count);
    if (axis >= count)
        return 0.0f;
    return axes[axis];
}

int GLFWJoystick::getJoystickID() const
{
    return joystickID;
}
