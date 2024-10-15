#pragma once

#include "event.h"

class EventHandlerConcept
{
public:
    virtual ~EventHandlerConcept() = default;
    virtual bool HandleEvent(const EventConcept*) const = 0;
    virtual EventTypeId GetId() const = 0;
};

template<typename TEvent> requires IsEventModel<TEvent>
class EventHandlerModel final : public EventHandlerConcept
{
public:
    using EventHandler = std::function<void(const TEvent&)>;

    EventHandlerModel(EventHandler&& handler)
        : m_Handler{std::move(handler)}
    {
    }

    bool HandleEvent(const EventConcept* const event) const override
    {
        if(event->GetId() != GetStaticId())
            return false;

        m_Handler(static_cast<const TEvent&>(*event));
        return true;
    }

    [[nodiscard]] EventTypeId GetId() const override { return GetStaticId(); }
    [[nodiscard]] static EventTypeId GetStaticId() { return TEvent::GetStaticId(); }
private:
    EventHandler m_Handler{};
};
