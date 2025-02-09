#ifndef QUEST_MENU_H
#define QUEST_MENU_H

#include "PCH.h"
#include "RE/G/GFxMovieDef.h"
#include "RE/G/GFxValue.h"
#include "RE/G/GPtr.h"
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include "Utility.h"

namespace Scaleform {

    extern bool knotworkStatus;

    class QuestMenu: RE::IMenu {
    public:
        static constexpr const char* MENU_PATH = "questjournal";
        static constexpr const char* MENU_NAME = "QuestMenu";

        QuestMenu();

        static void Register();
        static void Show();
        static void Hide();

        struct QuestObjective {
            std::string text;
            std::string state;
        };

        struct QuestData {
            std::string formID;
            std::string name;
            std::string type;
            std::string category;
            std::vector<Scaleform::QuestMenu::QuestObjective> objectives;
        };

        static std::vector<QuestData> GetRunningQuests();
        static std::string GetQuestCategory(RE::TESQuest* a_quest);
        static std::string GetQuestType(RE::TESQuest* a_quest);
        static std::string GetObjectiveState(RE::BGSQuestObjective* a_objective);
        //static std::string GetQuestStageLogEntry(RE::TESQuest* quest);
        static std::vector<Scaleform::QuestMenu::QuestObjective> GetQuestObjectives(RE::TESQuest* a_quest);
        static bool HasObjective(RE::TESQuest* a_quest);
        static bool HasObjectiveDisplayed(RE::TESQuest* a_quest);

        static constexpr std::string_view Name();

        virtual void Accept(RE::FxDelegateHandler::CallbackProcessor* a_cbReg) override; 

        static RE::stl::owner<RE::IMenu*> Creator() { return new QuestMenu(); }

    private:
        static void PlaySound(const RE::FxDelegateArgs& a_params);
        static void ToggleQuestActive(const RE::FxDelegateArgs& a_params);
        static void RequestQuestLocation(const RE::FxDelegateArgs& a_params);
        static void ShowQuestOnMap(const RE::FxDelegateArgs& a_params);
    };

    constexpr std::string_view QuestMenu::Name() { return QuestMenu::MENU_NAME; }

}

class JournalMenuEx : public RE::JournalMenu {
public:
    enum class Tab : std::uint32_t { kQuest, kPlayerInfo, kSystem };

    void Hook_Accept(RE::FxDelegateHandler::CallbackProcessor* a_cbReg)  // 01
    {
        _Accept(this, a_cbReg);
        fxDelegate->callbacks.Remove("RememberCurrentTabIndex");
        a_cbReg->Process("RememberCurrentTabIndex", RememberCurrentTabIndex);
    }

    RE::UI_MESSAGE_RESULTS Hook_ProcessMessage(RE::UIMessage& a_message)  // 04
    {
        using Message = RE::UI_MESSAGE_TYPE;
        if (a_message.type == Message::kShow) {
            auto ui = RE::UI::GetSingleton();
            auto uiStr = RE::InterfaceStrings::GetSingleton();
            if (ui->IsMenuOpen(uiStr->mapMenu)) {
                *_savedTabIdx = Tab::kQuest;
            } else {
                *_savedTabIdx = Tab::kSystem;
            }
        }

        return _ProcessMessage(this, a_message);
    }

    static void RememberCurrentTabIndex([[maybe_unused]] const RE::FxDelegateArgs& a_params) { return; }

    static void InstallHooks() {
        REL::Relocation<std::uintptr_t> vTable(RE::VTABLE_JournalMenu[0]);
        _Accept = vTable.write_vfunc(0x1, &JournalMenuEx::Hook_Accept);
        _ProcessMessage = vTable.write_vfunc(0x4, &JournalMenuEx::Hook_ProcessMessage);
    }

    using Accept_t = decltype(&RE::JournalMenu::Accept);
    using ProcessMessage_t = decltype(&RE::JournalMenu::ProcessMessage);

    static inline REL::Relocation<Accept_t> _Accept;
    static inline REL::Relocation<ProcessMessage_t> _ProcessMessage;
    static inline REL::Relocation<Tab*> _savedTabIdx{RELOCATION_ID(520167, 406697)};
};

#endif  // QUEST_MENU_H
