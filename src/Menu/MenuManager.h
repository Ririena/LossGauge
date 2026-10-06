#pragma once

namespace LossGauge
{
    class MenuManager
    {
    public:
        static void Register();

    private:
        static void __stdcall RenderSettings();
        static void __stdcall RenderDebug();

        static bool registered_;
    };
}