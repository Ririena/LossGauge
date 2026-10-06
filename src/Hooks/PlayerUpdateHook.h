#pragma once

namespace LossGauge
{
    class PlayerUpdateHook
    {
    public:
        static void Install();

        static void ResetHealthSnapshot();

        static float GetLastGameHours();

        static void ResetGameTimeSnapshot();


    private:
        static void Update(
            RE::PlayerCharacter* a_player,
            float a_delta);


        static inline REL::Relocation<
            decltype(Update)> originalUpdate_;


        static inline float previousHealth_{
            0.0f
        };


        static inline bool initialized_{
            false
        };


        static inline float lastGameHours_{
            0.0f
        };


        static inline bool gameTimeInitialized_{
            false
        };
    };
}