#include "Serialization.h"

namespace Serialization {
    std::unordered_set<std::string> ViewedQuests;

    void RevertCallback(SKSE::SerializationInterface* intfc) {
        logger::info("RevertCallback triggered.");
        ViewedQuests.clear();
    }

    void SaveCallback(SKSE::SerializationInterface* intfc) {
        logger::info("SaveCallback triggered.");

        if (!intfc->OpenRecord('QSTM', DATA_VERSION)) {
            logger::error("Failed to open record for viewed quests.");
            return;
        }

        std::uint32_t size = static_cast<std::uint32_t>(ViewedQuests.size());
        logger::info("Serializing ViewedQuests: {} entries", size);

        if (!intfc->WriteRecordData(&size, sizeof(size))) {
            logger::error("Failed to write size of viewed quests list.");
            return;
        }

        for (const auto& title : ViewedQuests) {
            std::uint32_t length = static_cast<std::uint32_t>(title.size());

            if (!intfc->WriteRecordData(&length, sizeof(length))) {
                logger::error("Failed to write string length for quest: {}", title);
                return;
            }

            if (!intfc->WriteRecordData(title.data(), length)) {
                logger::error("Failed to write viewed quest: {}", title);
                return;
            }
        }
    }

    void LoadCallback(SKSE::SerializationInterface* intfc) {
        logger::info("LoadCallback triggered.");
        std::uint32_t type, version, length;

        while (intfc->GetNextRecordInfo(type, version, length)) {
            if (type == 'QSTM') {
                logger::info("Loading viewed quests (version {})", version);
                if (version > DATA_VERSION) {
                    logger::error("Unsupported serialization version {}", version);
                    continue;
                }

                std::uint32_t size;
                if (!intfc->ReadRecordData(&size, sizeof(size))) {
                    logger::error("Failed to read size of ViewedQuests list.");
                    return;
                }

                ViewedQuests.clear();
                for (std::uint32_t i = 0; i < size; ++i) {
                    std::uint32_t strLength;
                    if (!intfc->ReadRecordData(&strLength, sizeof(strLength))) {
                        logger::error("Failed to read string length for viewed quest.");
                        return;
                    }

                    std::string title(strLength, '\0');

                    if (!intfc->ReadRecordData(title.data(), strLength)) {
                        logger::error("Failed to read viewed quest name.");
                        return;
                    }

                    ViewedQuests.insert(title);
                }

                logger::info("Successfully loaded {} viewed quests.", size);
            } else {
                logger::warn("Unknown record type: 0x{:X}", type);
            }
        }
    }
}
