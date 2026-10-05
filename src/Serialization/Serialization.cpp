#include "Serialization/Serialization.h"

#include "Gameplay/LossManager.h"
#include "Hooks/PlayerUpdateHook.h"

namespace LossGauge::Serialization
{
    namespace
    {
        // "LOSS"

        constexpr std::uint32_t kLossRecord =
            'SSOL';

        constexpr std::uint32_t kLossVersion =
            1;


        void SaveCallback(
            SKSE::SerializationInterface* a_interface)
        {
            if (!a_interface) {
                return;
            }

            auto* manager =
                LossManager::GetSingleton();

            const float loss =
                manager->GetLoss();


            logs::info(
                "================================");

            logs::info(
                "Serialization Save");

            logs::info(
                "--------------------------------");

            logs::info(
                "Saving Loss: {:.2f}",
                loss);


            if (!a_interface->OpenRecord(
                    kLossRecord,
                    kLossVersion)) {

                logs::error(
                    "Failed to open Loss "
                    "serialization record.");

                logs::info(
                    "================================");

                return;
            }


            if (!a_interface->WriteRecordData(
                    std::addressof(loss),
                    sizeof(loss))) {

                logs::error(
                    "Failed to write Loss "
                    "serialization data.");

                logs::info(
                    "================================");

                return;
            }


            logs::info(
                "Loss serialization saved.");

            logs::info(
                "================================");
        }


        void LoadCallback(
            SKSE::SerializationInterface* a_interface)
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


            std::uint32_t type = 0;
            std::uint32_t version = 0;
            std::uint32_t length = 0;

            bool foundLossRecord = false;


            while (a_interface->GetNextRecordInfo(
                type,
                version,
                length)) {

                if (type != kLossRecord) {
                    logs::warn(
                        "Unknown serialization record: "
                        "Type {:08X}, Version {}, "
                        "Length {}",
                        type,
                        version,
                        length);

                    continue;
                }


                if (version != kLossVersion) {
                    logs::warn(
                        "Unsupported Loss record "
                        "version: {}",
                        version);

                    continue;
                }


                if (length != sizeof(float)) {
                    logs::error(
                        "Invalid Loss record size: {}",
                        length);

                    continue;
                }


                float loadedLoss = 0.0f;


                const auto bytesRead =
                    a_interface->ReadRecordData(
                        std::addressof(loadedLoss),
                        sizeof(loadedLoss));


                if (bytesRead != sizeof(loadedLoss)) {
                    logs::error(
                        "Failed to read complete "
                        "Loss record.");

                    continue;
                }


                auto* manager =
                    LossManager::GetSingleton();


                manager->SetLoss(
                    loadedLoss);


                foundLossRecord =
                    true;


                logs::info(
                    "Loaded Loss: {:.2f}",
                    manager->GetLoss());
            }


            // Important for old saves that do not yet
            // contain Loss Gauge serialization data.

            if (!foundLossRecord) {
                logs::info(
                    "No Loss serialization record "
                    "found.");

                logs::info(
                    "Initializing Loss to 0.");

                LossManager::GetSingleton()->
                    ResetLoss();
            }


            // The loaded Loss may change Recoverable HP.
            // Enforce the restored cap immediately.

            auto* manager =
                LossManager::GetSingleton();


            manager->ClampCurrentHealth();


            // Clamp may have changed HP.
            // Synchronize damage detection baseline.

            PlayerUpdateHook::
                ResetHealthSnapshot();


            logs::info(
                "Restored Loss:       {:.2f}",
                manager->GetLoss());

            logs::info(
                "Max HP:              {:.2f}",
                manager->GetMaxHealth());

            logs::info(
                "Recoverable HP:      {:.2f}",
                manager->GetRecoverableHealth());

            logs::info(
                "Current HP:          {:.2f}",
                manager->GetCurrentHealth());

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


            // Critical for save isolation.
            
            // Skyrim calls Revert when the current
            // serialized state is being discarded.

            LossManager::GetSingleton()->
                ResetLoss();


            PlayerUpdateHook::
                ResetHealthSnapshot();


            logs::info(
                "Runtime Loss state cleared.");

            logs::info(
                "================================");
        }
    }


    void Register()
    {
        auto* serialization =
            SKSE::GetSerializationInterface();


        if (!serialization) {
            logs::critical(
                "Failed to get SKSE "
                "Serialization Interface.");

            return;
        }


        serialization->SetUniqueID(
            'GSSL');


        serialization->SetSaveCallback(
            SaveCallback);


        serialization->SetLoadCallback(
            LoadCallback);


        serialization->SetRevertCallback(
            RevertCallback);


        logs::info(
            "SKSE serialization registered.");
    }
}