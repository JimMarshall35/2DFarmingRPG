#ifndef INPUTCONTEXT_H
#define INPUTCONTEXT_H

#include "DynArray.h"
#include <stdbool.h>

typedef enum
{
	Axis,
	Button
}InputMappingType;

typedef enum
{
	MouseButton,
	KeyboardButton,
	GamepadButton
}ButtonSubType;

typedef enum
{
	MouseAxis,
	GamePadAxis,
	MouseScrollAxis
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
typedef struct
{
	InputMappingType type;
	const char* name;
	union
	{
		struct
		{
			ButtonSubType type;
			bool bCurrent;
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

typedef struct
{
	/*InputMapping AxisMappings[MAX_MAPPINGS];
	InputMapping ButtonMapping[MAX_MAPPINGS];*/
	struct
	{
		InputMappingArray MouseButtonMappings;
		InputMappingArray KeyboardButtonMappings;
		InputMappingArray GamepadMappings;
	}buttonMappings;

	struct
	{

		InputMappingArray Mouse;
		InputMappingArray Controller;
		InputMappingArray MouseScroll;
	}axisMappings;

	int screenW, screenH;
}InputContext;

void In_RecieveKeyboardKey(InputContext* context, int key, int scancode, int action, int mods);
void In_RecieveMouseMove(InputContext* context, double xposIn, double yposIn);
void In_RecieveMouseButton(InputContext* context, int button, int action, int mods);
void In_FramebufferResize(InputContext* context, int width, int height);
void In_RecieveScroll(InputContext* context, double xoffset, double yoffset);
void In_SetControllerPresent(int controllerNo);
void In_Poll();
InputContext In_InitInputContext();

#endif // !INPUTCONTEXT_H
