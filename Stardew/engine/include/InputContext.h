#ifndef INPUTCONTEXT_H
#define INPUTCONTEXT_H

#include "DynArray.h"
#include <stdbool.h>
#include "HandleDefs.h"

#define KEYSTROKE_LEFT      263
#define KEYSTROKE_RIGHT     262
#define KEYSTROKE_UP        265
#define KEYSTROKE_DOWN      264
#define KEYSTROKE_BACKSPACE 259
#define KEYSTROKE_ENTER     257


typedef enum
{
	Axis,
	Button
}InputMappingType;

typedef enum
{
	MouseButton,
	KeyboardButton,
	GamepadButton,
	MouseScrollButton,
	UnknownButton
}ButtonSubType;

typedef enum
{
	MouseAxis,
	GamePadAxis,
	MouseScrollAxis,
	UnknownAxis
}AxisSubType;

typedef enum
{
	gpAxis_LStick,
	gpAxis_RStick,
	gpAxis_LT,
	gpAxis_RT
}ControllerAxisType;

typedef enum
{
	Axis_X,
	Axis_Y
} WhichAxis;

typedef enum
{
	Axis_Pos,
	Axis_Neg
} WhichDirection;

typedef struct InputMapping
{
	InputMappingType type;
	char* name;
	union
	{
		struct
		{
			ButtonSubType type;
			bool bCurrent;
			bool bPressThisFrame;
			bool bReleaseThisFrame;
			union
			{
				struct
				{
					int keyboadCode;
				}keyboard;
				struct
				{
					int button;
				}mouseBtn;
				struct
				{
					int button;
				}gamepadBtn;
				struct
				{
					WhichAxis axis;
					WhichDirection dir;
				}mouseScrollButton;
			}data;
			// in future, game pad for example
		}ButtonMapping;
		struct
		{

			AxisSubType type;
			double fCurrent;
			union
			{
				struct
				{
					WhichAxis axis;
				}mouse;
				struct
				{
					ControllerAxisType type;
					WhichAxis axis;
				}controller;
				struct
				{
					WhichAxis axis;
				}mouseScroll;
			}data;
			
		}axisMapping;
	}data;
}InputMapping;

#define MAX_MAPPINGS 64

typedef struct
{
	InputMapping arr[MAX_MAPPINGS];
	int size;
	u64 ActiveMask;
}InputMappingArray;

#define MAX_KEYS_DOWN_PER_FRAME 256
struct TextInputState
{
	int keystrokes[MAX_KEYS_DOWN_PER_FRAME];
	int nKeystrokesThisFrame;
	bool capslockModifier;
	bool shiftModifier;
};

typedef struct InputContext
{
	/*InputMapping AxisMappings[MAX_MAPPINGS];
	InputMapping ButtonMapping[MAX_MAPPINGS];*/
	struct
	{
		InputMappingArray MouseButtonMappings;
		InputMappingArray KeyboardButtonMappings;
		InputMappingArray GamepadMappings;
		InputMappingArray MouseScrollButtonMappings;
	}buttonMappings;

	struct
	{

		InputMappingArray Mouse;
		InputMappingArray Controller;
		InputMappingArray MouseScroll;
	}axisMappings;

	struct TextInputState textInput;
	int screenW, screenH;
}InputContext;

/*
	Called by the glfw callbacks
*/
void In_RecieveKeyboardKey(InputContext* context, int key, int scancode, int action, int mods);
void In_RecieveMouseMove(InputContext* context, double xposIn, double yposIn);
void In_RecieveMouseButton(InputContext* context, int button, int action, int mods);
void In_FramebufferResize(InputContext* context, int width, int height);
void In_RecieveScroll(InputContext* context, double xoffset, double yoffset);
void In_SetControllerPresent(int controllerNo);

/*
	Called by the game framework
*/
void In_EndFrame(InputContext* context);
InputContext In_InitInputContext();

/*
	Called by game code and/or "game layer" level code
*/

struct AxisBinding
{
	AxisSubType type;
	int index;
};

struct ButtonBinding
{
	ButtonSubType type;
	int index;
};

struct AxisBinding In_FindAxisMapping(InputContext* context, const char* name);
struct ButtonBinding In_FindButtonMapping(InputContext* context, const char* name);

float In_GetAxisValue(InputContext* context, struct AxisBinding binding);
bool In_GetButtonValue(InputContext* context, struct ButtonBinding binding);
bool In_GetButtonPressThisFrame(InputContext* context, struct ButtonBinding binding);
bool In_GetButtonReleaseThisFrame(InputContext* context, struct ButtonBinding binding);

struct ActiveInputBindingsMask
{
	u64 MouseButtonMappings;
	u64 KeyboardButtonMappings;
	u64 GamepadButtonMappings;
	u64 MouseScrollButtonMappings;

	u64 MouseAxisMappings;
	u64 ControllerAxisMappings;
	u64 MouseScrollAxisMappings;
};

void In_GetMask(struct ActiveInputBindingsMask* pOutMask, InputContext* pCtx);
void In_SetMask(struct ActiveInputBindingsMask* mask, InputContext* pCtx);
void In_ActivateButtonBinding(struct ButtonBinding binding, struct ActiveInputBindingsMask* pMask);
void In_ActivateAxisBinding(struct AxisBinding binding, struct ActiveInputBindingsMask* pMask);
void In_DeactivateButtonBinding(struct ButtonBinding binding, struct ActiveInputBindingsMask* pMask);
void In_DeactivateAxisBinding(struct AxisBinding binding, struct ActiveInputBindingsMask* pMask);

#endif // !INPUTCONTEXT_H
