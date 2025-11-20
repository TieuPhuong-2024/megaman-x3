#ifndef __EVENT_H__
#define __EVENT_H__
#include <vector>
#include <algorithm>
#include "define.h"
#include <functional>

class EventArg;
typedef void (*EventFunction)(EventArg*); // or you can use: using EventFunction = void(*)(EventArg*);

class Event
{
public:
	Event();
	virtual ~Event();
	virtual void fireEvent(EventArg* e); // active an event - kích hoạt sự kiện
	void operator+=(EventFunction pFunction); // register an action - đăng kí sự kiện
	void operator-=(EventFunction pFunction); // remove an action - huỷ sự kiện
private:
	std::vector<EventFunction> _listAction;
};

typedef Event* pEvent;

class EventArg
{
public:
	EventArg()
	{
	}

	virtual ~EventArg()
	{
	}
};


#endif // !__EVENT_H__
