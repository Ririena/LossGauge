#pragma once

namespace LossGauge
{
    class LossGaugeMenu :
        public RE::IMenu
    {
    public:
        static constexpr std::string_view
            MENU_NAME = "LossGaugeMenu";

        static constexpr std::string_view
            SWF_PATH = "LossGauge/LossGauge";

        LossGaugeMenu();

        static void Register();
        static void Open();
        static void Close();
    };
}