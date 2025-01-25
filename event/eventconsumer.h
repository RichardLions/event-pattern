#pragma once

#include "event.h"

namespace Event
{
    class EventConsumerConcept
    {
    public:
        virtual ~EventConsumerConcept() = default;
        virtual bool ConsumeEvent(const EventConcept*) const = 0;
        virtual EventTypeId GetId() const = 0;
    };

    template<typename TEvent> requires IsEventModel<TEvent>
    class EventConsumerModel final : public EventConsumerConcept
    {
    public:
        using EventConsumer = std::function<void(const TEvent&)>;

        EventConsumerModel(EventConsumer&& consumer)
            : m_Consumer{std::move(consumer)}
        {
        }

        bool ConsumeEvent(const EventConcept* const event) const override
        {
            if(event->GetId() != GetStaticId())
                return false;

            m_Consumer(static_cast<const TEvent&>(*event));
            return true;
        }

        [[nodiscard]] EventTypeId GetId() const override { return GetStaticId(); }
        [[nodiscard]] static EventTypeId GetStaticId() { return TEvent::GetStaticId(); }
    private:
        EventConsumer m_Consumer{};
    };
}
