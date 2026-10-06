#include "UI/HUDVisibilityManager.h"

#include "UI/PrismaUIBridge.h"

namespace LossGauge
{
    HUDVisibilityManager*
    HUDVisibilityManager::GetSingleton()
    {
        static HUDVisibilityManager singleton;
        return &singleton;
    }


    void HUDVisibilityManager::Register()
    {
        if (registered_) {
            Refresh();
            return;
        }

        auto* ui =
            RE::UI::GetSingleton();

        if (!ui) {
            logs::error(
                "Failed to register HUD visibility: "
                "RE::UI unavailable.");

            return;
        }

        ui->AddEventSink<
            RE::MenuOpenCloseEvent>(
                this);

        registered_ =
            true;

        logs::info(
            "HUD visibility manager registered.");

        Refresh();
    }


    void HUDVisibilityManager::Refresh()
    {
        hudAllowed_ =
            !HasBlockingMenuOpen();

        Apply();
    }


    bool HUDVisibilityManager::
        IsHUDAllowed() const
    {
        return hudAllowed_;
    }


    RE::BSEventNotifyControl
    HUDVisibilityManager::ProcessEvent(
        const RE::MenuOpenCloseEvent* a_event,
        RE::BSTEventSource<
            RE::MenuOpenCloseEvent>*)
    {
        if (!a_event) {
            return RE::BSEventNotifyControl::
                kContinue;
        }

        if (!IsBlockingMenu(
                a_event->menuName)) {

            return RE::BSEventNotifyControl::
                kContinue;
        }

        // Recheck all blocking menus instead of
        // assuming that closing one menu means the
        // HUD can immediately be shown again.

        Refresh();

        return RE::BSEventNotifyControl::
            kContinue;
    }


    bool HUDVisibilityManager::IsBlockingMenu(
        const RE::BSFixedString& a_menuName) const
    {
        return
            a_menuName ==
                RE::InventoryMenu::MENU_NAME ||

            a_menuName ==
                RE::MagicMenu::MENU_NAME ||

            a_menuName ==
                RE::MapMenu::MENU_NAME ||

            a_menuName ==
                RE::JournalMenu::MENU_NAME ||

            a_menuName ==
                RE::FavoritesMenu::MENU_NAME ||

            a_menuName ==
                RE::Console::MENU_NAME ||

            a_menuName ==
                RE::LoadingMenu::MENU_NAME ||

            a_menuName ==
                RE::MainMenu::MENU_NAME;
    }


    bool HUDVisibilityManager::
        HasBlockingMenuOpen() const
    {
        auto* ui =
            RE::UI::GetSingleton();

        if (!ui) {
            return false;
        }

        return
            ui->IsMenuOpen(
                RE::InventoryMenu::MENU_NAME) ||

            ui->IsMenuOpen(
                RE::MagicMenu::MENU_NAME) ||

            ui->IsMenuOpen(
                RE::MapMenu::MENU_NAME) ||

            ui->IsMenuOpen(
                RE::JournalMenu::MENU_NAME) ||

            ui->IsMenuOpen(
                RE::FavoritesMenu::MENU_NAME) ||

            ui->IsMenuOpen(
                RE::Console::MENU_NAME) ||

            ui->IsMenuOpen(
                RE::LoadingMenu::MENU_NAME) ||

            ui->IsMenuOpen(
                RE::MainMenu::MENU_NAME);
    }


    void HUDVisibilityManager::Apply()
    {
        auto* bridge =
            PrismaUIBridge::GetSingleton();

        if (!bridge) {
            return;
        }

        bridge->SetHUDVisible(
            hudAllowed_);
    }
}