#include "Events/SleepEventHandler.h"

#include "Config/ConfigManager.h"
#include "Gameplay/LossManager.h"
#include "Hooks/PlayerUpdateHook.h"

namespace LossGauge
{
    SleepEventHandler*
    SleepEventHandler::GetSingleton()
    {
        static SleepEventHandler instance;

        return std::addressof(instance);
    }

    void SleepEventHandler::Register()
    {
        auto* eventSourceHolder =
            RE::ScriptEventSourceHolder::GetSingleton();

        if (!eventSourceHolder) {
            logs::critical(
                "SleepEventHandler: "
                "ScriptEventSourceHolder is null.");

            return;
        }

        auto* eventSource =
            eventSourceHolder->
                GetEventSource<
                    RE::TESSleepStopEvent>();

        if (!eventSource) {
            logs::critical(
                "SleepEventHandler: "
                "TESSleepStopEvent source is null.");

            return;
        }

        eventSource->AddEventSink(
            GetSingleton());

        logs::info(
            "SleepEventHandler registered.");
    }

    RE::BSEventNotifyControl
    SleepEventHandler::ProcessEvent(
        const RE::TESSleepStopEvent* a_event,
        RE::BSTEventSource<
            RE::TESSleepStopEvent>*)
    {
        if (!a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }

        logs::info(
            "TESSleepStopEvent received. "
            "Interrupted: {}",
            a_event->interrupted);

        // -----------------------------------------
        // Get game-time information
        // -----------------------------------------

        auto* calendar =
            RE::Calendar::GetSingleton();

        if (!calendar) {
            logs::warn(
                "Calendar is null. "
                "Sleep recovery skipped.");

            PlayerUpdateHook::
                ResetGameTimeSnapshot();

            PlayerUpdateHook::
                ResetHealthSnapshot();

            return RE::BSEventNotifyControl::kContinue;
        }

        const float gameHoursBefore =
            PlayerUpdateHook::
                GetLastGameHours();

        const float gameHoursAfter =
            calendar->
                GetHoursPassed();

        const float sleepHours =
            gameHoursAfter -
            gameHoursBefore;

        // -----------------------------------------
        // Measurement log
        // -----------------------------------------

        logs::info(
            "Sleep time measurement:");

        logs::info(
            "Game Hours Before: {:.4f}",
            gameHoursBefore);

        logs::info(
            "Game Hours After:  {:.4f}",
            gameHoursAfter);

        logs::info(
            "Measured Hours:    {:.4f}",
            sleepHours);

        if (a_event->interrupted) {
            logs::info(
                "Sleep was interrupted. "
                "Actual elapsed sleep time "
                "will be used for recovery.");
        }

        // -----------------------------------------
        // Config
        // -----------------------------------------

        auto* config =
            ConfigManager::GetSingleton();

        if (!config->
                IsSleepRecoveryEnabled()) {

            logs::info(
                "Sleep recovery disabled.");

            PlayerUpdateHook::
                ResetGameTimeSnapshot();

            PlayerUpdateHook::
                ResetHealthSnapshot();

            return RE::BSEventNotifyControl::kContinue;
        }

        // -----------------------------------------
        // Validate actual elapsed sleep time
        // -----------------------------------------
        //
        // Important:
        //
        // interrupted == true does NOT automatically
        // mean zero recovery.
        //
        // Example:
        //
        // Requested: 8h
        // ESC after: 6h
        //
        // sleepHours = 6h
        //
        // Those 6 actual elapsed hours should count.
        // -----------------------------------------

        if (!std::isfinite(sleepHours)) {
            logs::warn(
                "Invalid measured sleep "
                "duration: non-finite value.");

            PlayerUpdateHook::
                ResetGameTimeSnapshot();

            PlayerUpdateHook::
                ResetHealthSnapshot();

            return RE::BSEventNotifyControl::kContinue;
        }

        // -----------------------------------------
        // No actual time passed
        // -----------------------------------------

        if (sleepHours <= 0.0f) {
            if (a_event->interrupted) {
                logs::info(
                    "Sleep cancelled before any "
                    "measurable game time passed. "
                    "No Loss recovered.");
            }
            else {
                logs::info(
                    "No measurable sleep time "
                    "passed. No Loss recovered.");
            }

            PlayerUpdateHook::
                ResetGameTimeSnapshot();

            PlayerUpdateHook::
                ResetHealthSnapshot();

            return RE::BSEventNotifyControl::kContinue;
        }

        // -----------------------------------------
        // Sanity check
        // -----------------------------------------
        //
        // Keep the existing safety bound for now.
        // We can revisit >24h sleep support later.
        // -----------------------------------------

        if (sleepHours > 24.5f) {
            logs::warn(
                "Measured sleep duration is "
                "unexpectedly large: {:.4f}h. "
                "Loss recovery skipped.",
                sleepHours);

            PlayerUpdateHook::
                ResetGameTimeSnapshot();

            PlayerUpdateHook::
                ResetHealthSnapshot();

            return RE::BSEventNotifyControl::kContinue;
        }

        const float fullRecoveryHours =
            config->
                GetFullRecoveryHours();

        if (!std::isfinite(
                fullRecoveryHours) ||
            fullRecoveryHours <= 0.0f) {

            logs::warn(
                "Invalid FullRecoveryHours: "
                "{:.2f}.",
                fullRecoveryHours);

            PlayerUpdateHook::
                ResetGameTimeSnapshot();

            PlayerUpdateHook::
                ResetHealthSnapshot();

            return RE::BSEventNotifyControl::kContinue;
        }

        // -----------------------------------------
        // Loss state before recovery
        // -----------------------------------------

        auto* manager =
            LossManager::GetSingleton();

        const float lossBefore =
            manager->GetLoss();

        const float recoverableBefore =
            manager->
                GetRecoverableHealth();

        const float recoveryHoursBefore =
            manager->
                GetRecoveryHours();

        // -----------------------------------------
        // Cumulative recovery
        // -----------------------------------------
        //
        // RecoverFromSleep() already handles:
        //
        // 4h + 4h = 8h
        // 6h + 2h = 8h
        // 2h + 2h + 4h = 8h
        //
        // Interrupted sleep simply contributes
        // whatever ACTUAL game-time elapsed.
        // -----------------------------------------

        manager->RecoverFromSleep(
            sleepHours,
            fullRecoveryHours);

        // -----------------------------------------
        // State after recovery
        // -----------------------------------------

        const float lossAfter =
            manager->GetLoss();

        const float recoverableAfter =
            manager->
                GetRecoverableHealth();

        const float recoveryHoursAfter =
            manager->
                GetRecoveryHours();

        // -----------------------------------------
        // Synchronize snapshots
        // -----------------------------------------
        //
        // Do this after processing recovery so the
        // next sleep starts from the new game time.
        // -----------------------------------------

        PlayerUpdateHook::
            ResetGameTimeSnapshot();

        PlayerUpdateHook::
            ResetHealthSnapshot();

        // -----------------------------------------
        // Result log
        // -----------------------------------------

        logs::info(
            "================================");

        if (a_event->interrupted) {
            logs::info(
                "Interrupted Sleep Recovery Result");
        }
        else {
            logs::info(
                "Sleep Recovery Result");
        }

        logs::info(
            "--------------------------------");

        logs::info(
            "Interrupted:           {}",
            a_event->interrupted);

        logs::info(
            "Actual Sleep Hours:    {:.2f}",
            sleepHours);

        logs::info(
            "Full Recovery Hours:   {:.2f}",
            fullRecoveryHours);

        logs::info(
            "Recovery Hours Before: {:.2f}",
            recoveryHoursBefore);

        logs::info(
            "Recovery Hours After:  {:.2f}",
            recoveryHoursAfter);

        logs::info(
            "Loss Before:           {:.2f}",
            lossBefore);

        logs::info(
            "Loss After:            {:.2f}",
            lossAfter);

        logs::info(
            "Loss Recovered:        {:.2f}",
            (std::max)(
                0.0f,
                lossBefore -
                    lossAfter));

        logs::info(
            "Recoverable Before:    {:.2f}",
            recoverableBefore);

        logs::info(
            "Recoverable After:     {:.2f}",
            recoverableAfter);

        logs::info(
            "================================");

        return RE::BSEventNotifyControl::kContinue;
    }
}