#pragma once

#include "External/PrismaUI_API.h"
#include "UI/UIStateManager.h"

namespace LossGauge
{
    class PrismaUIBridge
    {
    public:
        static PrismaUIBridge* GetSingleton();

        void Initialize(
            PRISMA_UI_API::IVPrismaUI1* a_api,
            PrismaView a_view);

        void SetDomReady(
            bool a_ready);

        [[nodiscard]]
        bool SendState(
            const UIState& a_state);

        void Reset();

    private:
        PrismaUIBridge() = default;

        PrismaUIBridge(
            const PrismaUIBridge&) = delete;

        PrismaUIBridge(
            PrismaUIBridge&&) = delete;

        PrismaUIBridge& operator=(
            const PrismaUIBridge&) = delete;

        PrismaUIBridge& operator=(
            PrismaUIBridge&&) = delete;

        PRISMA_UI_API::IVPrismaUI1*
            api_{ nullptr };

        PrismaView
            view_{ 0 };

        bool
            domReady_{ false };
    };
}