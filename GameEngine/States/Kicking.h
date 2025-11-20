#pragma once
#include "PlayerState.h"

/*
 Kicking state:
 - Short, interruptible melee action.
 - Locks most player horizontal control for the kick duration, sets a brief
   horizontal impulse (optional) and plays the kicking animation.
 - After the kick duration ends, transition back to Standing.
 - If the player is airborne when kicking, we may transition to Falling/Standing
   depending on vertical position; for simplicity this state will return to
   Standing to give the player control back (other states can refine this).
 
 Usage:
   Player code should set this state when the kick input is pressed and the
   player's current state allows kicking (e.g. standing, running).
*/
class Kicking : public PlayerState
{
public:
	Kicking();
	~Kicking();

	// Called each frame to update state internal timers and effects.
	void update(float deltaTime) override;

	// Called to process input while in the kicking state.
	// We intentionally keep this minimal: kicks are short and normally should
	// not be interrupted by directional input (but jump can be allowed).
	void updateInput(float deltaTime) override;

	// Return the enum status representing this state.
	eStatus getState() override;

	// Factory helper (optional, follows pattern used in other states)
	static Kicking* create();

private:
	// Accumulated time (seconds) since the kick started.
	float _timeKick{ 0.0f };

	// How long the kick lasts (seconds).
	const float _kickDuration{ 0.30f };

	// Horizontal impulse applied at the start of the kick; small knockback/step.
	const float _kickImpulseVx{ 180.0f };

	// Whether we already applied the impulse (apply only once).
	bool _impulseApplied{ false };
};