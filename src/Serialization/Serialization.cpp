#include "Serialization/Serialization.h"

#include "Gameplay/LossManager.h"
#include "Gameplay/NaturalRegenController.h"
#include "Hooks/PlayerUpdateHook.h"

namespace LossGauge::Serialization
{
    namespace
    {
        // Keep the existing IDs.
        //
        // Changing these would break compatibility
        // with existing Loss Gauge saves.
        constexpr std::uint32_t kLossRecord =
            'SSOL';

        constexpr std::uint32_t kVersion1 =
            1;

        constexpr std::uint32_t kVersion2 =
            2;

        struct StateV2
        {
            float loss;
            float recoveryBaseLoss;
            float recoveryHours;
        };

        static_assert(
            sizeof(StateV2) ==
            sizeof(float) * 3);

        void SaveCallback(
            SKSE::SerializationInterface*
                a_interface)
        {
            if (!a_interface) {
                return;
            }

            auto* manager =
                LossManager::GetSingleton();

            StateV2 state{
                manager->GetLoss(),
                manager->GetRecoveryBaseLoss(),
                manager->GetRecoveryHours()
            };

            logs::info(
                "================================");

            logs::info(
                "Serialization Save");

            logs::info(
                "--------------------------------");

            logs::info(
                "Version:             {}",
                kVersion2);

            logs::info(
                "Saving Loss:         {:.2f}",
                state.loss);

            logs::info(
                "Recovery Base Loss:  {:.2f}",
                state.recoveryBaseLoss);

            logs::info(
                "Recovery Hours:      {:.2f}",
                state.recoveryHours);

            if (!a_interface->
                    OpenRecord(
                        kLossRecord,
                        kVersion2)) {

                logs::error(
                    "Failed to open Loss "
                    "serialization record.");

                logs::info(
                    "================================");

                return;
            }

            if (!a_interface->
                    WriteRecordData(
                        std::addressof(state),
                        sizeof(state))) {

                logs::error(
                    "Failed to write Loss Gauge "
                    "serialization state.");

                logs::info(
                    "================================");

                return;
            }

            logs::info(
                "Loss Gauge v2 state saved.");

            logs::info(
                "================================");
        }

        void LoadV1(
            SKSE::SerializationInterface*
                a_interface,
            std::uint32_t a_length)
        {
            if (a_length != sizeof(float)) {
                logs::error(
                    "Invalid v1 record size: {}",
                    a_length);

                return;
            }

            float loadedLoss =
                0.0f;

            const auto bytesRead =
                a_interface->
                    ReadRecordData(
                        std::addressof(
                            loadedLoss),
                        sizeof(loadedLoss));

            if (bytesRead !=
                sizeof(loadedLoss)) {

                logs::error(
                    "Failed to read v1 Loss "
                    "record.");

                return;
            }

            // ====================================
            // v1 -> v2 migration
            // ====================================
            //
            // v1 only stored Loss.
            //
            // Therefore there is no legitimate
            // recovery progress to restore.
            //
            // Start a fresh recovery cycle from
            // the loaded Loss.

            LossManager::
                GetSingleton()->
                RestoreState(
                    loadedLoss,
                    loadedLoss,
                    0.0f);

            logs::info(
                "Migrated serialization "
                "v1 -> v2.");

            logs::info(
                "Loaded Loss: {:.2f}",
                loadedLoss);

            logs::info(
                "Recovery progress initialized "
                "to 0h.");
        }

        void LoadV2(
            SKSE::SerializationInterface*
                a_interface,
            std::uint32_t a_length)
        {
            if (a_length != sizeof(StateV2)) {
                logs::error(
                    "Invalid v2 record size: {} "
                    "(expected {})",
                    a_length,
                    sizeof(StateV2));

                return;
            }

            StateV2 state{};

            const auto bytesRead =
                a_interface->
                    ReadRecordData(
                        std::addressof(state),
                        sizeof(state));

            if (bytesRead !=
                sizeof(state)) {

                logs::error(
                    "Failed to read complete "
                    "v2 Loss Gauge state.");

                return;
            }

            LossManager::
                GetSingleton()->
                RestoreState(
                    state.loss,
                    state.recoveryBaseLoss,
                    state.recoveryHours);

            logs::info(
                "Loaded Loss:         {:.2f}",
                state.loss);

            logs::info(
                "Recovery Base Loss:  {:.2f}",
                state.recoveryBaseLoss);

            logs::info(
                "Recovery Hours:      {:.2f}",
                state.recoveryHours);
        }

        void LoadCallback(
            SKSE::SerializationInterface*
                a_interface)
        {
            if (!a_interface) {
                return;
            }

            logs::info(
                "================================");

            logs::info(
                "Serialization Load");

            logs::info(
                "--------------------------------");

            std::uint32_t type =
                0;

            std::uint32_t version =
                0;

            std::uint32_t length =
                0;

            bool foundRecord =
                false;

            while (
                a_interface->
                    GetNextRecordInfo(
                        type,
                        version,
                        length)) {

                if (type != kLossRecord) {
                    logs::warn(
                        "Unknown serialization "
                        "record: Type {:08X}, "
                        "Version {}, Length {}",
                        type,
                        version,
                        length);

                    continue;
                }

                foundRecord =
                    true;

                logs::info(
                    "Found Loss Gauge record.");

                logs::info(
                    "Record Version: {}",
                    version);

                logs::info(
                    "Record Length:  {}",
                    length);

                switch (version) {
                case kVersion1:
                    LoadV1(
                        a_interface,
                        length);
                    break;

                case kVersion2:
                    LoadV2(
                        a_interface,
                        length);
                    break;

                default:
                    logs::error(
                        "Unsupported Loss Gauge "
                        "serialization version: {}",
                        version);
                    break;
                }
            }

            if (!foundRecord) {
                logs::info(
                    "No Loss Gauge serialization "
                    "record found.");

                logs::info(
                    "Initializing fresh state.");

                LossManager::
                    GetSingleton()->
                    ResetLoss();
            }

            auto* manager =
                LossManager::GetSingleton();

            manager->
                ClampCurrentHealth();

            PlayerUpdateHook::
                ResetHealthSnapshot();

            logs::info(
                "--------------------------------");

            logs::info(
                "Restored Loss:       {:.2f}",
                manager->GetLoss());

            logs::info(
                "Recovery Base:       {:.2f}",
                manager->
                    GetRecoveryBaseLoss());

            logs::info(
                "Recovery Hours:      {:.2f}",
                manager->
                    GetRecoveryHours());

            logs::info(
                "Max HP:              {:.2f}",
                manager->
                    GetMaxHealth());

            logs::info(
                "Recoverable HP:      {:.2f}",
                manager->
                    GetRecoverableHealth());

            logs::info(
                "Current HP:          {:.2f}",
                manager->
                    GetCurrentHealth());

            logs::info(
                "================================");
        }

        void RevertCallback(
            SKSE::SerializationInterface*)
        {
            logs::info(
                "================================");

            logs::info(
                "Serialization Revert");

            logs::info(
                "--------------------------------");

            // Do not touch Skyrim ActorValues here.
            // Only clear our DLL-side state.
            NaturalRegenController::
                GetSingleton()->
                ResetState();

            LossManager::
                GetSingleton()->
                ResetLoss();

            PlayerUpdateHook::
                ResetHealthSnapshot();

            PlayerUpdateHook::
                ResetGameTimeSnapshot();

            logs::info(
                "Runtime Loss state cleared.");

            logs::info(
                "================================");
        }
    }

    void Register()
    {
        auto* serialization =
            SKSE::
                GetSerializationInterface();

        if (!serialization) {
            logs::critical(
                "Failed to get SKSE "
                "Serialization Interface.");

            return;
        }

        // Existing Loss Gauge serialization ID.
        // DO NOT change.
        serialization->
            SetUniqueID(
                'GSSL');

        serialization->
            SetSaveCallback(
                SaveCallback);

        serialization->
            SetLoadCallback(
                LoadCallback);

        serialization->
            SetRevertCallback(
                RevertCallback);

        logs::info(
            "SKSE serialization registered.");
    }
}