#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <SKSE/SKSE.h>
#include "Utility.h"

namespace Serialization {
    extern std::unordered_set<std::string> ViewedQuests;

    void RevertCallback(SKSE::SerializationInterface* intfc);
    void SaveCallback(SKSE::SerializationInterface* intfc);
    void LoadCallback(SKSE::SerializationInterface* intfc);
}

#endif  // SERIALIZATION_H