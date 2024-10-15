# Event Pattern

This pattern was inspired by [Game Programming Patterns](https://gameprogrammingpatterns.com/event-queue.html) & [Event System](https://denyskryvytskyi.github.io/event-system).

## When To Use

The Event pattern reduces dependencies between event producers and event consumers. This is achieved by produced events being stored so they can be consumed asynchronously. Consuming events asynchronously opens up opportunities on when and how they are consumed based on technical requirements.

## Features

The example Event Queue implemented in this repository has the following features:
* Each consumer is uniquely identified by a consumer token.
* Each consumer chooses when to consume events.
* Each consumer has their own event queue. This means each consumer will receive a copy of any event they have registered for and can consume the event independently of other consumers.

## Example

Example producing and consuming an event.

```cpp
class Event final : public EventModel<Event>
{
public:
    ...
};

{
    EventQueue eventQueue{};

    const EventQueue::ConsumerToken token{EventQueue::GetConsumerToken()};
    eventQueue.RegisterConsumer<Event>(
        token,
        [&value](const Event& event)
        {
            ...
        });

    eventQueue.QueueEvent(Event{...});
    eventQueue.ConsumeEvents(token);
    eventQueue.UnregisterConsumer<Event>(token);
}
```

## Setup

This repository uses the .sln/.proj files created by Visual Studio 2022 Community Edition.
Using MSVC compiler, Preview version(C++23 Preview). 

### Catch2
The examples for how to use the pattern are written as Unit Tests.

Launching the program in Debug or Release will run the Unit Tests.

Alternative:
Installing the Test Adapter for Catch2 Visual Studio extension enables running the Unit Tests via the Test Explorer Window. Setup the Test Explorer to use the project's .runsettings file.

### vcpkg
This repository uses vcpkg in manifest mode for it's dependencies. To interact with vcpkg, open a Developer PowerShell (View -> Terminal).

To setup vcpkg, install it via the Visual Studio installer. To enable/disable it run these commands from the Developer PowerShell:
```
vcpkg integrate install
vcpkg integrate remove
```

To add additional dependencies run:
```
vcpkg add port [dependency name]
```

To update the version of a dependency modify the overrides section of vcpkg.json. 

To create a clean vcpkg.json and vcpkg-configuration.json file run:
```
vcpkg new --application
```

### TODO
- [x] Implementation
- [x] Unit Tests
- [x] Benchmarks
