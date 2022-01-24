#include "RenderSystem.h"

namespace engine
{
    RenderSystem::RenderSystem(void* hWindow, bool fullscreen, int width, int height)
        : mWindowWidth(width)
        , mWindowHeight(height)
    {

    }

    bool RenderSystem::OnResizeWindow(void* hWindow, int width, int height)
    {
        mWindowWidth = width;
        mWindowHeight = height;
        return true;
    }
}
