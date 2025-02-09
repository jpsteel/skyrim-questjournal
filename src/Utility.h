#ifndef UTILITY_H
#define UTILITY_H

#include <nlohmann/json.hpp>
#include <unordered_set>

namespace logger = SKSE::log;
extern nlohmann::json questLocationData;
extern std::string mainQuestsIDs;
extern std::string factionQuestsIDs;

void SetupLog();
void LoadAllQuestsSupplementalData();
void LoadQuestsCategoriesData();
bool IsPluginLoaded(const std::string& pluginName);
int GetPlayerLevel();
float GetPlayerXPProgression();
RE::FormID StringToFormID(const std::string& formIDStr);
RE::TESQuest* FindQuestByFormIDString(const std::string& formIDStr);
void ListActiveQuestFlags(RE::TESQuest* quest);
std::string ConcatenateViewedQuests(const std::unordered_set<std::string>& quests);

#endif  // UTILITY_H