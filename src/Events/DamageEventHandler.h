#pragma once

namespace LossGauge
{
    class DamageEventHandler :
        public RE::BSTEventSink<RE::TESHitEvent>
    {
    public:
        static DamageEventHandler* GetSingleton();

        void Register();

        void ResetHealthSnapshot();


        RE::BSEventNotifyControl ProcessEvent(
            const RE::TESHitEvent* a_event,
            RE::BSTEventSource<RE::TESHitEvent>* a_eventSource)
            override;


    private:
        DamageEventHandler() = default;

        DamageEventHandler(
            const DamageEventHandler&) = delete;

        DamageEventHandler(
            DamageEventHandler&&) = delete;

        DamageEventHandler& operator=(
            const DamageEventHandler&) = delete;

        DamageEventHandler& operator=(
            DamageEventHandler&&) = delete;


        float previousHealth_{ 0.0f };

        bool initialized_{ false };
        bool registered_{ false };
    };
}