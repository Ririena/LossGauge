#include "UI/PrismaUIBridge.h"

#include "Config/ConfigManager.h"

#include <cstdio>

namespace LossGauge
{
    PrismaUIBridge*
    PrismaUIBridge::GetSingleton()
    {
        static PrismaUIBridge singleton;

        return &singleton;
    }


    void PrismaUIBridge::Initialize(
        PRISMA_UI_API::IVPrismaUI1* a_api,
        PrismaView a_view)
    {
        api_ =
            a_api;

        view_ =
            a_view;

        domReady_ =
            false;

        editorPreview_ =
            false;

        hasLastState_ =
            false;

        lastState_ =
            UIState{};


        logs::info(
            "PrismaUIBridge initialized. View: {}",
            view_);


        EnsureUnfocused();
    }


    void PrismaUIBridge::SetDomReady(
        bool a_ready)
    {
        domReady_ =
            a_ready;


        if (!domReady_) {
            return;
        }


        logs::info(
            "PrismaUIBridge DOM ready.");


        EnsureUnfocused();


        if (SendConfig()) {

            logs::info(
                "PrismaUI configuration sent.");
        }
        else {

            logs::warn(
                "Failed to send PrismaUI "
                "configuration.");
        }
    }


    bool PrismaUIBridge::IsReady() const
    {
        if (!api_) {
            return false;
        }


        if (!domReady_) {
            return false;
        }


        if (view_ == 0) {
            return false;
        }


        if (!api_->IsValid(view_)) {
            return false;
        }


        return true;
    }


    void PrismaUIBridge::EnsureUnfocused()
    {
        if (!api_) {
            return;
        }


        if (view_ == 0) {
            return;
        }


        if (!api_->IsValid(view_)) {
            return;
        }


        if (api_->HasFocus(view_)) {

            api_->Unfocus(
                view_);


            logs::info(
                "PrismaUI HUD focus released.");
        }
    }


    bool PrismaUIBridge::SendConfig()
    {
        if (!IsReady()) {
            return false;
        }


        const auto* configManager =
            ConfigManager::
                GetSingleton();


        if (!configManager) {
            return false;
        }


        return SendConfig(
            configManager->
                GetUIConfig());
    }


    bool PrismaUIBridge::SendConfig(
        const UIConfig& a_config)
    {
        if (!IsReady()) {
            return false;
        }


        UIConfig ui =
            a_config;


        ui.Clamp();


        char script[512]{};


        std::snprintf(
            script,
            sizeof(script),

            "setUIConfig("
            "%.4f,"
            "%.4f,"
            "%.4f,"
            "%.4f,"
            "%u,"
            "%u,"
            "%u,"
            "%.4f,"
            "%s,"
            "%.4f"
            ");",

            ui.positionX,
            ui.positionY,
            ui.width,
            ui.height,

            static_cast<unsigned int>(
                ui.colorR),

            static_cast<unsigned int>(
                ui.colorG),

            static_cast<unsigned int>(
                ui.colorB),

            ui.opacity,

            ui.enableAnimation ?
                "true" :
                "false",

            ui.animationDuration);


        api_->Invoke(
            view_,
            script);


        return true;
    }


    bool PrismaUIBridge::SendStateInternal(
        const UIState& a_state)
    {
        if (!IsReady()) {
            return false;
        }


        const float lossPercent =
            a_state.lossPct *
            100.0f;


        const float recoverablePercent =
            a_state.recoverablePct *
            100.0f;


        char script[256]{};


        std::snprintf(
            script,
            sizeof(script),

            "setLossState("
            "%.4f,"
            "%.4f"
            ");",

            lossPercent,
            recoverablePercent);


        api_->Invoke(
            view_,
            script);


        return true;
    }


    bool PrismaUIBridge::SendState(
        const UIState& a_state)
    {
        // REAL gameplay state
        
        // This cache always stores the real state
        // coming from UIStateManager / gameplay.

        // Editor preview never changes this state.

        lastState_ =
            a_state;

        hasLastState_ =
            true;


        // Editor Preview

        if (editorPreview_) {

            UIState previewState =
                a_state;


            // Force the visual Loss fill to 100%.
            //
            // This affects only PrismaUI HUD.
            // It does NOT modify LossManager.

            previewState.lossPct =
                1.0f;


            previewState.recoverablePct =
                0.0f;


            return SendStateInternal(
                previewState);
        }


        // Normal gameplay

        return SendStateInternal(
            a_state);
    }


    void PrismaUIBridge::SetEditorPreview(
        bool a_enabled)
    {
        editorPreview_ =
            a_enabled;


        if (!IsReady()) {
            return;
        }


        // Enable Preview

        if (editorPreview_) {

            logs::info(
                "PrismaUI HUD editor preview "
                "enabled.");


            UIState previewState{};


            if (hasLastState_) {

                previewState =
                    lastState_;
            }


            // Full Loss fill so position, width,
            // height, color and opacity are always
            // visible while editing.

            previewState.lossPct =
                1.0f;


            previewState.recoverablePct =
                0.0f;


            (void)SendStateInternal(
                previewState);


            return;
        }


        // Disable Preview

        logs::info(
            "PrismaUI HUD editor preview "
            "disabled.");


        // Immediately restore the latest REAL
        // gameplay state.

        if (hasLastState_) {

            (void)SendStateInternal(
                lastState_);
        }
    }


    bool PrismaUIBridge::
        IsEditorPreviewEnabled() const
    {
        return editorPreview_;
    }


    void PrismaUIBridge::Reset()
    {
        domReady_ =
            false;

        editorPreview_ =
            false;

        hasLastState_ =
            false;

        lastState_ =
            UIState{};


        logs::info(
            "PrismaUIBridge reset.");
    }
}