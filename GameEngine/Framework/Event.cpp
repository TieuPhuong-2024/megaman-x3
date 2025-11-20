#include <algorithm>
#include <exception>
#include "Event.h"

Event::Event()
{
}

Event::~Event()
{
}

void Event::fireEvent(EventArg* e)
{
	if (_listAction.empty())
		return;
	for (auto action : _listAction)
	{
		action(e);
	}
}

void Event::operator+=(EventFunction pFunction)
{
	_listAction.push_back(pFunction);
}

void Event::operator-=(EventFunction pFunction)
{
	auto it = find(_listAction.begin(), _listAction.end(), pFunction);
	if (it == _listAction.end())
		throw std::exception();
	_listAction.erase(it);
}
