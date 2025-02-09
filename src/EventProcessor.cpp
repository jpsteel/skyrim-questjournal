#include "EventProcessor.h"
#include "Utility.h"
#include "QuestMenu.h"
#include "Scaleform.h"
#include "Serialization.h"

RE::GFxValue entryList;
RE::GFxValue currentTab;
std::string lastQuestFormID; 
std::string dt;
bool hideCompletedState;

RE::BSEventNotifyControl EventProcessor::ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                                      RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
    if (!event) {
        return RE::BSEventNotifyControl::kContinue;
    }
    if (event->opening) {
        auto ui = RE::UI::GetSingleton();
        if (event->menuName == RE::JournalMenu::MENU_NAME) { 
            if (auto menu = ui->GetMenu(RE::JournalMenu::MENU_NAME); menu) {
                menu->uiMovie->GetVariable(&currentTab, "_root.QuestJournalFader.Menu_mc.iCurrentTab");
                if (auto journal = ui->GetMenu(RE::JournalMenu::MENU_NAME); journal) {
                    RE::GFxValue titleList;
                    entryList = RE::GFxValue();
                    journal->uiMovie->GetVariable(&titleList, 
                                                "_root.QuestJournalFader.Menu_mc.QuestsFader.Page_mc.TitleList");
                    if (titleList.HasMember("entryList")) {
                        logger::info("Retrieving quests data");
                        titleList.GetMember("entryList", &entryList);
                    }
                } else {
                    return RE::BSEventNotifyControl::kContinue;
                }
            }
        } else if (event->menuName == Scaleform::QuestMenu::MENU_NAME) {
            if (auto menu = ui->GetMenu(Scaleform::QuestMenu::MENU_NAME); menu) {
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
        } else if (event->menuName == RE::MapMenu::MENU_NAME) {
            if (auto menu = ui->GetMenu(RE::MapMenu::MENU_NAME); menu) {
                logger::info("Map menu open!");
            }
        }
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
                }
            }
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}

void LogTitleListEntries(const RE::GFxValue& entryList) {
    if (entryList.IsArray()) {
        for (std::uint32_t i = 0; i < entryList.GetArraySize(); ++i) {
            logger::info("------ Start of Entry ------");
            RE::GFxValue entry;
            entryList.GetElement(i, &entry);

            RE::GFxValue title, description, formID, instance, completed, active, objectives, type, timeIndex, divider;

            if (entry.HasMember("text")) {
                entry.GetMember("text", &title); 
                if (title.IsString()) { 
                    logger::info("Quest Title: {}", title.GetString());
                }
            }

            if (entry.HasMember("description")) {
                entry.GetMember("description", &description); 
                if (description.IsString()) { 
                    logger::info("Description: {}", description.GetString());
                }
            }

            if (entry.HasMember("formID")) {
                entry.GetMember("formID", &formID);
                if (formID.IsNumber()) {
                    logger::info("Form ID: {}", formID.GetNumber());
                }
            }

            if (entry.HasMember("instance")) {
                entry.GetMember("instance", &instance);
                if (instance.IsNumber()) {
                    logger::info("Instance: {}", instance.GetNumber());
                }
            }

            if (entry.HasMember("completed")) {
                entry.GetMember("completed", &completed);
                if (completed.IsNumber()) {
                    logger::info("Completed: {}", completed.GetNumber());
                }
            }

            if (entry.HasMember("active")) {
                entry.GetMember("active", &active);
                if (active.IsBool()) {
                    logger::info("Active: {}", active.GetBool());
                }
            }

            if (entry.HasMember("objectives")) {
                entry.GetMember("objectives", &objectives);
                if (objectives.IsArray()) {
                    logger::info("-------------Objectives: {}", objectives.GetArraySize());

                    for (std::uint32_t i = 0; i < objectives.GetArraySize(); ++i) {
                        RE::GFxValue objective;
                        objectives.GetElement(i, &objective);
                        if (objective.HasMember("text")) {
                            RE::GFxValue textValue;
                            objective.GetMember("text", &textValue);
                            logger::info("[Objective {}] - {}", i, textValue.GetString());
                        }

                        if (objective.HasMember("formID")) {
                            RE::GFxValue formIDValue;
                            objective.GetMember("formID", &formIDValue);
                            logger::info("formID: {}", formIDValue.GetNumber());
                        }

                        if (objective.HasMember("instance")) {
                            RE::GFxValue instanceValue;
                            objective.GetMember("instance", &instanceValue);
                            logger::info("instance: {}", instanceValue.GetNumber()); 
                        }

                        if (objective.HasMember("active")) {
                            RE::GFxValue activeValue;
                            objective.GetMember("active", &activeValue);
                            logger::info("active: {}", activeValue.GetBool()); 
                        }

                        if (objective.HasMember("completed")) {
                            RE::GFxValue completedValue; 
                            objective.GetMember("completed", &completedValue); 
                            logger::info("completed: {}", completedValue.GetNumber()); 
                        }

                        if (objective.HasMember("failed")) {
                            RE::GFxValue failedValue;
                            objective.GetMember("failed", &failedValue); 
                            logger::info("failed: {}", failedValue.GetNumber()); 
                        }
                    }
                    logger::info("---------------");
                }
            }

            if (entry.HasMember("type")) {
                entry.GetMember("type", &type);
                logger::info("Quest Type: {}", type.GetNumber());
            }

            if (entry.HasMember("timeIndex")) {
                entry.GetMember("timeIndex", &timeIndex);
                if (timeIndex.IsNumber()) {
                    logger::info("Time Index: {}", timeIndex.GetNumber());
                }
            }

            if (entry.HasMember("divider")) {
                entry.GetMember("divider", &divider);
                if (divider.IsBool()) {
                    logger::info("Divider: {}", divider.GetBool());
                }
            }

            logger::info("------ End of Entry ------");
        }
    } else {
        logger::warn("TitleList entryList is not an array.");
    }
}

void GetInGameDate() {
    auto calendar = RE::Calendar::GetSingleton();
    char datetime[504];
    calendar->GetTimeDateString(datetime, 0x200u, 1);
    dt = datetime;
}