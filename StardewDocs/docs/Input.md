# Input

The game has a bindable input system allowing you to bind keyboard, mouse and gamepad controls to "buttons" and "axes".

So far the game pad feature has never been tested and the axis feature is quite underdeveloped.

To use the input system you must have a file called Keymap.json in your assets folder:

```json
{
	"Buttons" : {
		"Mouse" : [
			{ "select" : 0 }, 
			{ "altSelect" : 1 }
		],
		"Keyboard" : [
			{ "moveUp" : "W" }, 
			{ "moveLeft" : "A" }, 
			{ "moveDown" : "S" }, 
			{ "moveRight" : "D" },
			{ "playerMoveUp" : "Up"},
			{ "playerMoveDown" : "Down"},
			{ "playerMoveLeft" : "Left"},
			{ "playerMoveRight" : "Right"},
			{ "useTool" : "Space" },
			{ "nextTool" : "E" },
			{ "prevTool" : "Q" },
			{ "settings" : "Escape" },
			{ "nextItem" : "X" },
			{ "prevItem" : "Z" },
			{ "mainActionBinding" : "C"}
		],
		"MouseScroll" : [
			{ "zoomIn" : 2 },
			{ "zoomOut" : 0 }
		],
		"GamePad" : [

		]
		
	},
	"Axes" : {
		"Mouse" : [
			{"CursorPosX" : {"axis" : "x"}},
			{"CursorPosY" : {"axis" : "y"}}
		],
		"GamePad" : [
			{"moveCharacterX"    : {"stick" : "lstick", "axis" : "x"}},
			{"moveCharacterY"    : {"stick" : "lstick", "axis" : "y"}}
		],
		"MouseScroll" : [
			{ "changeTool" : {"axis" : "y"} } 
		]
	}
}
```

You can setup axes and buttons in this file.

Two kinds of value for keyboard buttons are allowed: directly using a numeric json value as a GLFW keyboard code (like GLFW_KEY_SPACE etc) or a string desription of the keyboard button, allowed values are:

- Unknown
- Space
- Apostrophe
- Comma
- Minus
- Period
- Slash
- Num0
- Num1
- Num2
- Num3
- Num4
- Num5
- Num6
- Num7
- Num8
- Num9
- Semicolon
- Equal
- A
- B
- C
- D
- E
- F
- G
- H
- I
- J
- K
- L
- M
- N
- O
- P
- Q
- R
- S
- T
- U
- V
- W
- X
- Y
- Z
- LeftBracket
- Backslash
- RightBracket
- GraveAccent
- World1
- World2
- Escape
- Enter
- Tab
- Backspace
- Insert
- Delete
- Right
- Left
- Down
- Up
- PageUp
- PageDown
- Home
- End
- CapsLock
- ScrollLock
- NumLock
- PrintScreen
- Pause
- F1
- F2
- F3
- F4
- F5
- F6
- F7
- F8
- F9
- F10
- F11
- F12
- F13
- F14
- F15
- F16
- F17
- F18
- F19
- F20
- F21
- F22
- F23
- F24
- F25
- Keypad0
- Keypad1
- Keypad2
- Keypad3
- Keypad4
- Keypad5
- Keypad6
- Keypad7
- Keypad8
- Keypad9
- KeypadDecimal
- KeypadDivide
- KeypadMultiply
- KeypadSubtract
- KeypadAdd
- KeypadEnter
- KeypadEqual
- LeftShift
- LeftControl
- LeftAlt
- LeftSuper
- RightShift
- RightControl
- RightAlt
- RightSuper
- Menu

I'll use some examples from the code to illustrate how to use the input system.
To use a button input in gameplay code you first find a button binding, the string passed refers to one of the button names in the json config file above.

```c
struct ButtonBinding moveUpBinding = In_FindButtonMapping(pInputCtx, "playerMoveUp");
```

Then you build up a mask that describes a set of inputs that you want to be "active" at any given time.

```c
struct ActiveInputBindingsMask playerControlsMask;
memset(&playerControlsMask, 0, sizeof(struct ActiveInputBindingsMask));
In_ActivateButtonBinding(pPlayerEntData->moveUpBinding, &playerControlsMask);
In_ActivateButtonBinding(pPlayerEntData->moveDownBinding, &playerControlsMask);
In_ActivateButtonBinding(pPlayerEntData->moveLeftBinding, &playerControlsMask);
In_ActivateButtonBinding(pPlayerEntData->moveRightBinding, &playerControlsMask);
```

then set the mask to make the inputs useable

```c
In_SetMask(&playerControlsMask, inputContext);
```

then in gameplay code you can do:

```c
if(In_GetButtonValue(context, pPlayerEntData->moveUpBinding))
{
    pPlayerEntData->movementBits |= WfPD_Up;
}
if(In_GetButtonValue(context, pPlayerEntData->moveDownBinding))
{
    pPlayerEntData->movementBits |= WfPD_Down;
}
if(In_GetButtonValue(context, pPlayerEntData->moveLeftBinding))
{
    pPlayerEntData->movementBits |= WfPD_Left;
}
if(In_GetButtonValue(context, pPlayerEntData->moveRightBinding))
{
    pPlayerEntData->movementBits |= WfPD_Right;
}
```

and you can also check whether the button has been pressed or released this frame:

```c
if(In_GetButtonPressThisFrame(context, pPlayerEntData->mainActionBinding)
{

}
if(In_GetButtonReleaseThisFrame(context pPlayerEntData->mainActionBinding))
{

}
```

to use an axis is similar, you find the binding:

```c
pData->freeLookCtrls.freeLookCursorXAxisBinding = In_FindAxisMapping(inputContext, FREE_LOOK_CURSOR_X_BINDING_NAME);
pData->freeLookCtrls.freeLookCursorYAxisBinding = In_FindAxisMapping(inputContext, FREE_LOOK_CURSOR_Y_BINDING_NAME);
```

add them to a mask:

```c
In_ActivateAxisBinding(pData->freeLookCtrls.freeLookCursorXAxisBinding, &pData->freeLookCtrls.freeLookInputsMask);
In_ActivateAxisBinding(pData->freeLookCtrls.freeLookCursorYAxisBinding, &pData->freeLookCtrls.freeLookInputsMask);
```

then set the mask active:

```c
In_SetMask(&pData->freeLookCtrls.freeLookInputsMask, inputContext);
```

Then you can query the axis like so:

```c
int screenX = In_GetAxisValue(context, pData->freeLookCtrls.freeLookCursorXAxisBinding);
int screenY = In_GetAxisValue(context, pData->freeLookCtrls.freeLookCursorYAxisBinding);
```
you can get the active mask, if you you want to set new buttons or axes as active but preserve existing ones:

```c
void In_GetMask(struct ActiveInputBindingsMask* pOutMask, InputContext* pCtx);
```

the entire API is:

```c
void In_GetMask(struct ActiveInputBindingsMask* pOutMask, InputContext* pCtx);
void In_SetMask(struct ActiveInputBindingsMask* mask, InputContext* pCtx);
void In_ActivateButtonBinding(struct ButtonBinding binding, struct ActiveInputBindingsMask* pMask);
void In_ActivateAxisBinding(struct AxisBinding binding, struct ActiveInputBindingsMask* pMask);
void In_DeactivateButtonBinding(struct ButtonBinding binding, struct ActiveInputBindingsMask* pMask);
void In_DeactivateAxisBinding(struct AxisBinding binding, struct ActiveInputBindingsMask* pMask);
```

In the future there will be categories in the config file for a "virtual mouse" axis for game pads, and the ability to treat the mouse wheel as either an axis or a button (with pre defined axis bounds)
