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


        // ========================================
        // JavaScript -> C++
        // ========================================

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
            api_->Unfocus(view_);
        }


        api_->Hide(view_);

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
        return
            domReady_;
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


    // ============================================
    // Parse UI configuration sent from JavaScript
    //
    // Payload:
    //
    // X|Y|Width|Height|
    // R|G|B|
    // Opacity|
    // AnimationEnabled|
    // AnimationDuration
    // ============================================

    bool PrismaUIEditor::ParseUIConfig(
        const char* a_argument,
        UIConfig& a_config)
    {
        if (!a_argument) {
            return false;
        }


        UIConfig parsedConfig{};


        unsigned int r = 90;
        unsigned int g = 90;
        unsigned int b = 90;

        int animationEnabled = 1;


        const int parsed =
            std::sscanf(
                a_argument,

                "%f|%f|%f|%f|"
                "%u|%u|%u|"
                "%f|%d|%f",

                &parsedConfig.positionX,
                &parsedConfig.positionY,
                &parsedConfig.width,
                &parsedConfig.height,

                &r,
                &g,
                &b,

                &parsedConfig.opacity,
                &animationEnabled,
                &parsedConfig.animationDuration);


        if (parsed != 10) {
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


        parsedConfig.colorR =
            static_cast<std::uint8_t>(
                r);

        parsedConfig.colorG =
            static_cast<std::uint8_t>(
                g);

        parsedConfig.colorB =
            static_cast<std::uint8_t>(
                b);


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


        // ========================================
        // Load CURRENT ConfigManager values
        // into the editor.
        //
        // This is the baseline used by Cancel.
        // ========================================

        const auto* config =
            ConfigManager::
                GetSingleton();


        if (config) {

            const auto& ui =
                config->
                    GetUIConfig();


            char script[512]{};


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
        }


        // ========================================
        // Show
        // ========================================

        api_->Show(
            view_);


        // ========================================
        // Focus
        //
        // pauseGame        = false
        // disableFocusMenu = true
        //
        // Goal:
        //
        // - Prisma editor remains interactive.
        // - Skyrim gameplay is not paused.
        // - Prisma Focus Menu is disabled.
        // - Test whether Skyrim / Edge UI HUD
        //   remains visible while editing.
        // ========================================

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


        open_ =
            true;


        logs::info(
            "PrismaUIEditor opened "
            "with pauseGame=false, "
            "disableFocusMenu=true.");


        return true;
    }


    bool PrismaUIEditor::Close()
    {
        if (!IsViewValid()) {
            open_ =
                false;

            return false;
        }


        // ========================================
        // Restore ConfigManager state.
        //
        // This makes Close / Cancel / ESC act
        // as rollback for realtime preview.
        //
        // After a successful Save, ConfigManager
        // already contains the new values, so this
        // keeps the newly saved configuration.
        // ========================================

        auto* bridge =
            PrismaUIBridge::
                GetSingleton();


        if (bridge) {
            (void)bridge->
                SendConfig();
        }


        // ========================================
        // Release focus
        // ========================================

        if (api_->HasFocus(view_)) {
            api_->Unfocus(view_);
        }


        // ========================================
        // Hide
        // ========================================

        api_->Hide(view_);

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
        if (IsViewValid()) {

            if (api_->HasFocus(view_)) {
                api_->Unfocus(view_);
            }


            api_->Hide(view_);
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


    // ============================================
    // JavaScript -> C++
    // CLOSE / CANCEL / ESC
    // ============================================

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


    // ============================================
    // JavaScript -> C++
    // REALTIME PREVIEW
    // ============================================

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


    // ============================================
    // JavaScript -> C++
    // SAVE
    // ============================================

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


        // ========================================
        // Commit editor values
        // ========================================

        config->
            SetUIConfig(
                newConfig);


        // ========================================
        // Write LossGauge.toml
        // ========================================

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

            newConfig.enableAnimation ?
                "On" :
                "Off",

            newConfig.animationDuration);


        // ========================================
        // Make sure HUD uses committed values
        // ========================================

        auto* bridge =
            PrismaUIBridge::
                GetSingleton();


        if (bridge) {
            (void)bridge->
                SendConfig();
        }


        // ========================================
        // Close editor
        //
        // Close() calls SendConfig() again.
        // That's harmless and ensures HUD ends in
        // the committed ConfigManager state.
        // ========================================

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