#include "BzfPlatform.h"

void BzfPlatform::addResizeCallback(std::function<void(BzfPlatform *, BzfWindow *, int, int)> callback)
{
    resizeCallbacks.push_back(callback);
}

void BzfPlatform::addMoveCallback(std::function<void(BzfPlatform *, BzfWindow *, int, int)> callback)
{
    moveCallbacks.push_back(callback);
}

void BzfPlatform::setKeyCallback(std::function<void(BzfPlatform *, BzfWindow *, BzfKey, BzfKeyAction, int)> callback)
{
    keyCallback = callback;
}

void BzfPlatform::setTextCallback(std::function<void(BzfPlatform *, BzfWindow *, char[32])> callback)
{
    textCallback = callback;
}

void BzfPlatform::setCursorPosCallback(std::function<void(BzfPlatform *, BzfWindow *, double, double)> callback)
{
    cursorPosCallback = callback;
}

void BzfPlatform::setMouseButtonCallback(
    std::function<void(BzfPlatform *, BzfWindow *, BzfMouseButton, BzfButtonAction, int)> callback)
{
    mouseButtonCallback = callback;
}

void BzfPlatform::setScrollCallback(std::function<void(BzfPlatform *, BzfWindow *, double, double)> callback)
{
    scrollCallback = callback;
}

void BzfPlatform::setJoystickButtonCallback(
    std::function<void(BzfPlatform *, BzfWindow *, BzfJoyButton, BzfButtonAction)> callback)
{
    joystickButtonCallback = callback;
}

void BzfPlatform::setJoystickHatCallback(std::function<void(BzfPlatform *, BzfWindow *, BzfJoyHat, BzfJoyHatDirection)>
        callback)
{
    joystickHatCallback = callback;
}
