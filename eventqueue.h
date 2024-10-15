#pragma once

#include <functional>
#include <unordered_set>

#include "eventconsumer.h"

class EventQueue
{
public:
    enum class ConsumerToken : uint32_t { Invalid = 0 };
    using EventTypeIds = std::unordered_set<EventTypeId>;
    using Consumers = std::vector<std::unique_ptr<EventConsumerConcept>>;
    using Events = std::vector<std::shared_ptr<EventConcept>>;
    using ConsumerGroup = std::tuple<EventTypeIds, Consumers, Events>;
    using ConsumerGroups = std::unordered_map<ConsumerToken, ConsumerGroup>;

    [[nodiscard]] static ConsumerToken GetConsumerToken()
    {
        return Increament(ms_TokenSequence);
    }

    template<typename TEvent> requires IsEventModel<TEvent>
    void RegisterConsumer(const ConsumerToken token, EventConsumerModel<TEvent>::EventConsumer&& consumer)
    {
        assert(token > ConsumerToken::Invalid);
        ConsumerGroup& group{m_Groups[token]};

        EventTypeIds& ids{std::get<EventTypeIds>(group)};
        assert(ids.contains(TEvent::GetStaticId()) == false);
        ids.insert(TEvent::GetStaticId());

        Consumers& consumers{std::get<Consumers>(group)};
        consumers.push_back(std::make_unique<EventConsumerModel<TEvent>>(std::move(consumer)));
    }

    template<typename TEvent> requires IsEventModel<TEvent>
    void UnregisterConsumer(const ConsumerToken token)
    {
        auto itr{m_Groups.find(token)};
        assert(itr != std::end(m_Groups));
        ConsumerGroup& group{itr->second};

        EventTypeIds& ids{std::get<EventTypeIds>(group)};
        [[maybe_unused]] const size_t removedIds{ids.erase(TEvent::GetStaticId())};
        assert(removedIds == 1);

        Consumers& consumers{std::get<Consumers>(group)};
        [[maybe_unused]] const size_t removedConsumers{std::erase_if(
            consumers,
            [](const std::unique_ptr<EventConsumerConcept>& consumer)
            {
                return consumer->GetId() == TEvent::GetStaticId();
            })};
        assert(removedConsumers == 1);
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

    /// Does not support queueing events or adding/removing consumers during event consumption.
    void ConsumeEvents(const ConsumerToken token)
    {
        auto itr{m_Groups.find(token)};
        assert(itr != std::end(m_Groups));
        ConsumerGroup& group{itr->second};

        const Consumers& consumers{std::get<Consumers>(group)};
        Events& events{std::get<Events>(group)};
        for(const std::shared_ptr<EventConcept>& event : events)
        {
            for(const std::unique_ptr<EventConsumerConcept>& consumer : consumers)
            {
                if(consumer->ConsumeEvent(event.get()))
                    break;
            }
        }
        events.clear();
    }
private:
    ConsumerGroups m_Groups{};
    static ConsumerToken ms_TokenSequence;
};

EventQueue::ConsumerToken EventQueue::ms_TokenSequence{ConsumerToken::Invalid};
