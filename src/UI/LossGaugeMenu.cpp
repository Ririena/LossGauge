#include "UI/LossGaugeMenu.h"

namespace LossGauge
{
    LossGaugeMenu::LossGaugeMenu()
    {
        logs::info(
            "LossGaugeMenu instance creating...");

        // HUD-style menu:
        // - does not pause the game
        // - remains open
        // - does not act like an item/application menu

        menuFlags.set(
            RE::UI_MENU_FLAGS::kAlwaysOpen);

        menuFlags.set(
            RE::UI_MENU_FLAGS::kAllowSaving);

        auto* scaleformManager =
            RE::BSScaleformManager::
                GetSingleton();

        if (!scaleformManager) {
            logs::error(
                "LossGaugeMenu: "
                "BSScaleformManager unavailable.");

            return;
        }

        const bool loaded =
            scaleformManager->LoadMovie(
                this,
                uiMovie,
                SWF_PATH.data());

        if (!loaded) {
            logs::error(
                "LossGaugeMenu: "
                "failed to load SWF '{}'.",
                SWF_PATH);

            return;
        }

        logs::info(
            "LossGaugeMenu: "
            "SWF '{}' loaded successfully.",
            SWF_PATH);
    }

    void LossGaugeMenu::Register()
    {
        auto* ui =
            RE::UI::GetSingleton();

        if (!ui) {
            logs::error(
                "LossGaugeMenu registration "
                "failed: UI unavailable.");

            return;
        }

        ui->Register(
            MENU_NAME,
            []() -> RE::IMenu* {
                return new LossGaugeMenu();
            });

        logs::info(
            "LossGaugeMenu registered.");
    }

    void LossGaugeMenu::Open()
    {
        auto* messageQueue =
            RE::UIMessageQueue::
                GetSingleton();

        if (!messageQueue) {
            logs::error(
                "LossGaugeMenu open failed: "
                "UIMessageQueue unavailable.");

            return;
        }

        messageQueue->AddMessage(
            MENU_NAME,
            RE::UI_MESSAGE_TYPE::kShow,
            nullptr);

        logs::info(
            "LossGaugeMenu show message sent.");
    }

    void LossGaugeMenu::Close()
    {
        auto* messageQueue =
            RE::UIMessageQueue::
                GetSingleton();

        if (!messageQueue) {
            logs::error(
                "LossGaugeMenu close failed: "
                "UIMessageQueue unavailable.");

            return;
        }

        messageQueue->AddMessage(
            MENU_NAME,
            RE::UI_MESSAGE_TYPE::kHide,
            nullptr);

        logs::info(
            "LossGaugeMenu hide message sent.");
    }
}