#include "UI/PrismaUIEditor.h"

#include "Config/ConfigManager.h"
#include "UI/PrismaUIBridge.h"

#include <algorithm>
#include <cstdio>

namespace LossGauge
{
    PrismaUIEditor*
    PrismaUIEditor::GetSingleton()
    {
        static PrismaUIEditor singleton;
        return &singleton;
    }


    void PrismaUIEditor::Initialize(
        PRISMA_UI_API::IVPrismaUI1* a_api,
        PrismaView a_view)
    {
        api_ =
            a_api;

        view_ =
            a_view;

        domReady_ =
            false;

        open_ =
            false;

        listenersRegistered_ =
            false;

        logs::info(
            "PrismaUIEditor initialized. "
            "View: {}",
            view_);

        if (!IsViewValid()) {

            logs::error(
                "Cannot register PrismaUIEditor "
                "listeners: view is invalid.");

            return;
        }

        api_->RegisterJSListener(
            view_,
            "closeLossGaugeEditor",
            OnCloseRequested);

        api_->RegisterJSListener(
            view_,
            "previewLossGaugeEditor",
            OnPreviewRequested);

        api_->RegisterJSListener(
            view_,
            "saveLossGaugeEditor",
            OnSaveRequested);

        listenersRegistered_ =
            true;

        logs::info(
            "PrismaUIEditor JS listeners "
            "registered.");
    }


    void PrismaUIEditor::SetDomReady(
        bool a_ready)
    {
        domReady_ =
            a_ready;

        if (!domReady_) {
            return;
        }

        logs::info(
            "PrismaUIEditor DOM ready.");

        if (!IsViewValid()) {

            logs::error(
                "PrismaUIEditor view is invalid "
                "after DOM ready.");

            return;
        }

        if (api_->HasFocus(view_)) {

            api_->Unfocus(
                view_);
        }

        api_->Hide(
            view_);

        open_ =
            false;

        logs::info(
            "PrismaUIEditor hidden and ready.");
    }


    bool PrismaUIEditor::IsInitialized() const
    {
        return
            api_ != nullptr &&
            view_ != 0;
    }


    bool PrismaUIEditor::IsDomReady() const
    {
        return domReady_;
    }


    bool PrismaUIEditor::IsOpen() const
    {
        if (!IsViewValid()) {
            return false;
        }

        return
            open_ &&
            !api_->IsHidden(view_);
    }


    PrismaView
    PrismaUIEditor::GetView() const
    {
        return view_;
    }


    bool PrismaUIEditor::IsViewValid() const
    {
        if (!api_) {
            return false;
        }

        if (view_ == 0) {
            return false;
        }

        return
            api_->IsValid(view_);
    }


    bool PrismaUIEditor::ParseUIConfig(
        const char* a_argument,
        UIConfig& a_config)
    {
        if (!a_argument) {
            return false;
        }

        UIConfig parsedConfig{};

        unsigned int r =
            90;

        unsigned int g =
            90;

        unsigned int b =
            90;

        int borderEnabled =
            0;

        unsigned int borderR =
            0;

        unsigned int borderG =
            0;

        unsigned int borderB =
            0;

        int animationEnabled =
            1;

        const int parsed =
            std::sscanf(
                a_argument,

                "%f|%f|%f|%f|"
                "%u|%u|%u|"
                "%f|"
                "%f|"
                "%d|"
                "%f|"
                "%u|%u|%u|"
                "%f|"
                "%d|"
                "%f",

                &parsedConfig.positionX,
                &parsedConfig.positionY,
                &parsedConfig.width,
                &parsedConfig.height,

                &r,
                &g,
                &b,

                &parsedConfig.opacity,

                &parsedConfig.borderRadius,

                &borderEnabled,

                &parsedConfig.borderWidth,

                &borderR,
                &borderG,
                &borderB,

                &parsedConfig.borderOpacity,

                &animationEnabled,

                &parsedConfig.animationDuration);

        if (parsed != 17) {
            return false;
        }

        r =
            (std::min)(
                r,
                255u);

        g =
            (std::min)(
                g,
                255u);

        b =
            (std::min)(
                b,
                255u);

        borderR =
            (std::min)(
                borderR,
                255u);

        borderG =
            (std::min)(
                borderG,
                255u);

        borderB =
            (std::min)(
                borderB,
                255u);

        parsedConfig.colorR =
            static_cast<std::uint8_t>(
                r);

        parsedConfig.colorG =
            static_cast<std::uint8_t>(
                g);

        parsedConfig.colorB =
            static_cast<std::uint8_t>(
                b);

        parsedConfig.enableBorder =
            borderEnabled != 0;

        parsedConfig.borderColorR =
            static_cast<std::uint8_t>(
                borderR);

        parsedConfig.borderColorG =
            static_cast<std::uint8_t>(
                borderG);

        parsedConfig.borderColorB =
            static_cast<std::uint8_t>(
                borderB);

        parsedConfig.enableAnimation =
            animationEnabled != 0;

        parsedConfig.Clamp();

        a_config =
            parsedConfig;

        return true;
    }


    bool PrismaUIEditor::Open()
    {
        if (!IsViewValid()) {

            logs::warn(
                "Cannot open PrismaUIEditor: "
                "view is unavailable.");

            return false;
        }

        if (!domReady_) {

            logs::warn(
                "Cannot open PrismaUIEditor: "
                "DOM is not ready.");

            return false;
        }

        if (!listenersRegistered_) {

            logs::warn(
                "Cannot open PrismaUIEditor: "
                "JS listeners are unavailable.");

            return false;
        }

        if (IsOpen()) {
            return true;
        }

        const auto* config =
            ConfigManager::
                GetSingleton();

        if (config) {

            UIConfig ui =
                config->
                    GetUIConfig();

            ui.Clamp();

            char script[768]{};

            std::snprintf(
                script,
                sizeof(script),

                "setEditorConfig("
                "%.4f,"
                "%.4f,"
                "%.4f,"
                "%.4f,"
                "%u,"
                "%u,"
                "%u,"
                "%.4f,"
                "%.4f,"
                "%s,"
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

                ui.borderRadius,

                ui.enableBorder ?
                    "true" :
                    "false",

                ui.borderWidth,

                static_cast<unsigned int>(
                    ui.borderColorR),

                static_cast<unsigned int>(
                    ui.borderColorG),

                static_cast<unsigned int>(
                    ui.borderColorB),

                ui.borderOpacity,

                ui.enableAnimation ?
                    "true" :
                    "false",

                ui.animationDuration);

            api_->Invoke(
                view_,
                script);
        }

        api_->Show(
            view_);

        const bool focused =
            api_->Focus(
                view_,
                true,
                false);

        if (!focused) {

            logs::error(
                "Failed to focus "
                "PrismaUIEditor.");

            api_->Hide(
                view_);

            open_ =
                false;

            return false;
        }

        auto* bridge =
            PrismaUIBridge::
                GetSingleton();

        if (bridge) {

            bridge->
                SetEditorPreview(
                    true);
        }

        open_ =
            true;

        logs::info(
            "PrismaUIEditor opened with "
            "pauseGame=true, "
            "disableFocusMenu=false.");

        return true;
    }


    bool PrismaUIEditor::Close()
    {
        if (!IsViewValid()) {

            open_ =
                false;

            return false;
        }

        auto* bridge =
            PrismaUIBridge::
                GetSingleton();

        if (bridge) {

            // Restore committed configuration.

            (void)bridge->
                SendConfig();

            // Restore real gameplay state.

            bridge->
                SetEditorPreview(
                    false);
        }

        if (api_->HasFocus(view_)) {

            api_->Unfocus(
                view_);
        }

        api_->Hide(
            view_);

        open_ =
            false;

        logs::info(
            "PrismaUIEditor closed.");

        return true;
    }


    bool PrismaUIEditor::Toggle()
    {
        if (IsOpen()) {
            return Close();
        }

        return Open();
    }


    void PrismaUIEditor::Reset()
    {
        auto* bridge =
            PrismaUIBridge::
                GetSingleton();

        if (bridge) {

            bridge->
                SetEditorPreview(
                    false);
        }

        if (IsViewValid()) {

            if (api_->HasFocus(view_)) {

                api_->Unfocus(
                    view_);
            }

            api_->Hide(
                view_);
        }

        api_ =
            nullptr;

        view_ =
            0;

        domReady_ =
            false;

        open_ =
            false;

        listenersRegistered_ =
            false;

        logs::info(
            "PrismaUIEditor reset.");
    }


    void PrismaUIEditor::OnCloseRequested(
        const char* a_argument)
    {
        logs::info(
            "PrismaUIEditor close requested "
            "from JavaScript. Argument: {}",
            a_argument ?
                a_argument :
                "");

        auto* editor =
            GetSingleton();

        if (!editor->Close()) {

            logs::error(
                "Failed to close PrismaUIEditor "
                "from JavaScript callback.");
        }
    }


    void PrismaUIEditor::OnPreviewRequested(
        const char* a_argument)
    {
        UIConfig preview{};

        if (!ParseUIConfig(
                a_argument,
                preview)) {

            logs::warn(
                "Invalid PrismaUIEditor "
                "preview payload: {}",
                a_argument ?
                    a_argument :
                    "<null>");

            return;
        }

        auto* bridge =
            PrismaUIBridge::
                GetSingleton();

        if (!bridge) {
            return;
        }

        if (!bridge->
                SendConfig(
                    preview)) {

            logs::warn(
                "Failed to send realtime "
                "Loss Gauge UI preview.");
        }
    }


    void PrismaUIEditor::OnSaveRequested(
        const char* a_argument)
    {
        logs::info(
            "PrismaUIEditor save requested "
            "from JavaScript.");

        UIConfig newConfig{};

        if (!ParseUIConfig(
                a_argument,
                newConfig)) {

            logs::error(
                "Invalid PrismaUIEditor "
                "save payload: {}",
                a_argument ?
                    a_argument :
                    "<null>");

            return;
        }

        auto* config =
            ConfigManager::
                GetSingleton();

        if (!config) {

            logs::error(
                "Cannot save PrismaUIEditor: "
                "ConfigManager unavailable.");

            return;
        }

        // Commit editor values.

        config->
            SetUIConfig(
                newConfig);

        // Write LossGauge.toml.

        if (!config->Save()) {

            logs::error(
                "Failed to save Loss Gauge "
                "UI configuration.");

            return;
        }

        logs::info(
            "Loss Gauge UI configuration "
            "saved from PrismaUI Editor.");

        logs::info(
            "Saved UI: "
            "X={:.1f}, Y={:.1f}, "
            "W={:.1f}, H={:.1f}, "
            "RGB({}, {}, {}), "
            "Opacity={:.2f}, "
            "Radius={:.2f}, "
            "Border={}, "
            "BorderWidth={:.2f}, "
            "BorderRGB({}, {}, {}), "
            "BorderOpacity={:.2f}, "
            "Animation={}, "
            "Duration={:.2f}s",

            newConfig.positionX,
            newConfig.positionY,
            newConfig.width,
            newConfig.height,

            static_cast<int>(
                newConfig.colorR),

            static_cast<int>(
                newConfig.colorG),

            static_cast<int>(
                newConfig.colorB),

            newConfig.opacity,

            newConfig.borderRadius,

            newConfig.enableBorder ?
                "On" :
                "Off",

            newConfig.borderWidth,

            static_cast<int>(
                newConfig.borderColorR),

            static_cast<int>(
                newConfig.borderColorG),

            static_cast<int>(
                newConfig.borderColorB),

            newConfig.borderOpacity,

            newConfig.enableAnimation ?
                "On" :
                "Off",

            newConfig.animationDuration);


        auto* bridge =
            PrismaUIBridge::
                GetSingleton();

        if (bridge) {

            (void)bridge->
                SendConfig();
        }

        // Close editor.

        auto* editor =
            GetSingleton();

        if (!editor->Close()) {

            logs::warn(
                "Configuration was saved, "
                "but PrismaUIEditor could "
                "not be closed.");
        }
    }
}