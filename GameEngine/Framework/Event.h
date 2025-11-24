#pragma once

#include <functional>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <typeindex>
#include <any>

namespace Framework
{
    // Forward declarations
    class Event;
    class EventDispatcher;
    class EventListener;

    // Base event class
    class Event
    {
    public:
        virtual ~Event() = default;
        virtual std::type_index getType() const = 0;
        virtual std::string getName() const = 0;
        virtual std::shared_ptr<Event> clone() const = 0;

        // Event properties
        bool isHandled() const { return _handled; }
        void setHandled(bool handled) { _handled = handled; }
        bool isPropagationStopped() const { return _propagationStopped; }
        void stopPropagation() { _propagationStopped = true; }

        // Add custom data to events
        template <typename T>
        void setData(const std::string &key, T &&value)
        {
            _data[key] = std::make_shared<std::decay_t<T>>(std::forward<T>(value));
        }

        template <typename T>
        T getData(const std::string &key) const
        {
            auto it = _data.find(key);
            if (it != _data.end())
            {
                return *std::static_pointer_cast<T>(it->second);
            }
            return T{};
        }

        template <typename T>
        bool hasData(const std::string &key) const
        {
            auto it = _data.find(key);
            return it != _data.end();
        }

    private:
        bool _handled = false;
        bool _propagationStopped = false;
        std::unordered_map<std::string, std::shared_ptr<void>> _data;
    };

    // Specific event types
    class KeyEvent : public Event
    {
    public:
        enum class Type
        {
            KeyDown,
            KeyUp,
            KeyPressed,
            KeyReleased
        };

        KeyEvent(Type type, int keyCode) : _type(type), _keyCode(keyCode) {}

        Type getEventType() const { return _type; }
        int getKeyCode() const { return _keyCode; }

        std::type_index getType() const override { return std::type_index(typeid(KeyEvent)); }
        std::string getName() const override { return "KeyEvent"; }
        std::shared_ptr<Event> clone() const override { return std::make_shared<KeyEvent>(*this); }

    private:
        Type _type;
        int _keyCode;
    };

    class MouseEvent : public Event
    {
    public:
        enum class Type
        {
            MouseDown,
            MouseUp,
            MouseMove,
            MouseWheel
        };

        MouseEvent(Type type, int x, int y, int button = 0)
            : _type(type), _x(x), _y(y), _button(button) {}

        Type getEventType() const { return _type; }
        int getX() const { return _x; }
        int getY() const { return _y; }
        int getButton() const { return _button; }

        std::type_index getType() const override { return std::type_index(typeid(MouseEvent)); }
        std::string getName() const override { return "MouseEvent"; }
        std::shared_ptr<Event> clone() const override { return std::make_shared<MouseEvent>(*this); }

    private:
        Type _type;
        int _x, _y;
        int _button;
    };

    class CollisionEvent : public Event
    {
    public:
        CollisionEvent(void *objectA, void *objectB, const std::string &collisionType = "default")
            : _objectA(objectA), _objectB(objectB), _collisionType(collisionType) {}

        void *getObjectA() const { return _objectA; }
        void *getObjectB() const { return _objectB; }
        const std::string &getCollisionType() const { return _collisionType; }

        std::type_index getType() const override { return std::type_index(typeid(CollisionEvent)); }
        std::string getName() const override { return "CollisionEvent"; }
        std::shared_ptr<Event> clone() const override { return std::make_shared<CollisionEvent>(*this); }

    private:
        void *_objectA;
        void *_objectB;
        std::string _collisionType;
    };

    class GameEvent : public Event
    {
    public:
        enum class Type
        {
            GameStart,
            GameEnd,
            GamePause,
            GameResume,
            GameOver
        };

        GameEvent(Type type) : _type(type) {}

        Type getEventType() const { return _type; }

        std::type_index getType() const override { return std::type_index(typeid(GameEvent)); }
        std::string getName() const override { return "GameEvent"; }
        std::shared_ptr<Event> clone() const override { return std::make_shared<GameEvent>(*this); }

    private:
        Type _type;
    };

    class CustomEvent : public Event
    {
    public:
        CustomEvent(const std::string &eventName) : _eventName(eventName) {}

        const std::string &getEventName() const { return _eventName; }

        std::type_index getType() const override { return std::type_index(typeid(CustomEvent)); }
        std::string getName() const override { return _eventName; }
        std::shared_ptr<Event> clone() const override { return std::make_shared<CustomEvent>(*this); }

    private:
        std::string _eventName;
    };

    // Event listener function type
    using EventListenerFunc = std::function<void(std::shared_ptr<Event>)>;

    // Event listener class
    class EventListener
    {
    public:
        EventListener(const EventListenerFunc &func, const std::string &name = "")
            : _func(func), _name(name) {}

        void operator()(std::shared_ptr<Event> event) const
        {
            if (_func)
            {
                _func(event);
            }
        }

        const std::string &getName() const { return _name; }

    private:
        EventListenerFunc _func;
        std::string _name;
    };

    // Event dispatcher - central event management
    class EventDispatcher
    {
    public:
        ~EventDispatcher()
        {
            removeAllListeners();
        }

        // Add listener for specific event type
        template <typename T>
        void addEventListener(const EventListenerFunc &listener, const std::string &name = "")
        {
            auto typeId = std::type_index(typeid(T));
            auto &listeners = _listeners[typeId];
            listeners.push_back(std::make_shared<EventListener>(listener, name));
        }

        // Remove listener by name
        void removeEventListener(const std::type_index &typeId, const std::string &name)
        {
            auto it = _listeners.find(typeId);
            if (it != _listeners.end())
            {
                auto &listeners = it->second;
                listeners.erase(
                    std::remove_if(listeners.begin(), listeners.end(),
                                   [name](const std::shared_ptr<EventListener> &listener)
                                   {
                                       return listener->getName() == name;
                                   }),
                    listeners.end());
            }
        }

        // Remove all listeners for a specific event type
        void removeEventListeners(const std::type_index &typeId)
        {
            _listeners.erase(typeId);
        }

        // Remove all listeners
        void removeAllListeners()
        {
            _listeners.clear();
        }

        // Dispatch event to all listeners
        void dispatchEvent(std::shared_ptr<Event> event)
        {
            if (!event || event->isPropagationStopped())
            {
                return;
            }

            auto typeId = event->getType();
            auto it = _listeners.find(typeId);
            if (it != _listeners.end())
            {
                // Create a copy of listeners to avoid issues if listeners modify the list
                auto listenersCopy = it->second;
                for (auto &listener : listenersCopy)
                {
                    if (event->isPropagationStopped())
                    {
                        break;
                    }
                    (*listener)(event);
                }
            }

            // Always dispatch to wildcard listeners (listening to all events)
            auto wildcardIt = _listeners.find(std::type_index(typeid(void)));
            if (wildcardIt != _listeners.end())
            {
                auto listenersCopy = wildcardIt->second;
                for (auto &listener : listenersCopy)
                {
                    if (event->isPropagationStopped())
                    {
                        break;
                    }
                    (*listener)(event);
                }
            }
        }

        // Check if there are listeners for a specific event type
        bool hasEventListeners(const std::type_index &typeId) const
        {
            auto it = _listeners.find(typeId);
            return it != _listeners.end() && !it->second.empty();
        }

        // Get count of listeners for a specific event type
        size_t getListenerCount(const std::type_index &typeId) const
        {
            auto it = _listeners.find(typeId);
            if (it != _listeners.end())
            {
                return it->second.size();
            }
            return 0;
        }

        // Global instance
        static EventDispatcher &getInstance()
        {
            static EventDispatcher instance;
            return instance;
        }

    private:
        std::unordered_map<std::type_index, std::vector<std::shared_ptr<EventListener>>> _listeners;

        EventDispatcher() = default;
        EventDispatcher(const EventDispatcher &) = delete;
        EventDispatcher &operator=(const EventDispatcher &) = delete;
    };

    // Convenience functions for global event management
    namespace Events
    {
        // Template function to add event listener
        template <typename T, typename Func>
        void addListener(Func &&func, const std::string &name = "")
        {
            EventDispatcher::getInstance().addEventListener<T>(std::forward<Func>(func), name);
        }

        // Template function to remove event listener
        template <typename T>
        void removeListener(const std::string &name)
        {
            EventDispatcher::getInstance().removeEventListener(std::type_index(typeid(T)), name);
        }

        // Dispatch any event
        inline void dispatch(std::shared_ptr<Event> event)
        {
            EventDispatcher::getInstance().dispatchEvent(event);
        }

        // Convenience functions for common events
        inline void dispatchKeyEvent(KeyEvent::Type type, int keyCode)
        {
            dispatch(std::make_shared<KeyEvent>(type, keyCode));
        }

        inline void dispatchMouseEvent(MouseEvent::Type type, int x, int y, int button = 0)
        {
            dispatch(std::make_shared<MouseEvent>(type, x, y, button));
        }

        inline void dispatchCollisionEvent(void *objectA, void *objectB, const std::string &collisionType = "default")
        {
            dispatch(std::make_shared<CollisionEvent>(objectA, objectB, collisionType));
        }

        inline void dispatchGameEvent(GameEvent::Type type)
        {
            dispatch(std::make_shared<GameEvent>(type));
        }

        inline void dispatchCustomEvent(const std::string &eventName)
        {
            dispatch(std::make_shared<CustomEvent>(eventName));
        }
    }

    // RAII Event Listener wrapper for automatic cleanup
    template <typename T>
    class EventListenerScope
    {
    public:
        EventListenerScope(EventListenerFunc &&func, const std::string &name = "")
            : _typeId(std::type_index(typeid(T))), _name(name)
        {
            EventDispatcher::getInstance().addEventListener<T>(std::forward<EventListenerFunc>(func), name);
        }

        ~EventListenerScope()
        {
            EventDispatcher::getInstance().removeEventListener(_typeId, _name);
        }

        // Non-copyable, non-movable
        EventListenerScope(const EventListenerScope &) = delete;
        EventListenerScope &operator=(const EventListenerScope &) = delete;
        EventListenerScope(EventListenerScope &&) = delete;
        EventListenerScope &operator=(EventListenerScope &&) = delete;

    private:
        std::type_index _typeId;
        std::string _name;
    };
}