#pragma once

#include <algorithm>
#include <cstdint>

namespace LossGauge
{
    struct UIConfig
    {
        // Position

        float positionX{ 56.0f };

        // Distance from bottom of screen.
        float positionY{ 118.0f };


        // Size

        float width{ 246.0f };
        float height{ 4.0f };


        // Color

        std::uint8_t colorR{ 90 };
        std::uint8_t colorG{ 90 };
        std::uint8_t colorB{ 90 };


        // Opacity

        float opacity{ 0.95f };


        // Animation

        bool enableAnimation{ true };

        float animationDuration{ 0.12f };


        // Validation

        void Clamp()
        {
            positionX =
                (std::max)(
                    0.0f,
                    positionX);

            positionY =
                (std::max)(
                    0.0f,
                    positionY);

            width =
                (std::max)(
                    1.0f,
                    width);

            height =
                (std::max)(
                    1.0f,
                    height);

            opacity =
                std::clamp(
                    opacity,
                    0.0f,
                    1.0f);

            animationDuration =
                std::clamp(
                    animationDuration,
                    0.0f,
                    5.0f);
        }
    };
}