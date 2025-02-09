#include "RE/T/TESQuest.h"
#include "RE/T/TESWorldSpace.h"
#include "RE/B/BGSLocalizedStringDL.h"
#include "RE/L/Location.h"

#include "QuestMenu.h"
#include "Scaleform.h"
#include "Utility.h"

namespace Scaleform {
    QuestMenu::QuestMenu() {
        auto scaleformManager = RE::BSScaleformManager::GetSingleton();
        scaleformManager->LoadMovieEx(this, MENU_PATH, [this](RE::GFxMovieDef* a_def) {
            using StateType = RE::GFxState::StateType;

            fxDelegate.reset(new RE::FxDelegate());
            fxDelegate->RegisterHandler(this);
            a_def->SetState(StateType::kExternalInterface, fxDelegate.get());
            fxDelegate->Release();

            auto logger = new Logger<QuestMenu>(); 
            a_def->SetState(StateType::kLog, logger);
            logger->Release();
        });

        inputContext = Context::kMenuMode;
        depthPriority = 3;
        menuFlags.set(RE::UI_MENU_FLAGS::kPausesGame, RE::UI_MENU_FLAGS::kDisablePauseMenu,
                      RE::UI_MENU_FLAGS::kUsesBlurredBackground, RE::UI_MENU_FLAGS::kModal,
                      RE::UI_MENU_FLAGS::kUsesMenuContext, RE::UI_MENU_FLAGS::kTopmostRenderedMenu,
                      RE::UI_MENU_FLAGS::kUsesMovementToDirection);

        if (!RE::BSInputDeviceManager::GetSingleton()->IsGamepadEnabled()) {
            menuFlags |= RE::UI_MENU_FLAGS::kUsesCursor;
        }
    }

    void QuestMenu::Register() {
        auto ui = RE::UI::GetSingleton();
        if (ui) {
            ui->Register(QuestMenu::MENU_NAME, Creator);
            logger::debug("Registered {}", QuestMenu::MENU_NAME);
        }
    }

    void QuestMenu::Show() {
        auto uiMessageQueue = RE::UIMessageQueue::GetSingleton();
        if (uiMessageQueue) {
            uiMessageQueue->AddMessage(QuestMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
            //RE::PlaySound("UIJournalOpen"); Not needed since Journal Menu opens right before with sound
            RE::UIBlurManager::GetSingleton()->IncrementBlurCount();
        }
    }

    void QuestMenu::Hide() {
        auto uiMessageQueue = RE::UIMessageQueue::GetSingleton();
        if (uiMessageQueue) {
            uiMessageQueue->AddMessage(QuestMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
            RE::PlaySound("UIJournalClose");
            RE::UIBlurManager::GetSingleton()->DecrementBlurCount();
        }
    }

    void QuestMenu::Accept(RE::FxDelegateHandler::CallbackProcessor* a_cbReg) {
        a_cbReg->Process("PlaySound", PlaySound);
        a_cbReg->Process("ToggleQuestActive", ToggleQuestActive);
        a_cbReg->Process("RequestQuestLocation", RequestQuestLocation);
        a_cbReg->Process("ShowQuestOnMap", ShowQuestOnMap);
    }

    void QuestMenu::PlaySound(const RE::FxDelegateArgs& a_params) {
        assert(a_params.GetArgCount() == 1);
        assert(a_params[0].IsString());

        RE::PlaySound(a_params[0].GetString());
    }
    
    void QuestMenu::ToggleQuestActive(const RE::FxDelegateArgs& a_params) {
        assert(a_params.GetArgCount() == 1);
        assert(a_params[0].IsString());

        RE::TESQuest* quest = FindQuestByFormIDString(a_params[0].GetString());
        if (quest == nullptr) {
            logger::warn("Couldn't find quest for formID {}", a_params[0].GetString());
            return;
        }
        logger::info("Toggling active status for quest: {}", quest->GetName());
        if (quest->IsActive()) {
            quest->data.flags &=
                static_cast<RE::QuestFlag>(~static_cast<std::underlying_type_t<RE::QuestFlag>>(RE::QuestFlag::kActive));
        } else {
            quest->data.flags |= RE::QuestFlag::kActive;
        }
    }

    void QuestMenu::RequestQuestLocation(const RE::FxDelegateArgs& a_params) {
        assert(a_params.GetArgCount() == 0);

        auto ui = RE::UI::GetSingleton();
        if (auto menu = ui->GetMenu(Scaleform::QuestMenu::MENU_NAME); menu) {
            RE::GFxValue currentQuest;
            menu->uiMovie->GetVariable(&currentQuest, "_root.QuestJournal_mc.currentQuest");
            RE::GFxValue formID;
            if (currentQuest.HasMember("formID")) {
                currentQuest.GetMember("formID", &formID);
                RE::TESQuest* quest = FindQuestByFormIDString(formID.GetString());

                if (quest) {
                    auto pluginName = quest->GetFile(0)->GetFilename();
                    std::string editorID = quest->GetFormEditorID();

                    if (questLocationData.contains(pluginName) && questLocationData[pluginName].contains(editorID)) {
                        const auto& questData = questLocationData[pluginName][editorID];
                        if (questData.contains("location")) {
                            currentQuest.SetMember("location", questData["location"].get<std::string>().c_str());
                        } else {
                            logger::info("Setting location: none");
                            currentQuest.SetMember("location", "none");
                        }

                        if (questData.contains("type")) {
                            currentQuest.SetMember("type", questData["type"].get<std::string>().c_str());
                        }
                    } else {
                        logger::warn("No location found for quest {} ({})", quest->GetName(), editorID);
                        currentQuest.SetMember("location", "none");
                    }
                }
            }
        }
    }

    void QuestMenu::ShowQuestOnMap(const RE::FxDelegateArgs& a_params) {
        assert(a_params.GetArgCount() == 1); 
        assert(a_params[0].IsString());

        QuestMenu::Hide();
        auto uiMessageQueue = RE::UIMessageQueue::GetSingleton();
        if (uiMessageQueue) {
            uiMessageQueue->AddMessage(RE::MapMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
        }
    }

    std::vector<QuestMenu::QuestData> QuestMenu::GetRunningQuests() {
        std::vector<QuestData> questList;

        auto dataHandler = RE::TESDataHandler::GetSingleton();
        if (dataHandler) {
            for (auto& quest : dataHandler->GetFormArray<RE::TESQuest>()) {
                if (quest && quest->IsCompleted() && HasObjective(quest)) {
                    QuestData qd; 
                    qd.formID = quest->GetFormID();
                    qd.name = quest->GetName();
                    logger::info("-- QUEST -- {}", qd.name);
                    qd.category = "comp";
                    qd.type = GetQuestType(quest);
                    qd.objectives = GetQuestObjectives(quest);
                    questList.push_back(qd);
                    //GetQuestStageLogEntry(quest); 
                }
                else if (quest && quest->IsEnabled() && HasObjectiveDisplayed(quest)) {
                    QuestData qd;
                    qd.formID = quest->GetFormID();
                    qd.name = quest->GetName();
                    logger::info("-- QUEST -- {}", qd.name);
                    qd.category = GetQuestCategory(quest);
                    qd.type = GetQuestType(quest);
                    qd.objectives = GetQuestObjectives(quest);
                    questList.push_back(qd);
                    //GetQuestStageLogEntry(quest); 
                }
            }
        }

        return questList;
    }

    std::string QuestMenu::GetQuestType(RE::TESQuest* a_quest) {
        std::string typeValue = "none";
        auto type = a_quest->GetType();
        switch (type) {
            case RE::QUEST_DATA::Type::kNone:
                break;
            case RE::QUEST_DATA::Type::kMainQuest:
                typeValue = "Main";
                break;
            case RE::QUEST_DATA::Type::kDLC01_Vampire:
                typeValue = "DLC01Vampires";
                break;
            case RE::QUEST_DATA::Type::kDLC02_Dragonborn:
                typeValue = "DLC02";
                break;
            case RE::QUEST_DATA::Type::kMagesGuild:
                typeValue = "MagesGuild";
                break;
            case RE::QUEST_DATA::Type::kThievesGuild:
                typeValue = "ThievesGuild";
                break;
            case RE::QUEST_DATA::Type::kDarkBrotherhood:
                typeValue = "DarkBrotherhood";
                break;
            case RE::QUEST_DATA::Type::kCompanionsQuest:
                typeValue = "Companion";
                break;
            case RE::QUEST_DATA::Type::kCivilWar:
                typeValue = "CivilWarImperial";
                break;
            case RE::QUEST_DATA::Type::kDaedric:
                typeValue = "Daedric";
                break;
            case RE::QUEST_DATA::Type::kSideQuest:
                typeValue = "Misc";
                break;
            case RE::QUEST_DATA::Type::kMiscellaneous:
                typeValue = "Favor";
                break;
            default:
                break;
        }
        return typeValue;
    }

    std::string QuestMenu::GetQuestCategory(RE::TESQuest* a_quest) { 
        std::string category = "none";
        auto type = a_quest->GetType();
        switch (type) {
            case RE::QUEST_DATA::Type::kNone:
                break;
            case RE::QUEST_DATA::Type::kMainQuest:
            case RE::QUEST_DATA::Type::kDLC01_Vampire:
            case RE::QUEST_DATA::Type::kDLC02_Dragonborn:
                category = "main";
                break;
            case RE::QUEST_DATA::Type::kMagesGuild:
            case RE::QUEST_DATA::Type::kThievesGuild:
            case RE::QUEST_DATA::Type::kDarkBrotherhood:
            case RE::QUEST_DATA::Type::kCompanionsQuest:
            case RE::QUEST_DATA::Type::kCivilWar:
                category = "fact";
                break;
            case RE::QUEST_DATA::Type::kDaedric:
            case RE::QUEST_DATA::Type::kSideQuest:
                category = "side";
                break;
            case RE::QUEST_DATA::Type::kMiscellaneous:
                category = "misc";
                break;
            default:
                break;
        }
        return category;
    }

    std::string QuestMenu::GetObjectiveState(RE::BGSQuestObjective* a_objective) { 
        std::string stateValue = "0";
        auto state = a_objective->state.get();
        switch (state) { 
            case RE::QUEST_OBJECTIVE_STATE::kDormant:
                break;
            case RE::QUEST_OBJECTIVE_STATE::kDisplayed:
                stateValue = "1";
                break;
            case RE::QUEST_OBJECTIVE_STATE::kCompleted:
                stateValue = "2";
                break;
            case RE::QUEST_OBJECTIVE_STATE::kCompletedDisplayed:
                stateValue = "3";
                break;
            case RE::QUEST_OBJECTIVE_STATE::kFailed:
                stateValue = "4";
                break;
            case RE::QUEST_OBJECTIVE_STATE::kFailedDisplayed:
                stateValue = "5";
                break;
            default:
                break;
        }
        return stateValue;
    }
    /* std::string QuestMenu::GetQuestStageLogEntry(RE::TESQuest* quest) {
        logger::info("Gettin quest stage log entry for {}", quest->GetName());
        RE::TESQuestStage* currentStage = nullptr;
        for (auto& stage : *quest->waitingStages) {
            if (stage->data.index == quest->GetCurrentStageID()) {
                logger::info("Current stage ID: {}", quest->GetCurrentStageID());
                currentStage = stage;
                break;
            }
        }
        if (currentStage == nullptr) {
            for (auto& stage : *quest->executedStages) {
                if (stage.data.index == quest->GetCurrentStageID()) {
                    logger::info("Current stage ID: {}", quest->GetCurrentStageID());
                    currentStage = &stage;
                    break;
                }
            }
        }
        if (currentStage == nullptr) {
            logger::info("Current stage not found");
            return "Current stage not found";
        }

        auto* player = RE::PlayerCharacter::GetSingleton();
        RE::PlayerCharacter::PLAYER_RUNTIME_DATA& runtimeData = player->GetPlayerRuntimeData();
        RE::BSString logEntry;
        logger::info("Instance data size: {}", quest->instanceData.size());
        return logEntry.c_str();
    }*/

    std::vector<Scaleform::QuestMenu::QuestObjective> QuestMenu::GetQuestObjectives(RE::TESQuest* a_quest) {
        std::vector<Scaleform::QuestMenu::QuestObjective> questObjectives;
        QuestObjective qo;
        for (auto& objective : a_quest->objectives) {
            std::string objText = objective->displayText.c_str();
            qo.text = objText;
            qo.state = GetObjectiveState(objective);
            logger::info("[Objective] {}: {}", qo.text, qo.state);
            questObjectives.push_back(qo);
        }
        return questObjectives; 
    }

    bool QuestMenu::HasObjective(RE::TESQuest* a_quest) {
        for (auto& objective : a_quest->objectives) {
            if (objective) {
                return true;
            }
        }
        return false;
    }

    bool QuestMenu::HasObjectiveDisplayed(RE::TESQuest* a_quest) { 
        for (auto& objective : a_quest->objectives) {
            if (objective && objective->state.all(RE::QUEST_OBJECTIVE_STATE::kDisplayed)) {
                return true;
            }
        }
        return false;
    }
}
