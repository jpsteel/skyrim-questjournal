#ifndef EVENT_PROCESSOR_H
#define EVENT_PROCESSOR_H

#pragma once
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include "Utility.h"


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

void LogTitleListEntries(const RE::GFxValue& titleList);
void GetInGameDate();

#endif  // EVENT_PROCESSOR_H