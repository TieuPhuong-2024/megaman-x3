#pragma once
#include "Framework/InputController.h"

class IObserver
{
public:
	virtual void eventKeyDown(KeyEventArg* e);
};

class IObserverKeyUp : public IObserver
{
public:
	virtual void eventKeyUp(KeyEventArg* e);
};