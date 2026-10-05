#pragma once

namespace LossGauge
{
    class SleepEventHandler :
        public RE::BSTEventSink<
            RE::TESSleepStopEvent>
    {
    public:
        static SleepEventHandler*
            GetSingleton();

        static void Register();

        RE::BSEventNotifyControl ProcessEvent(
            const RE::TESSleepStopEvent* a_event,
            RE::BSTEventSource<
                RE::TESSleepStopEvent>* a_eventSource)
            override;

    private:
        SleepEventHandler() = default;

        SleepEventHandler(
            const SleepEventHandler&) = delete;

        SleepEventHandler(
            SleepEventHandler&&) = delete;

        SleepEventHandler& operator=(
            const SleepEventHandler&) = delete;

        SleepEventHandler& operator=(
            SleepEventHandler&&) = delete;
    };
}