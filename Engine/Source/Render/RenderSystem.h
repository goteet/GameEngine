#pragma once
#include "GEInclude.h"

namespace engine
{
    class RenderSystem : public GE::RenderSystem
    {
    public:
        DefineRTTI;

        virtual unsigned int GetWindowWidth() const override { return mWindowWidth; }

        virtual unsigned int GetWindowHeight() const override { return mWindowHeight; }

        virtual math::point3d<float> ScreenToView(const math::point3d<int>& screen) const override { return math::point3d<float>::zero(); }

        virtual math::point3d<int> ViewToScreen(const math::point3d<float>& view) const override { return math::point3d<int>::zero(); }

        RenderSystem(void* hWindow, bool fullscreen, int width, int height);

        bool OnResizeWindow(void* hWindow, int width, int height);

    private:
        int mWindowWidth = 0;
        int mWindowHeight = 0;
    };
}
