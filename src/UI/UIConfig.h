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


        // Fill

        std::uint8_t colorR{ 90 };
        std::uint8_t colorG{ 90 };
        std::uint8_t colorB{ 90 };

        float opacity{ 0.95f };


        // Shape

        float borderRadius{ 0.0f };


        // Border

        bool enableBorder{ false };

        float borderWidth{ 1.0f };

        std::uint8_t borderColorR{ 0 };
        std::uint8_t borderColorG{ 0 };
        std::uint8_t borderColorB{ 0 };

        float borderOpacity{ 1.0f };


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

            borderRadius =
                std::clamp(
                    borderRadius,
                    0.0f,
                    100.0f);

            borderWidth =
                std::clamp(
                    borderWidth,
                    0.0f,
                    20.0f);

            borderOpacity =
                std::clamp(
                    borderOpacity,
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