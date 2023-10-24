#pragma once

namespace W3D
{

/*
* The EventType, MouseButton, KeyCode, KeyAction, MouseAction,
* Event, ResizeEvent KeyPressEvent, MouseInputEvent types defined
* below are for us to manage user input and provide the appropriate
* responses.
*/

// WE CARE ABOUT THREE BROAD TYPES OF EVENTS
enum class EventType
{
	eKeyInput,
	eMouseButton,
	eResize
};

// THE POSSIBLE MOUSE BUTTONS WE WILL RESPOND DO
enum class MouseButton
{
	eLeft,
	eRight,
	eMiddle,
	eUnknown
};

// THE POSSIBLE KEYBOARD KEYS WE WILL RESPOND TO
enum class KeyCode
{
	eUnknown,

	eW,
	eS,
	eA,
	eD,

	e1,
	e2,
	e3,
};

// THERE ARE FOUR DIFFERENT TYPES OF KEYBOARD KEY INTERACTIONS
enum class KeyAction
{
	eDown,
	eUp,
	eRepeat,
	eUnknown
};

// THERE ARE FOUR DIFFERENT TYPES OF MOUSE BUTTON INTERACTIONS
enum class MouseAction
{
	eDown,
	eUp,
	eMove,
	eUnknown
};

// Event OBJECTS WILL STORE INFORMATION ABOUT A GIVEN EVENT, i.e. USER INTERACTION
struct Event
{
	Event(EventType type) :
	    type(type){};
	EventType type;
};

// FOR WHEN THE USER RESIZES THE WINDOW
struct ResizeEvent : Event
{
	ResizeEvent() :
	    Event(EventType::eResize){};
};

// FOR WHEN THE USER PRESSES/RELEASES A KEYBOARD KEY
struct KeyInputEvent : Event
{
	KeyInputEvent(KeyCode code, KeyAction action) :
	    Event(EventType::eKeyInput),
	    code(code),
	    action(action)
	{
	}
	KeyCode   code;
	KeyAction action;
};

// FOR WHEN THE USER PRESSES/RELEASES/MOVES THE MOUSE
struct MouseButtonInputEvent : Event
{
	MouseButtonInputEvent(MouseButton button, MouseAction action, float pos_x, float pos_y) :
	    Event(EventType::eMouseButton),
	    button(button),
	    action(action),
	    xpos(pos_x),
	    ypos(pos_y)
	{
	}
	MouseButton button;
	MouseAction action;
	float       xpos;
	float       ypos;
};

}        // namespace W3D