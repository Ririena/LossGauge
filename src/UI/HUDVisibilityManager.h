#pragma once

namespace LossGauge
{
    class HUDVisibilityManager :
        public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
    public:
        static HUDVisibilityManager* GetSingleton();

        void Register();
        void Refresh();

        [[nodiscard]]
        bool IsHUDAllowed() const;

        RE::BSEventNotifyControl ProcessEvent(
            const RE::MenuOpenCloseEvent* a_event,
            RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) override;

    private:
        HUDVisibilityManager() = default;

        HUDVisibilityManager(
            const HUDVisibilityManager&) = delete;

        HUDVisibilityManager(
            HUDVisibilityManager&&) = delete;

        HUDVisibilityManager& operator=(
            const HUDVisibilityManager&) = delete;

        HUDVisibilityManager& operator=(
            HUDVisibilityManager&&) = delete;

        [[nodiscard]]
        bool IsBlockingMenu(
            const RE::BSFixedString& a_menuName) const;

        [[nodiscard]]
        bool HasBlockingMenuOpen() const;

        void Apply();

        bool registered_{ false };
        bool hudAllowed_{ true };
    };
}