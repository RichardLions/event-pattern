#pragma once

template<typename TEnum> requires std::is_scoped_enum_v<TEnum>
TEnum Increament(TEnum& value)
{
    value = static_cast<TEnum>(static_cast<std::underlying_type_t<TEnum>>(value) + 1);
    return value;
}

enum class EventTypeId : uint32_t { Invalid = 0 };
inline EventTypeId EVENT_TYPE_ID_SEQUENCE{EventTypeId::Invalid};
template<typename TEvent> inline const EventTypeId EVENT_TYPE_ID = Increament(EVENT_TYPE_ID_SEQUENCE);

class EventConcept
{
public:
    virtual ~EventConcept() = default;
    virtual EventTypeId GetId() const = 0;
};

template<typename TEvent>
class EventModel : public EventConcept
{
public:
    [[nodiscard]] EventTypeId GetId() const final { return GetStaticId(); }
    [[nodiscard]] static EventTypeId GetStaticId() { return EVENT_TYPE_ID<TEvent>; }
};

template<class TEvent>
concept IsEventModel = std::derived_from<TEvent, EventModel<TEvent>>;
