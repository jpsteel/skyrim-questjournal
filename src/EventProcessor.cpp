#include "EventProcessor.h"
#include "Utility.h"
#include "QuestMenu.h"
#include "Scaleform.h"
#include "Serialization.h"

RE::GFxValue entryList;
std::string lastQuestFormID; 
std::string dt;
bool hideCompletedState;
bool Scaleform::showOnMapStatus;
bool journalHotkey;
uint32_t Scaleform::questTargetID;

RE::BSEventNotifyControl EventProcessor::ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                                      RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
    if (!event) {
        return RE::BSEventNotifyControl::kContinue;
    }
    if (event->opening) {
        auto ui = RE::UI::GetSingleton();
        if (event->menuName == RE::JournalMenu::MENU_NAME) {
            if (auto journal = ui->GetMenu(RE::JournalMenu::MENU_NAME); journal) {
                if (Scaleform::showOnMapStatus) {
                    std::array<RE::GFxValue, 1> targetID;
                    targetID[0] = Scaleform::questTargetID;
                    logger::info("Showing questTargetID {} on Map.", Scaleform::questTargetID);
                    journal->uiMovie->Invoke("_root.QuestJournalFader.Menu_mc.QuestsFader.Page_mc.QJO_ShowOnMap",
                                             nullptr, targetID.data(), targetID.size());
                    Scaleform::showOnMapStatus = false;
                } else {
                    RE::GFxValue titleList;
                    entryList = RE::GFxValue();
                    journal->uiMovie->Invoke("_root.QuestJournalFader.Menu_mc.QuestsFader.Page_mc.QJO_LoadQuests",
                                                nullptr, nullptr, 0);
                    journal->uiMovie->GetVariable(&titleList,
                                                    "_root.QuestJournalFader.Menu_mc.QuestsFader.Page_mc.TitleList");
                    if (titleList.HasMember("entryList")) {
                        logger::info("Retrieving quests data");
                        titleList.GetMember("entryList", &entryList);
                    }

                    if (ui->IsMenuOpen(RE::MapMenu::MENU_NAME)) {
                        journal->uiMovie->Invoke("_root.QuestJournalFader.Menu_mc.QuestsFader.Page_mc.QJO_EndPage",
                                                 nullptr, nullptr, 0);
                    }
                    if (journalHotkey && !RE::BSInputDeviceManager::GetSingleton()->IsGamepadEnabled()) {
                        journal->uiMovie->Invoke("_root.QuestJournalFader.Menu_mc.QuestsFader.Page_mc.QJO_EndPage",
                                                 nullptr, nullptr, 0);
                        journalHotkey = false;
                    } else if (journalHotkey && RE::BSInputDeviceManager::GetSingleton()->IsGamepadEnabled()) {
                        std::array<RE::GFxValue, 2> switchArgs;
                        switchArgs[0] = 2;
                        switchArgs[1] = true;
                        journal->uiMovie->Invoke("_root.QuestJournalFader.Menu_mc.SwitchPageToFront", nullptr,
                                                 switchArgs.data(), switchArgs.size());
                        journalHotkey = false;
                    }
                }
            } else {
                return RE::BSEventNotifyControl::kContinue;
            }
        } else if (event->menuName == Scaleform::QuestMenu::MENU_NAME) {
            if (auto menu = ui->GetMenu(Scaleform::QuestMenu::MENU_NAME); menu) {
                if (ui->IsMenuOpen(RE::CursorMenu::MENU_NAME)) {
                    auto uiMessageQueue = RE::UIMessageQueue::GetSingleton();
                    if (uiMessageQueue) {
                        uiMessageQueue->AddMessage(RE::CursorMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
                    }
                }
                logger::info("Sending quests list");
                std::array<RE::GFxValue, 4> configArgs;
                std::array<RE::GFxValue, 3> headerInfo;
                std::array<RE::GFxValue, 1> gamepad;
                std::array<RE::GFxValue, 1> lastDisplayed;
                std::array<RE::GFxValue, 1> viewedQuestsString;
                std::array<RE::GFxValue, 2> playerFactions;
                std::array<RE::GFxValue, 1> hideCompleted;
                configArgs[0] = entryList;
                configArgs[1] = hideCompletedState;
                configArgs[2] = mainQuestsIDs;
                configArgs[3] = factionQuestsIDs;
                headerInfo[0] = GetPlayerLevel();
                headerInfo[1] = GetPlayerXPProgression();
                GetInGameDate();
                headerInfo[2] = dt;
                gamepad[0] = RE::BSInputDeviceManager::GetSingleton()->IsGamepadEnabled();
                lastDisplayed[0] = lastQuestFormID;
                auto player = RE::PlayerCharacter::GetSingleton();
                RE::TESForm* stormcloakForm = RE::TESForm::LookupByEditorID("CWSonsFaction");
                RE::TESFaction* stormcloakFaction = skyrim_cast<RE::TESFaction*>(stormcloakForm);
                RE::TESForm* volkiharForm = RE::TESForm::LookupByEditorID("DLC1VampireFaction");
                RE::TESFaction* volkiharFaction = skyrim_cast<RE::TESFaction*>(volkiharForm);
                playerFactions[0] = player->IsInFaction(stormcloakFaction);
                playerFactions[1] = player->IsInFaction(volkiharFaction);
                logger::info("Is player Stormcloak? {} Is player Volkihar? {}", player->IsInFaction(stormcloakFaction),
                             player->IsInFaction(volkiharFaction));
                viewedQuestsString[0] = ConcatenateViewedQuests(Serialization::ViewedQuests);
                menu->uiMovie->Invoke("_root.QuestJournal_mc.SetLastDisplayedQuest", nullptr, lastDisplayed.data(),
                                      lastDisplayed.size());
                menu->uiMovie->Invoke("_root.QuestJournal_mc.SetViewedQuests", nullptr, viewedQuestsString.data(), 
                                      viewedQuestsString.size());
                menu->uiMovie->Invoke("_root.QuestJournal_mc.SetPlayerFactions", nullptr, playerFactions.data(), 
                                      playerFactions.size()); 
                menu->uiMovie->Invoke("_root.QuestJournal_mc.onQuestsDataComplete", nullptr, configArgs.data(),
                                      configArgs.size());
                logger::info("Quests data received!");
                menu->uiMovie->Invoke("_root.QuestJournal_mc.SetHeaderInfo", nullptr, headerInfo.data(),
                                      headerInfo.size());
                menu->uiMovie->Invoke("_root.QuestJournal_mc.SetGamepad", nullptr, gamepad.data(), gamepad.size());
            }

            if (ui->IsMenuOpen(RE::MapMenu::MENU_NAME)) {
                SetINIValue("fMaxMarkerSelectionDist:MapMenu", 0.0);
            }
        }
    }
    else if (event->menuName == Scaleform::QuestMenu::MENU_NAME) {
        SetINIValue("fMaxMarkerSelectionDist:MapMenu", 0.003);
    }

    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl EventProcessor::ProcessEvent(RE::InputEvent* const* eventPtr,
                                                      RE::BSTEventSource<RE::InputEvent*>*) {
    if (!eventPtr || !*eventPtr || !RE::Main::GetSingleton()->gameActive) {
        return RE::BSEventNotifyControl::kContinue;
    }

    auto* event = *eventPtr;

    if (event->eventType == RE::INPUT_EVENT_TYPE::kButton) {
        auto* buttonEvent = event->AsButtonEvent();
        auto dxScanCode = buttonEvent->GetIDCode();
        auto userEvent = RE::UserEvents::GetSingleton();
        if (buttonEvent->IsDown()) {
            auto ui = RE::UI::GetSingleton();
            if (buttonEvent->QUserEvent() == userEvent->cancel || buttonEvent->QUserEvent() == userEvent->tweenMenu ||
                buttonEvent->QUserEvent() == userEvent->journal) {
                if (ui->IsMenuOpen(Scaleform::QuestMenu::MENU_NAME)) {
                    if (auto menu = ui->GetMenu(Scaleform::QuestMenu::MENU_NAME); menu) {
                        logger::info("Saving last quest displayed.");
                        RE::GFxValue lastQuest1;
                        RE::GFxValue lastQuest2;
                        menu->uiMovie->GetVariable(&lastQuest1, "_root.QuestJournal_mc.currentQuest");
                        if (lastQuest1.HasMember("formID")) {
                            lastQuest1.GetMember("formID", &lastQuest2);
                            lastQuestFormID = lastQuest2.GetString();
                            logger::info("Last displayed quest: {}", lastQuestFormID);
                        }
                        logger::info("Saving viewed quests.");
                        RE::GFxValue viewedQuestsList;
                        menu->uiMovie->GetVariable(&viewedQuestsList, "_root.QuestJournal_mc.aViewedQuests");
                        for (std::uint32_t i = 0; i < viewedQuestsList.GetArraySize(); ++i) {
                            RE::GFxValue viewedEntry;
                            viewedQuestsList.GetElement(i, &viewedEntry);
                            Serialization::ViewedQuests.insert(viewedEntry.GetString());
                        }
                        logger::info("Saving Hide Completed state.");
                        RE::GFxValue hideCompletedQuests;
                        menu->uiMovie->GetVariable(&hideCompletedQuests, "_root.QuestJournal_mc.bHideCompleted");
                        hideCompletedState = hideCompletedQuests.GetBool();
                    } else {
                        logger::warn("Couldn't get last displayed quest.");
                    }
                    Scaleform::QuestMenu::Hide();
                    return RE::BSEventNotifyControl::kContinue;
                } else if (buttonEvent->QUserEvent() == userEvent->journal) {
                    journalHotkey = true;
                }
            //gamepad Y returns the dx code 32768 for some reason
            } else if (dxScanCode == 50 || dxScanCode == 279 || dxScanCode == 32768) {
                if (ui->IsMenuOpen(Scaleform::QuestMenu::MENU_NAME)) {
                    if (auto menu = ui->GetMenu(Scaleform::QuestMenu::MENU_NAME); menu) {
                        logger::info("Saving last quest displayed.");
                        RE::GFxValue lastQuest1;
                        RE::GFxValue lastQuest2;
                        menu->uiMovie->GetVariable(&lastQuest1, "_root.QuestJournal_mc.currentQuest");
                        if (lastQuest1.HasMember("formID")) {
                            lastQuest1.GetMember("formID", &lastQuest2);
                            lastQuestFormID = lastQuest2.GetString();
                            logger::info("Last displayed quest: {}", lastQuestFormID);
                        }
                        logger::info("Saving viewed quests.");
                        RE::GFxValue viewedQuestsList;
                        menu->uiMovie->GetVariable(&viewedQuestsList, "_root.QuestJournal_mc.aViewedQuests");
                        for (std::uint32_t i = 0; i < viewedQuestsList.GetArraySize(); ++i) {
                            RE::GFxValue viewedEntry;
                            viewedQuestsList.GetElement(i, &viewedEntry);
                            Serialization::ViewedQuests.insert(viewedEntry.GetString());
                        }
                        logger::info("Saving Hide Completed state.");
                        RE::GFxValue hideCompletedQuests;
                        menu->uiMovie->GetVariable(&hideCompletedQuests, "_root.QuestJournal_mc.bHideCompleted");
                        hideCompletedState = hideCompletedQuests.GetBool();
                    } else {
                        logger::warn("Couldn't get last displayed quest.");
                    }
                }
            }
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}

void GetInGameDate() {
    auto calendar = RE::Calendar::GetSingleton();
    char datetime[504];
    calendar->GetTimeDateString(datetime, 0x200u, 1);
    dt = datetime;
}