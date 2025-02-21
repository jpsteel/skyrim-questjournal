#include "Utility.h"
#include "Scaleform.h"
#include "SKSE/Interfaces.h"
#include "SKSE/API.h"
#include <Windows.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_map>
#include <bitset>

namespace logger = SKSE::log;
namespace fs = std::filesystem;
const std::string QUEST_LOCATION_DIRECTORY = "DATA/interface/quests";
const std::string CATEGORIES_MAPPING_DIRECTORY = "DATA/interface/quests/categories/mapping.json";
nlohmann::json questLocationData;
std::string mainQuestsIDs;
std::string factionQuestsIDs;

void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
}

RE::FormID StringToFormID(const std::string& formIDStr) {
    std::uint32_t formIDDecimal = std::stoul(formIDStr, nullptr, 10);

    if ((formIDDecimal & 0xFF000000) == 0xFE000000) {
        return static_cast<RE::FormID>(formIDDecimal);
    } else {
        return static_cast<RE::FormID>(formIDDecimal);
    }
}

RE::TESQuest* FindQuestByFormIDString(const std::string& formIDStr) {
    RE::FormID formID = StringToFormID(formIDStr);

    auto dataHandler = RE::TESDataHandler::GetSingleton();
    if (!dataHandler) {
        logger::error("Failed to get TESDataHandler.");
        return nullptr;
    }

    for (auto* quest : dataHandler->GetFormArray<RE::TESQuest>()) {
        if (quest && quest->formID == formID) {
            return quest;
        }
    }

    logger::warn("Quest with formID {} not found.", formIDStr);
    return nullptr;
}

void LoadAllQuestsSupplementalData() {
    try {
        for (const auto& fileEntry : fs::directory_iterator(QUEST_LOCATION_DIRECTORY)) {
            if (fileEntry.is_regular_file() && fileEntry.path().extension() == ".json") {
                logger::info("Loading quest location file: {}", fileEntry.path().string());
                std::ifstream file(fileEntry.path());

                if (!file.is_open()) {
                    logger::warn("Failed to open JSON file: {}", fileEntry.path().string());
                    continue;
                }

                try {
                    nlohmann::json fileData;
                    file >> fileData;

                    logger::info("Parsing quest location data from file: {}", fileEntry.path().string());

                    for (auto& [plugin, quests] : fileData.items()) {
                        logger::info("Processing plugin: {}", plugin);

                        for (auto& [formID, attributes] : quests.items()) {
                            std::string location = attributes.value("location", "unknown");

                            if (attributes.contains("type")) {
                                if (attributes["type"].is_number_integer()) {
                                    int typeValue = attributes["type"].get<int>();
                                    attributes["type"] = std::to_string(typeValue);
                                } else if (!attributes["type"].is_string()) {
                                    attributes["type"] = "unspecified";
                                }
                            }

                            logger::info("  Quest FormID: {} -> Location: {}, Type: {}", formID.c_str(), location,
                                         attributes.value("type", "unspecified"));
                        }

                        questLocationData[plugin].update(quests);
                    }

                    logger::info("Successfully loaded quest location file: {}", fileEntry.path().string());
                } catch (const std::exception& e) {
                    logger::error("Error parsing JSON file {}: {}", fileEntry.path().string(), e.what());
                }
            }
        }
        logger::info("All quest location data successfully loaded.");
    } catch (const std::exception& e) {
        logger::error("Error iterating through directory {}: {}", QUEST_LOCATION_DIRECTORY, e.what());
    }
}

void LoadQuestsCategoriesData() {
    const std::string FILE_PATH = CATEGORIES_MAPPING_DIRECTORY;

    try {
        std::ifstream file(FILE_PATH);
        if (!file.is_open()) {
            logger::warn("Failed to open JSON file: {}", FILE_PATH);
            return;
        }

        try {
            nlohmann::json fileData;
            file >> fileData;

            logger::info("Parsing quest category data from file: {}", FILE_PATH);

            auto concatenateList = [](const nlohmann::json& jsonArray) -> std::string {
                std::string result;
                for (const auto& item : jsonArray) {
                    if (!result.empty()) {
                        result += ";";
                    }
                    result += item.dump();
                }
                return result;
            };

            if (fileData.contains("main") && fileData["main"].is_array()) {
                mainQuestsIDs = concatenateList(fileData["main"]);
                logger::info("Loaded main quests: {}", mainQuestsIDs);
            } else {
                logger::warn("Missing or invalid 'main' quest list.");
            }

            if (fileData.contains("faction") && fileData["faction"].is_array()) {
                factionQuestsIDs = concatenateList(fileData["faction"]);
                logger::info("Loaded faction quests: {}", factionQuestsIDs);
            } else {
                logger::warn("Missing or invalid 'faction' quest list.");
            }

            logger::info("Successfully loaded quest categories mapping.");
        } catch (const std::exception& e) {
            logger::error("Error parsing JSON file {}: {}", FILE_PATH, e.what());
        }
    } catch (const std::exception& e) {
        logger::error("Error opening file {}: {}", FILE_PATH, e.what());
    }
}

bool IsPluginLoaded(const std::string& pluginName) {
    std::string dllName = pluginName + ".dll";

    HMODULE module = GetModuleHandleA(dllName.c_str());
    if (module) {
        return true;
    } else {
        return false;
    }
}

int GetPlayerLevel() { 
    auto player = RE::PlayerCharacter::GetSingleton();
    auto playerLevel = player->GetLevel();
    return playerLevel;
}

float GetPlayerXPProgression() {
    auto player = RE::PlayerCharacter::GetSingleton();
    auto playerXP = player->GetInfoRuntimeData().skills->data->xp;
    auto playerLevelThreshold = player->GetInfoRuntimeData().skills->data->levelThreshold;
    float levelProgression = round((playerXP / playerLevelThreshold) * 140); //LevelProgresssBar movieclip is made of 140 frames and not 100 for some reason...
    return levelProgression;
}

void ListActiveQuestFlags(RE::TESQuest* quest) {
    if (!quest) {
        logger::warn("Quest is null.");
        return;
    }

    uint16_t flags = quest->data.flags.underlying(); 

    logger::info("Flags for quest '{}': {:#x}", quest->GetName(), flags); 

    struct FlagInfo {
        RE::QuestFlag flag;
        const char* name;
    };


    const std::vector<FlagInfo> knownFlags = {
        {RE::QuestFlag::kStopStart, "kStopStart"},
        {RE::QuestFlag::kNone, "kNone"},
        {RE::QuestFlag::kEnabled, "kEnabled"},
        {RE::QuestFlag::kCompleted, "kCompleted"},
        {RE::QuestFlag::kAddIdleToHello, "kAddIdleToHello"},
        {RE::QuestFlag::kAllowRepeatStages, "kAllowRepeatStages"},
        {RE::QuestFlag::kStartsEnabled, "kStartsEnabled"},
        {RE::QuestFlag::kDisplayedInHUD, "kDisplayedInHUD"},
        {RE::QuestFlag::kWarnOnAliasFillFailure, "kWarnOnAliasFillFailure"},
        {RE::QuestFlag::kActive, "kActive"},
        {RE::QuestFlag::kRepeatsConditions, "kRepeatsConditions"},
        {RE::QuestFlag::kKeepInstance, "kKeepInstance"},
        {RE::QuestFlag::kWantDormant, "kWantDormant"},
    };

    for (const auto& flagInfo : knownFlags) {
        if (flags & static_cast<uint16_t>(flagInfo.flag)) {
            logger::info("  Active Flag: {}", flagInfo.name);
        }
    }

    std::bitset<16> bits(flags);
    logger::info("  Remaining Unknown Flags: {}", bits.to_string());
}

std::string ConcatenateViewedQuests(const std::unordered_set<std::string>& quests) {
    if (quests.empty()) {
        return "";
    }

    std::ostringstream oss;
    auto it = quests.begin();
    oss << *it;

    for (++it; it != quests.end(); ++it) {
        oss << ";" << *it;
    }

    return oss.str();
}

void SetINIValue(const char* settingName, float value) {
    auto iniSetting = RE::INISettingCollection::GetSingleton()->GetSetting(settingName);
    if (iniSetting) {
        iniSetting->data.f = value;
        RE::INISettingCollection::GetSingleton()->WriteSetting(iniSetting);
        logger::info("{} INI value set to {}", settingName, value);
    }
}

void GetINIValue(const char* settingName) {
    auto iniSetting = RE::INISettingCollection::GetSingleton()->GetSetting(settingName);
    if (iniSetting) {
        logger::info("INI setting value for {} is {}", settingName, iniSetting->GetFloat());
    }
}

void LogActiveInputHandlers() {
    auto menuControls = RE::MenuControls::GetSingleton();
    if (!menuControls) {
        logger::error("MenuControls not found!");
        return;
    }

    logger::info("Checking active InputHandlers...");

    for (auto& handler : menuControls->handlers) {
        if (handler) {
            logger::info("Active InputHandler: {}", typeid(*handler).name());
        }
    }
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