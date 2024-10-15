#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include "eventqueue.h"

class EventA final : public EventModel<EventA>
{
public:
    explicit EventA(const uint32_t value)
        : m_Value{value}
    {
    }

    uint32_t m_Value{0};
};

class EventB final : public EventModel<EventB>
{
public:
    explicit EventB(const bool value)
        : m_Value{value}
    {
    }

    bool m_Value{0};
};

TEST_CASE("Event Pattern - Static Event Ids")
{
    REQUIRE(EventA::GetStaticId() > EventTypeId::Invalid);
    REQUIRE(EventB::GetStaticId() > EventTypeId::Invalid);

    REQUIRE(EventA::GetStaticId() == EventA::GetStaticId());
    REQUIRE(EventB::GetStaticId() == EventB::GetStaticId());
    REQUIRE(EventA::GetStaticId() != EventB::GetStaticId());

    const EventA eventA{0};
    const EventA eventAA{0};
    const EventB eventB{false};
    const EventB eventBB{false};

    REQUIRE(eventA.GetId() == eventAA.GetId());
    REQUIRE(eventB.GetId() == eventBB.GetId());
    REQUIRE(eventA.GetId() != eventB.GetId());

    const EventConcept* const eventAPtr{&eventA};
    const EventConcept* const eventAAPtr{&eventAA};
    const EventConcept* const eventBPtr{&eventB};
    const EventConcept* const eventBBPtr{&eventBB};

    REQUIRE(eventAPtr->GetId() == eventAAPtr->GetId());
    REQUIRE(eventBPtr->GetId() == eventBBPtr->GetId());
    REQUIRE(eventAPtr->GetId() != eventBPtr->GetId());
}

TEST_CASE("Event Pattern - Register/Unregister Event Consumer")
{
    EventQueue eventQueue{};
    const EventQueue::ConsumerToken token{EventQueue::GetConsumerToken()};
    REQUIRE(token > EventQueue::ConsumerToken::Invalid);

    uint32_t value{0};
    eventQueue.RegisterConsumer<EventA>(
        token,
        [&value](const EventA& event)
        {
            REQUIRE(event.m_Value == 1);
            ++value;
        });
    eventQueue.RegisterConsumer<EventB>(
        token,
        [&value](const EventB& event)
        {
            REQUIRE(event.m_Value == true);
            --value;
        });
    eventQueue.QueueEvent(EventA{1});
    REQUIRE(value == 0);

    eventQueue.ConsumeEvents(token);
    eventQueue.ConsumeEvents(token);
    REQUIRE(value == 1);

    eventQueue.QueueEvent(EventA{2});
    eventQueue.UnregisterConsumer<EventA>(token);
    eventQueue.ConsumeEvents(token);
    REQUIRE(value == 1);

    eventQueue.QueueEvent(EventA{2});
    eventQueue.QueueEvent(EventB{true});
    eventQueue.ConsumeEvents(token);
    REQUIRE(value == 0);

    eventQueue.QueueEvent(EventB{false});
    eventQueue.UnregisterConsumer<EventB>(token);
    eventQueue.ConsumeEvents(token);
    REQUIRE(value == 0);
}

TEST_CASE("Event Pattern  - Benchmarks")
{
    BENCHMARK("Consume Events")
    {
        constexpr uint32_t eventCount{100'000};

        EventQueue eventQueue{};
        const EventQueue::ConsumerToken token{EventQueue::GetConsumerToken()};

        eventQueue.RegisterConsumer<EventA>(
            token,
            [](const EventA&){});
        eventQueue.RegisterConsumer<EventB>(
            token,
            [](const EventB&){});

        for(uint32_t i{0}; i != eventCount; ++i)
        {
            eventQueue.QueueEvent(EventA{1});
            eventQueue.QueueEvent(EventB{1});
            eventQueue.ConsumeEvents(token);
        }
    };

    BENCHMARK("Register/Unregister Event Consumer")
    {
        constexpr uint32_t registerCount{10'000};

        std::vector<EventQueue::ConsumerToken> tokens{};
        tokens.reserve(registerCount);

        EventQueue eventQueue{};
        for(uint32_t i{0}; i != registerCount; ++i)
        {
            const EventQueue::ConsumerToken token{EventQueue::GetConsumerToken()};
            eventQueue.RegisterConsumer<EventA>(
                token,
                [](const EventA&){});
            eventQueue.RegisterConsumer<EventB>(
                token,
                [](const EventB&){});
            tokens.push_back(token);
        }

        for(const EventQueue::ConsumerToken token : tokens)
        {
            eventQueue.UnregisterConsumer<EventB>(token);
            eventQueue.UnregisterConsumer<EventA>(token);
        }
    };
}
