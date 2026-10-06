#include "UI/PrismaUIBridge.h"

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
            "PrismaUIBridge initialized. "
            "View: {}",
            view_);
    }

    void PrismaUIBridge::SetDomReady(
        bool a_ready)
    {
        domReady_ =
            a_ready;

        if (domReady_) {
            logs::info(
                "PrismaUIBridge DOM ready.");
        }
    }

    bool PrismaUIBridge::SendState(
        const UIState& a_state)
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

        if (!api_->
                IsValid(view_)) {

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
            "setLossState(%.4f, %.4f);",
            lossPercent,
            recoverablePercent);

        api_->
            Invoke(
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