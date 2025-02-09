#include <spdlog/sinks/basic_file_sink.h>

#include "Utility.h"
#include "QuestMenu.h"
#include "EventProcessor.h"
#include "Scaleform.h"
#include "Serialization.h"

bool Scaleform::knotworkStatus;

void SKSEMessageHandler(SKSE::MessagingInterface::Message* message) {
    auto eventProcessor = EventProcessor::GetSingleton();
    switch (message->type) {

        case (SKSE::MessagingInterface::kDataLoaded):
            RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(eventProcessor);
            Scaleform::QuestMenu::Register();
            break;

        case (SKSE::MessagingInterface::kInputLoaded):
            RE::BSInputDeviceManager::GetSingleton()->AddEventSink<RE::InputEvent*>(eventProcessor);
            break;

        case SKSE::MessagingInterface::kPostLoadGame:
        case SKSE::MessagingInterface::kPostLoad:
            Scaleform::knotworkStatus = IsPluginLoaded("Knotwork");
            if (Scaleform::knotworkStatus) {
                logger::info("Knotwork.dll plugin detected, additional knotwork will be loaded.");
            } else {
                logger::info("Knotwork.dll plugin not installed.");
            }
            break;
        case SKSE::MessagingInterface::kNewGame:
        case SKSE::MessagingInterface::kSaveGame:
            break;

        default:
            break;
    }
}


extern "C" [[maybe_unused]] __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);

    SetupLog();
    spdlog::set_level(spdlog::level::debug);

    auto* serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID('QSTM');
    serialization->SetSaveCallback(Serialization::SaveCallback);
    serialization->SetLoadCallback(Serialization::LoadCallback); 
    serialization->SetRevertCallback(Serialization::RevertCallback); 

    SKSE::GetMessagingInterface()->RegisterListener(SKSEMessageHandler);
    LoadAllQuestsSupplementalData();
    LoadQuestsCategoriesData();
    JournalMenuEx::InstallHooks();
    logger::info("Quest Journal is in Player's pocket!");

    return true;
}