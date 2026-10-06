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
            api_->Unfocus(view_);

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


    bool PrismaUIBridge::SendState(
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


    void PrismaUIBridge::Reset()
    {
        domReady_ =
            false;


        logs::info(
            "PrismaUIBridge reset.");
    }
}