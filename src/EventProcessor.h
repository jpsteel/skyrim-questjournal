#ifndef EVENT_PROCESSOR_H
#define EVENT_PROCESSOR_H

#include <unordered_map>

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

class EventProcessor : public RE::BSTEventSink<RE::InputEvent*>,
                       public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
public:
    static EventProcessor* GetSingleton() {
        static EventProcessor instance;
        return &instance;
    }

    RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* eventPtr,
                                          RE::BSTEventSource<RE::InputEvent*>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;
};

void GetInGameDate();

#endif  // EVENT_PROCESSOR_H
