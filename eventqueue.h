#pragma once

#include <functional>
#include <unordered_set>

#include "eventhandler.h"

class EventQueue
{
public:
    enum class Token : uint32_t { Invalid = 0 };
    using EventTypeIds = std::unordered_set<EventTypeId>;
    using Handlers = std::vector<std::unique_ptr<EventHandlerConcept>>;
    using Events = std::vector<std::shared_ptr<EventConcept>>;
    using HandlerGroup = std::tuple<EventTypeIds, Handlers, Events>;
    using HandlerGroups = std::unordered_map<Token, HandlerGroup>;

    [[nodiscard]] static Token GetToken()
    {
        return Increament(ms_TokenSequence);
    }

    template<typename TEvent> requires IsEventModel<TEvent>
    void RegisterHandler(const Token token, EventHandlerModel<TEvent>::EventHandler&& handler)
    {
        assert(token > Token::Invalid);
        HandlerGroup& group{m_Groups[token]};

        EventTypeIds& ids{std::get<EventTypeIds>(group)};
        assert(ids.contains(TEvent::GetStaticId()) == false);
        ids.insert(TEvent::GetStaticId());

        Handlers& handlers{std::get<Handlers>(group)};
        handlers.push_back(std::make_unique<EventHandlerModel<TEvent>>(std::move(handler)));
    }

    template<typename TEvent> requires IsEventModel<TEvent>
    void UnregisterHandler(const Token token)
    {
        auto itr{m_Groups.find(token)};
        assert(itr != std::end(m_Groups));
        HandlerGroup& group{itr->second};

        EventTypeIds& ids{std::get<EventTypeIds>(group)};
        [[maybe_unused]] const size_t removedIds{ids.erase(TEvent::GetStaticId())};
        assert(removedIds == 1);

        Handlers& handlers{std::get<Handlers>(group)};
        [[maybe_unused]] const size_t removedHandlers{std::erase_if(
            handlers,
            [](const std::unique_ptr<EventHandlerConcept>& handler)
            {
                return handler->GetId() == TEvent::GetStaticId();
            })};
        assert(removedHandlers == 1);
    }

    template<typename TEvent> requires IsEventModel<TEvent>
    void QueueEvent(TEvent&& event)
    {
        std::shared_ptr<EventConcept> sharedEvent{std::make_shared<TEvent>(std::move(event))};
        for(auto& [token, group] : m_Groups)
        {
            const EventTypeIds& ids{std::get<EventTypeIds>(group)};
            if(ids.contains(TEvent::GetStaticId()))
            {
                Events& events{std::get<Events>(group)};
                events.push_back(sharedEvent);
            }
        }
    }

    /// Does not support queueing events or adding/removing listeners during dispatch.
    void DispatchEvents(const Token token)
    {
        auto itr{m_Groups.find(token)};
        assert(itr != std::end(m_Groups));

        const Handlers& handlers{std::get<Handlers>(itr->second)};
        Events& events{std::get<Events>(itr->second)};
        for(const std::shared_ptr<EventConcept>& event : events)
        {
            for(const std::unique_ptr<EventHandlerConcept>& handler : handlers)
            {
                if(handler->HandleEvent(event.get()))
                    break;
            }
        }
        events.clear();
    }

private:
    HandlerGroups m_Groups{};
    static Token ms_TokenSequence;
};

EventQueue::Token EventQueue::ms_TokenSequence{Token::Invalid};
