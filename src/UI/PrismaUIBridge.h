#pragma once

#include "External/PrismaUI_API.h"
#include "UI/UIConfig.h"
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

        [[nodiscard]]
        bool SendConfig();

        [[nodiscard]]
        bool SendConfig(
            const UIConfig& a_config);

        void SetEditorPreview(
            bool a_enabled);

        [[nodiscard]]
        bool IsEditorPreviewEnabled() const;

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

        [[nodiscard]]
        bool IsReady() const;

        [[nodiscard]]
        bool SendStateInternal(
            const UIState& a_state);

        void EnsureUnfocused();

        PRISMA_UI_API::IVPrismaUI1*
            api_{ nullptr };

        PrismaView
            view_{ 0 };

        bool
            domReady_{ false };

        bool
            editorPreview_{ false };

        bool
            hasLastState_{ false };

        UIState
            lastState_{};
    };
}