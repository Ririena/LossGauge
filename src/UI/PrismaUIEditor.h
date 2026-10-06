#pragma once

#include "External/PrismaUI_API.h"
#include "UI/UIConfig.h"

namespace LossGauge
{
    class PrismaUIEditor
    {
    public:
        static PrismaUIEditor* GetSingleton();

        void Initialize(
            PRISMA_UI_API::IVPrismaUI1* a_api,
            PrismaView a_view);

        void SetDomReady(
            bool a_ready);

        [[nodiscard]]
        bool IsInitialized() const;

        [[nodiscard]]
        bool IsDomReady() const;

        [[nodiscard]]
        bool IsOpen() const;

        [[nodiscard]]
        PrismaView GetView() const;

        bool Open();

        bool Close();

        bool Toggle();

        void Reset();

    private:
        PrismaUIEditor() = default;

        PrismaUIEditor(
            const PrismaUIEditor&) = delete;

        PrismaUIEditor(
            PrismaUIEditor&&) = delete;

        PrismaUIEditor& operator=(
            const PrismaUIEditor&) = delete;

        PrismaUIEditor& operator=(
            PrismaUIEditor&&) = delete;

        [[nodiscard]]
        bool IsViewValid() const;

        [[nodiscard]]
        static bool ParseUIConfig(
            const char* a_argument,
            UIConfig& a_config);

        static void OnCloseRequested(
            const char* a_argument);

        static void OnPreviewRequested(
            const char* a_argument);

        static void OnSaveRequested(
            const char* a_argument);

        PRISMA_UI_API::IVPrismaUI1*
            api_{ nullptr };

        PrismaView
            view_{ 0 };

        bool
            domReady_{ false };

        bool
            open_{ false };

        bool
            listenersRegistered_{ false };
    };
}