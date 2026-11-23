#include "InputContext.h"
#include <string.h>
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "Platform.h"
#include "KeyCodes.h"
#include "FileHelpers.h"
#include "AssertLib.h"
#include "Log.h"
#include "StringKeyHashMap.h"
#include "cwalk.h"
#include "main.h"

static int gJoystick = -1;

struct HashMap gGLFWCodeKeyboardNameLUT;

int In_FindMouseButtonMapping(InputContext* context, const char* name)
{
	for (int i = 0; i < context->buttonMappings.MouseButtonMappings.size; i++)
	{
		if (strcmp(context->buttonMappings.MouseButtonMappings.arr[i].name, name) == 0)
		{
			return i;
		}
	}
	return NULL_HANDLE;
}

int In_FindKeyboardButtonMapping(InputContext* context, const char* name)
{
	for (int i = 0; i < context->buttonMappings.KeyboardButtonMappings.size; i++)
	{
		if (strcmp(context->buttonMappings.KeyboardButtonMappings.arr[i].name, name) == 0)
		{
			return i;
		}
	}
	return NULL_HANDLE;
}

int In_FindGamepadButtonMapping(InputContext* context, const char* name)
{
	for (int i = 0; i < context->buttonMappings.GamepadMappings.size; i++)
	{
		if (strcmp(context->buttonMappings.GamepadMappings.arr[i].name, name) == 0)
		{
			return i;
		}
	}
	return NULL_HANDLE;
}

int In_FindMouseScrollButtonMapping(InputContext* context, const char* name)
{
	for (int i = 0; i < context->buttonMappings.MouseScrollButtonMappings.size; i++)
	{
		if (strcmp(context->buttonMappings.MouseScrollButtonMappings.arr[i].name, name) == 0)
		{
			return i;
		}
	}
	return NULL_HANDLE;
}

int In_FindMouseAxisMapping(InputContext* context, const char* name)
{
	for (int i = 0; i < context->axisMappings.Mouse.size; i++)
	{
		if (strcmp(context->axisMappings.Mouse.arr[i].name, name) == 0)
		{
			return i;
		}
	}
	return NULL_HANDLE;
}

int In_FindControllerAxisMapping(InputContext* context, const char* name)
{
	for (int i = 0; i < context->axisMappings.Controller.size; i++)
	{
		if (strcmp(context->axisMappings.Controller.arr[i].name, name) == 0)
		{
			return i;
		}
	}
	return NULL_HANDLE;
}

int In_FindMouseScrollAxisMapping(InputContext* context, const char* name)
{
	for (int i = 0; i < context->axisMappings.MouseScroll.size; i++)
	{
		if (strcmp(context->axisMappings.MouseScroll.arr[i].name, name) == 0)
		{
			return i;
		}
	}
	return NULL_HANDLE;
}


struct AxisBinding In_FindAxisMapping(InputContext* context, const char* name)
{
	int handle = In_FindMouseAxisMapping(context, name);
	if (handle != NULL_HANDLE)
	{
		struct AxisBinding b = { MouseAxis, handle };
		return b;
	}
	handle = In_FindControllerAxisMapping(context, name);
	if (handle != NULL_HANDLE)
	{
		struct AxisBinding b = { GamePadAxis, handle };
		return b;
	}
	handle = In_FindMouseScrollAxisMapping(context, name);
	if (handle != NULL_HANDLE)
	{
		struct AxisBinding b = { MouseScrollAxis, handle };
		return b;
	}
	struct AxisBinding b = { UnknownAxis, NULL_HANDLE };
	return b;
}

struct ButtonBinding In_FindButtonMapping(InputContext* context, const char* name)
{
	int handle = In_FindMouseButtonMapping(context, name);
	if (handle != NULL_HANDLE)
	{
		struct ButtonBinding b = { MouseButton, handle };
		return b;
	}
	handle = In_FindGamepadButtonMapping(context, name);
	if (handle != NULL_HANDLE)
	{
		struct ButtonBinding b = { GamepadButton, handle };
		return b;
	}
	handle = In_FindKeyboardButtonMapping(context, name);
	if (handle != NULL_HANDLE)
	{
		struct ButtonBinding b = { KeyboardButton, handle };
		return b;
	}
	handle = In_FindMouseScrollButtonMapping(context, name);
	if (handle != NULL_HANDLE)
	{
		struct ButtonBinding b = { MouseScrollButton, handle };
		return b;
	}
	struct ButtonBinding b = { UnknownButton, NULL_HANDLE };
	return b;
}

float In_GetAxisValue(InputContext* context, struct AxisBinding binding)
{
	EASSERT(binding.index > -1);
	EASSERT(binding.type != UnknownAxis);
	switch (binding.type)
	{
	case MouseAxis:
		return context->axisMappings.Mouse.arr[binding.index].data.axisMapping.fCurrent;
		break;
	case GamePadAxis:
		return context->axisMappings.Controller.arr[binding.index].data.axisMapping.fCurrent;
	case MouseScrollAxis:
		return context->axisMappings.MouseScroll.arr[binding.index].data.axisMapping.fCurrent;
	}
	return 0.0f;
}

bool In_GetButtonValue(InputContext* context, struct ButtonBinding binding)
{
	EASSERT(binding.index > -1);
	EASSERT(binding.type != UnknownButton);
	switch (binding.type)
	{
	case MouseButton:
		return context->buttonMappings.MouseButtonMappings.arr[binding.index].data.ButtonMapping.bCurrent;
	case KeyboardButton:
		return context->buttonMappings.KeyboardButtonMappings.arr[binding.index].data.ButtonMapping.bCurrent;
	case GamepadButton:
		return context->buttonMappings.GamepadMappings.arr[binding.index].data.ButtonMapping.bCurrent;
	case MouseScrollButton:
		return context->buttonMappings.MouseScrollButtonMappings.arr[binding.index].data.ButtonMapping.bCurrent;
	}
	return false;
}

float In_GetMouseAxisValue(InputContext* context, HMouseAxisBinding hBinding)
{
	return (float)context->axisMappings.Mouse.arr[hBinding].data.axisMapping.fCurrent;
}

bool In_GetMouseButtonValue(InputContext* context, HMouseButtonBinding hBinding)
{
	return context->buttonMappings.MouseButtonMappings.arr[hBinding].data.ButtonMapping.bCurrent;
}

static int Shifted(int c, bool capslock)
{
	// todo: do this properly for other keys besides 0-9
	if(c >= STARDEW_KEY_0 && c <= STARDEW_KEY_9)
	{
		int LUT[10] =
		{
			')',
			'!',
			'"',
			'$',//'£', // £ causes warning, multi character
			'$',
			'%',
			'^',
			'&',
			'*',
			'(',
		};
		return LUT[c - STARDEW_KEY_0];
	}
	else if(c >= STARDEW_KEY_A && c <= STARDEW_KEY_Z)
	{
		if(capslock)
		{
			return tolower(c);	
		}
		else
		{
			return c;
		}
	}
	else
	{
		return c;
	}
	
}

static void DoTextInput(InputContext* context, int key, int scancode, int action, int mods)
{

	int ascii = 0;

	if(key == STARDEW_KEY_CAPS_LOCK)
	{
		switch(action)
		{
		case STARDEW_PRESS:
			EASSERT(!context->textInput.capslockModifier);
			context->textInput.capslockModifier = true;
			break;
		case STARDEW_RELEASE:
			EASSERT(context->textInput.capslockModifier);
			context->textInput.capslockModifier = false;
			break;
		}
	}
	else if(key == STARDEW_KEY_LEFT_SHIFT)
	{
		switch(action)
		{
		case STARDEW_PRESS:
			EASSERT(!context->textInput.shiftModifier);
			context->textInput.shiftModifier = true;
			break;
		case STARDEW_RELEASE:
			EASSERT(context->textInput.shiftModifier);
			context->textInput.shiftModifier = false;
			break;
		}
	}
	else if (action == STARDEW_PRESS)
	{
		if(context->textInput.shiftModifier)
		{
			ascii = Shifted((char)key, context->textInput.capslockModifier);
		}
		else if(!context->textInput.capslockModifier && (key >= STARDEW_KEY_A && key <= STARDEW_KEY_Z))
		{
			ascii = tolower(key);
		}
		else
		{
			ascii = key;
		}
		context->textInput.keystrokes[context->textInput.nKeystrokesThisFrame++] = ascii;
	}
	
}

void In_RecieveKeyboardKey(InputContext* context, int key, int scancode, int action, int mods)
{
	DoTextInput(context, key, scancode, action, mods);
	for (int i = 0; i < context->buttonMappings.KeyboardButtonMappings.size; i++)
	{
		InputMapping* pMapping = &context->buttonMappings.KeyboardButtonMappings.arr[i];
		if (context->buttonMappings.KeyboardButtonMappings.ActiveMask & (1 << i))
		{
			if (pMapping->data.ButtonMapping.data.keyboard.keyboadCode == key)
			{
				if (action == STARDEW_PRESS)
				{
					pMapping->data.ButtonMapping.bCurrent = true;
					pMapping->data.ButtonMapping.bPressThisFrame = true;
				}
				else if (action == STARDEW_RELEASE)
				{
					pMapping->data.ButtonMapping.bCurrent = false;
					pMapping->data.ButtonMapping.bReleaseThisFrame = true;
				}
			}
		}
	}
}

void In_RecieveMouseMove(InputContext* context, double xposIn, double yposIn)
{
	for (int i = 0; i < context->axisMappings.Mouse.size; i++)
	{
		InputMapping* pMapping = &context->axisMappings.Mouse.arr[i];
		if (context->axisMappings.Mouse.ActiveMask & (1 << i))
		{
			if (pMapping->data.axisMapping.data.mouse.axis == Axis_X)
			{
				pMapping->data.axisMapping.fCurrent = xposIn;
			}
			else if (pMapping->data.axisMapping.data.mouse.axis == Axis_Y)
			{
				pMapping->data.axisMapping.fCurrent = yposIn;
			}
		}
	}
}

void In_RecieveMouseButton(InputContext* context, int button, int action, int mods)
{
	for (int i = 0; i < context->buttonMappings.MouseButtonMappings.size; i++)
	{
		InputMapping* pMapping = &context->buttonMappings.MouseButtonMappings.arr[i];
		if (context->buttonMappings.MouseButtonMappings.ActiveMask & (1 << i))
		{
			if (pMapping->data.ButtonMapping.data.mouseBtn.button == button)
			{
				if (action == STARDEW_PRESS)
				{
					static int dCounter = 0;
					pMapping->data.ButtonMapping.bCurrent = true;
					pMapping->data.ButtonMapping.bPressThisFrame = true;
				}
				else if (action == STARDEW_RELEASE)
				{
					pMapping->data.ButtonMapping.bCurrent = false;
					pMapping->data.ButtonMapping.bReleaseThisFrame = true;
				}
			}
		}
	}
}

void In_FramebufferResize(InputContext* context, int width, int height)
{
	context->screenH = height;
	context->screenW = width;
}

void In_RecieveScroll(InputContext* context, double xoffset, double yoffset)
{
	for (int i = 0; i < context->axisMappings.MouseScroll.size; i++)
	{
		InputMapping* pMapping = &context->axisMappings.MouseScroll.arr[i];
		if (context->axisMappings.MouseScroll.ActiveMask & (1 << i))
		{
			if (pMapping->data.axisMapping.data.mouseScroll.axis == Axis_X)
			{
				pMapping->data.axisMapping.fCurrent = xoffset;
			}
			else if (pMapping->data.axisMapping.data.mouseScroll.axis == Axis_Y)
			{
				pMapping->data.axisMapping.fCurrent = yoffset;
			}
		}
	}
	for (int i = 0; i < context->buttonMappings.MouseScrollButtonMappings.size; i++)
	{
		InputMapping* pMapping = &context->buttonMappings.MouseScrollButtonMappings.arr[i];
		if (context->buttonMappings.MouseScrollButtonMappings.ActiveMask & (1 << i))
		{
			switch (pMapping->data.ButtonMapping.data.mouseScrollButton.axis)
			{
			case Axis_X:
				if (pMapping->data.ButtonMapping.data.mouseScrollButton.dir == Axis_Pos && xoffset > 0)
				{
					pMapping->data.ButtonMapping.bCurrent = true;
					pMapping->data.ButtonMapping.bPressThisFrame = true;
					pMapping->data.ButtonMapping.bReleaseThisFrame = true;
				}
				if (pMapping->data.ButtonMapping.data.mouseScrollButton.dir == Axis_Neg && xoffset < 0)
				{
					pMapping->data.ButtonMapping.bCurrent = true;
					pMapping->data.ButtonMapping.bPressThisFrame = true;
					pMapping->data.ButtonMapping.bReleaseThisFrame = true;
				}
				break;
			case Axis_Y:
				if (pMapping->data.ButtonMapping.data.mouseScrollButton.dir == Axis_Pos && yoffset > 0)
				{
					pMapping->data.ButtonMapping.bCurrent = true;
					pMapping->data.ButtonMapping.bPressThisFrame = true;
					pMapping->data.ButtonMapping.bReleaseThisFrame = true;
				}
				if (pMapping->data.ButtonMapping.data.mouseScrollButton.dir == Axis_Neg && yoffset < 0)
				{
					pMapping->data.ButtonMapping.bCurrent = true;
					pMapping->data.ButtonMapping.bPressThisFrame = true;
					pMapping->data.ButtonMapping.bReleaseThisFrame = true;
				}
				break;
			}
		}
	}
}

void In_SetControllerPresent(int controllerNo)
{
	gJoystick = controllerNo;
}

void In_EndFrame(InputContext* context)
{
	context->textInput.nKeystrokesThisFrame = 0;

	/*
		Reset these - mappings where the mouse scroll wheel is behaving as a button.
		Unlike other buttons this one isn't pressed and released, its an instantaneous event that should be handled once
	*/
	for (int i = 0; i < context->buttonMappings.MouseScrollButtonMappings.size; i++)
	{
		InputMapping* pMapping = &context->buttonMappings.MouseScrollButtonMappings.arr[i];
		pMapping->data.ButtonMapping.bCurrent = false;
		pMapping->data.ButtonMapping.bPressThisFrame = false;
		pMapping->data.ButtonMapping.bReleaseThisFrame = false;
	}

	for (int i = 0; i < context->buttonMappings.KeyboardButtonMappings.size; i++)
	{
		InputMapping* pMapping = &context->buttonMappings.KeyboardButtonMappings.arr[i];
		pMapping->data.ButtonMapping.bPressThisFrame = false;
		pMapping->data.ButtonMapping.bReleaseThisFrame = false;
	}

	for (int i = 0; i < context->buttonMappings.MouseButtonMappings.size; i++)
	{
		InputMapping* pMapping = &context->buttonMappings.MouseButtonMappings.arr[i];
		pMapping->data.ButtonMapping.bPressThisFrame = false;
		pMapping->data.ButtonMapping.bReleaseThisFrame = false;
	}

	for (int i = 0; i < context->buttonMappings.GamepadMappings.size; i++)
	{
		InputMapping* pMapping = &context->buttonMappings.GamepadMappings.arr[i];
		pMapping->data.ButtonMapping.bPressThisFrame = false;
		pMapping->data.ButtonMapping.bReleaseThisFrame = false;
	}

}

typedef void(*SetButtonCodeCallback)(InputMapping*, int);

static void AddChildButtonStructs(cJSON* parent, InputMappingArray* outMappings, SetButtonCodeCallback callback, ButtonSubType btnSubType)
{
	while (parent)
	{
		const char* name = parent->child->string;
		int val = -1;
		if(parent->child->type == cJSON_Number)
		{
			val = parent->child->valueint;
		}
		else if(parent->child->type == cJSON_String)
		{
			int* pVal = NULL;
			pVal = HashmapSearch(&gGLFWCodeKeyboardNameLUT, parent->child->valuestring);
			if(!pVal)
			{
				Log_Error("Invalid button value: '%s'", parent->child->valuestring);
				continue;
			}
			else
			{
				val = *pVal;
			}
		}
		
		InputMapping mapping;
		size_t  allocSize = strlen(name) + 1;
		mapping.name = malloc(allocSize);
		memset(mapping.name, 0, allocSize);
		strcpy(mapping.name, name);
		mapping.type = Button;
		mapping.data.ButtonMapping.type = btnSubType;
		mapping.data.ButtonMapping.bCurrent = false;
		mapping.data.ButtonMapping.bPressThisFrame = false;
		mapping.data.ButtonMapping.bReleaseThisFrame = false;

		callback(&mapping, val);
		outMappings->arr[outMappings->size++] = mapping;
		parent = parent->next;
	}
}

static void SetKeyboardCode(InputMapping* mapping, int code) { mapping->data.ButtonMapping.data.keyboard.keyboadCode = code; }
static void SetMouseBtnCode(InputMapping* mapping, int code) { mapping->data.ButtonMapping.data.mouseBtn.button = code; }
static void SetGamepadBtnCode(InputMapping* mapping, int code) { mapping->data.ButtonMapping.data.gamepadBtn.button = code; }

static void SetMouseScrollBtnCode(InputMapping* mapping, int code) 
{
	/*
		X Axis + : 3
		X Axis - : 1
		Y Axis + : 2
		Y Axis - : 0
	*/
	if (code & 1)
	{
		mapping->data.ButtonMapping.data.mouseScrollButton.axis = Axis_X;
	}
	else
	{
		mapping->data.ButtonMapping.data.mouseScrollButton.axis = Axis_Y;
	}

	if (code & 2)
	{
		mapping->data.ButtonMapping.data.mouseScrollButton.dir = Axis_Pos;
	}
	else
	{
		mapping->data.ButtonMapping.data.mouseScrollButton.dir = Axis_Neg;
	}
}

static void AddMouseAxisMappingsStructs(cJSON* parent, InputContext* ctx)
{
	while (parent)
	{
		InputMapping mapping;
		memset(&mapping, 0, sizeof(InputMapping));

		cJSON* child = parent->child;
		const char* name = child->string;
		mapping.name = malloc(strlen(name) + 1);
		strcpy(mapping.name, name);
		mapping.type = Axis;
		//strcpy(mapping.name, name);
		mapping.data.axisMapping.type = MouseAxis;
		cJSON* props = child->child;
		while (props)
		{
			if (strcmp("axis", props->string) == 0)
			{
				if (strcmp("x", props->valuestring) == 0)
				{
					mapping.data.axisMapping.data.mouse.axis = Axis_X;
				}
				else if (strcmp("y", props->valuestring) == 0)
				{
					mapping.data.axisMapping.data.mouse.axis = Axis_Y;
				}
			}
			props = props->next;
		}
		parent = parent->next;
		ctx->axisMappings.Mouse.arr[ctx->axisMappings.Mouse.size++] = mapping;
	}
}

static void AddScrollAxisMappingsStructs(cJSON* parent, InputContext* ctx)
{
	while (parent)
	{
		InputMapping mapping;
		memset(&mapping, 0, sizeof(InputMapping));

		cJSON* child = parent->child;
		const char* name = child->string;
		mapping.name = malloc(strlen(name) + 1);
		strcpy(mapping.name, name);
		mapping.type = Axis;
		//strcpy(mapping.name, name);
		mapping.data.axisMapping.type = MouseScrollAxis;
		cJSON* props = child->child;
		while (props)
		{
			if (strcmp("axis", props->string) == 0)
			{
				if (strcmp("x", props->valuestring) == 0)
				{
					mapping.data.axisMapping.data.mouseScroll.axis = Axis_X;
				}
				else if (strcmp("y", props->valuestring) == 0)
				{
					mapping.data.axisMapping.data.mouseScroll.axis = Axis_Y;
				}
			}
			props = props->next;
		}
		parent = parent->next;
		ctx->axisMappings.MouseScroll.arr[ctx->axisMappings.MouseScroll.size++] = mapping;
	}
}

static void AddGamepadAxisMappingsStructs(cJSON* parent, InputContext* ctx)
{
	while (parent)
	{
		InputMapping mapping;
		memset(&mapping, 0, sizeof(InputMapping));

		cJSON* child = parent->child;
		const char* name = child->string;
		mapping.name = malloc(strlen(name) + 1);
		strcpy(mapping.name, name);
		mapping.type = Axis;
		//strcpy(mapping.name, name);
		mapping.data.axisMapping.type = GamePadAxis;
		cJSON* props = child->child;
		while (props)
		{
			if (strcmp("axis", props->string) == 0)
			{
				if (strcmp("x", props->valuestring) == 0)
				{
					mapping.data.axisMapping.data.controller.axis = Axis_X;
				}
				else if (strcmp("y", props->valuestring) == 0)
				{
					mapping.data.axisMapping.data.controller.axis = Axis_Y;
				}
			}
			else if (strcmp("stick", props->string) == 0)
			{
				if (strcmp("lstick", props->valuestring) == 0)
				{
					mapping.data.axisMapping.data.controller.type = gpAxis_LStick;
				}
				else if (strcmp("rstick", props->valuestring) == 0)
				{
					mapping.data.axisMapping.data.controller.type = gpAxis_RStick;
				}
				else if (strcmp("lTrigger", props->valuestring) == 0)
				{
					mapping.data.axisMapping.data.controller.type = gpAxis_LT;
				}
				else if (strcmp("rTrigger", props->valuestring) == 0)
				{
					mapping.data.axisMapping.data.controller.type = gpAxis_RT;
				}

			}
			props = props->next;
		}
		parent = parent->next;
		ctx->axisMappings.Controller.arr[ctx->axisMappings.Controller.size++] = mapping;
	}
}

/// @brief Load a lookup hashmap from the name of a key as a string to a glfw keyboard code
void LoadGLFWKeyboardInputLUT()
{
	// generated by GenerateGLFWKeyboardHashmap.py
	HashmapInit(&gGLFWCodeKeyboardNameLUT, 256, sizeof(int));
	int v = - 1;
	v = -1;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Unknown", &v);
	v = 32;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Space", &v);
	v = 39;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Apostrophe", &v);
	v = 44;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Comma", &v);
	v = 45;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Minus", &v);
	v = 46;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Period", &v);
	v = 47;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Slash", &v);
	v = 48;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Num0", &v);
	v = 49;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Num1", &v);
	v = 50;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Num2", &v);
	v = 51;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Num3", &v);
	v = 52;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Num4", &v);
	v = 53;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Num5", &v);
	v = 54;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Num6", &v);
	v = 55;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Num7", &v);
	v = 56;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Num8", &v);
	v = 57;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Num9", &v);
	v = 59;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Semicolon", &v);
	v = 61;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Equal", &v);
	v = 65;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "A", &v);
	v = 66;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "B", &v);
	v = 67;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "C", &v);
	v = 68;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "D", &v);
	v = 69;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "E", &v);
	v = 70;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F", &v);
	v = 71;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "G", &v);
	v = 72;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "H", &v);
	v = 73;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "I", &v);
	v = 74;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "J", &v);
	v = 75;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "K", &v);
	v = 76;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "L", &v);
	v = 77;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "M", &v);
	v = 78;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "N", &v);
	v = 79;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "O", &v);
	v = 80;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "P", &v);
	v = 81;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Q", &v);
	v = 82;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "R", &v);
	v = 83;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "S", &v);
	v = 84;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "T", &v);
	v = 85;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "U", &v);
	v = 86;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "V", &v);
	v = 87;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "W", &v);
	v = 88;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "X", &v);
	v = 89;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Y", &v);
	v = 90;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Z", &v);
	v = 91;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "LeftBracket", &v);
	v = 92;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Backslash", &v);
	v = 93;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "RightBracket", &v);
	v = 96;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "GraveAccent", &v);
	v = 161;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "World1", &v);
	v = 162;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "World2", &v);
	v = 256;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Escape", &v);
	v = 257;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Enter", &v);
	v = 258;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Tab", &v);
	v = 259;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Backspace", &v);
	v = 260;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Insert", &v);
	v = 261;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Delete", &v);
	v = 262;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Right", &v);
	v = 263;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Left", &v);
	v = 264;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Down", &v);
	v = 265;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Up", &v);
	v = 266;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "PageUp", &v);
	v = 267;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "PageDown", &v);
	v = 268;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Home", &v);
	v = 269;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "End", &v);
	v = 280;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "CapsLock", &v);
	v = 281;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "ScrollLock", &v);
	v = 282;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "NumLock", &v);
	v = 283;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "PrintScreen", &v);
	v = 284;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Pause", &v);
	v = 290;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F1", &v);
	v = 291;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F2", &v);
	v = 292;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F3", &v);
	v = 293;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F4", &v);
	v = 294;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F5", &v);
	v = 295;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F6", &v);
	v = 296;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F7", &v);
	v = 297;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F8", &v);
	v = 298;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F9", &v);
	v = 299;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F10", &v);
	v = 300;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F11", &v);
	v = 301;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F12", &v);
	v = 302;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F13", &v);
	v = 303;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F14", &v);
	v = 304;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F15", &v);
	v = 305;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F16", &v);
	v = 306;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F17", &v);
	v = 307;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F18", &v);
	v = 308;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F19", &v);
	v = 309;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F20", &v);
	v = 310;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F21", &v);
	v = 311;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F22", &v);
	v = 312;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F23", &v);
	v = 313;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F24", &v);
	v = 314;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "F25", &v);
	v = 320;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Keypad0", &v);
	v = 321;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Keypad1", &v);
	v = 322;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Keypad2", &v);
	v = 323;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Keypad3", &v);
	v = 324;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Keypad4", &v);
	v = 325;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Keypad5", &v);
	v = 326;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Keypad6", &v);
	v = 327;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Keypad7", &v);
	v = 328;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Keypad8", &v);
	v = 329;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Keypad9", &v);
	v = 330;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "KeypadDecimal", &v);
	v = 331;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "KeypadDivide", &v);
	v = 332;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "KeypadMultiply", &v);
	v = 333;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "KeypadSubtract", &v);
	v = 334;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "KeypadAdd", &v);
	v = 335;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "KeypadEnter", &v);
	v = 336;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "KeypadEqual", &v);
	v = 340;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "LeftShift", &v);
	v = 341;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "LeftControl", &v);
	v = 342;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "LeftAlt", &v);
	v = 343;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "LeftSuper", &v);
	v = 344;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "RightShift", &v);
	v = 345;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "RightControl", &v);
	v = 346;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "RightAlt", &v);
	v = 347;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "RightSuper", &v);
	v = 348;
	HashmapInsert(&gGLFWCodeKeyboardNameLUT, "Menu", &v);
}

InputContext In_InitInputContext()
{
	LoadGLFWKeyboardInputLUT();
#define ERROR(test, str) if(!test) {Log_Error("In_InitInputContext %s", str); InputContext v; memset(&v, 0, sizeof(InputContext)); return v;} 
	InputContext ctx;
	memset(&ctx, 0, sizeof(InputContext));

	char buf[256];
	cwk_path_join(gCmdArgs.configDir, "Keymap.json", buf, 256);

	int size = 0;
	char* data = LoadFile(buf, &size);
	ERROR(data, "can't load data");
	cJSON* json = cJSON_ParseWithLength(data, size);
	ERROR(json, "can't parse json");
	cJSON* buttons = cJSON_GetObjectItem(json, "Buttons");
	ERROR(buttons, "json child 'Buttons' not found.");


	cJSON* keyboard = cJSON_GetObjectItem(buttons, "Keyboard");
	ERROR(keyboard, "json child 'Buttons.Keyboard' not found.");
	cJSON* keyboardKeyMap = keyboard->child;
	AddChildButtonStructs(keyboardKeyMap, &ctx.buttonMappings.KeyboardButtonMappings, &SetKeyboardCode, KeyboardButton);

	cJSON* mouseBtn = cJSON_GetObjectItem(buttons, "Mouse");
	ERROR(mouseBtn, "mouseBtn");
	cJSON* mouseBtnMap = mouseBtn->child;
	AddChildButtonStructs(mouseBtnMap, &ctx.buttonMappings.MouseButtonMappings, &SetMouseBtnCode, MouseButton);

	cJSON* mouseScrollBtn = cJSON_GetObjectItem(buttons, "MouseScroll");
	ERROR(mouseScrollBtn, "MouseScroll");
	cJSON* mouseScrollBtnMap = mouseScrollBtn->child;
	AddChildButtonStructs(mouseScrollBtnMap, &ctx.buttonMappings.MouseScrollButtonMappings, &SetMouseScrollBtnCode, MouseScrollButton);
	
	cJSON* gamepadBtn = cJSON_GetObjectItem(buttons, "GamePad");
	ERROR(gamepadBtn, "GamePad");
	cJSON* gamepadBtnMap = gamepadBtn->child;
	AddChildButtonStructs(gamepadBtnMap, &ctx.buttonMappings.GamepadMappings, &SetGamepadBtnCode, GamepadButton);

	cJSON* axes = cJSON_GetObjectItem(json, "Axes");
	ERROR(axes, "json child 'Axes' not found.");

	cJSON* mouseAxes = cJSON_GetObjectItem(axes, "Mouse");
	ERROR(mouseAxes, "json child 'Axes.Mouse' not found.");
	cJSON* parent = mouseAxes->child;
	AddMouseAxisMappingsStructs(parent, &ctx);

	cJSON* gamepadAxis = cJSON_GetObjectItem(axes, "GamePad");
	ERROR(gamepadAxis, "json child 'Axes.GamePad' not found.");
	parent = gamepadAxis->child;
	AddGamepadAxisMappingsStructs(parent, &ctx);

	cJSON* mouseScroll = cJSON_GetObjectItem(axes, "MouseScroll");
	ERROR(mouseScroll, "json child 'Axes.MouseScroll' not found.");
	parent = mouseScroll->child;
	AddScrollAxisMappingsStructs(parent, &ctx);

	// HACK BEGIN - to be done some other better way in future
	ctx.axisMappings.Mouse.ActiveMask |= 1;
	ctx.axisMappings.Mouse.ActiveMask |= 2;
	ctx.buttonMappings.MouseButtonMappings.ActiveMask |= 3;
	// HACK END

	cJSON_Delete(json);
	free(data);
	return ctx;

#undef ERROR
}


void In_GetMask(struct ActiveInputBindingsMask* pOutMask, InputContext* pCtx)
{
	pOutMask->MouseButtonMappings = pCtx->buttonMappings.MouseButtonMappings.ActiveMask;
	pOutMask->KeyboardButtonMappings = pCtx->buttonMappings.KeyboardButtonMappings.ActiveMask;
	pOutMask->GamepadButtonMappings = pCtx->buttonMappings.GamepadMappings.ActiveMask;
	pOutMask->MouseScrollButtonMappings = pCtx->buttonMappings.MouseScrollButtonMappings.ActiveMask;

	pOutMask->MouseAxisMappings = pCtx->axisMappings.Mouse.ActiveMask;
	pOutMask->ControllerAxisMappings = pCtx->axisMappings.Controller.ActiveMask;
	pOutMask->MouseScrollAxisMappings = pCtx->axisMappings.MouseScroll.ActiveMask;
}

void In_SetMask(struct ActiveInputBindingsMask* mask, InputContext* pCtx)
{
	pCtx->buttonMappings.MouseButtonMappings.ActiveMask = mask->MouseButtonMappings;
	pCtx->buttonMappings.KeyboardButtonMappings.ActiveMask = mask->KeyboardButtonMappings;
	pCtx->buttonMappings.GamepadMappings.ActiveMask = mask->GamepadButtonMappings;
	pCtx->buttonMappings.MouseScrollButtonMappings.ActiveMask = mask->MouseScrollButtonMappings;

	pCtx->axisMappings.Mouse.ActiveMask = mask->MouseAxisMappings;
	pCtx->axisMappings.Controller.ActiveMask = mask->ControllerAxisMappings;
	pCtx->axisMappings.MouseScroll.ActiveMask = mask->MouseScrollAxisMappings;
}

void In_ActivateButtonBinding(struct ButtonBinding binding, struct ActiveInputBindingsMask* pMask)
{
	EASSERT(binding.index <= 63);
	switch (binding.type)
	{
	case MouseButton:
		pMask->MouseButtonMappings |= (1 << binding.index);
		break;
	case KeyboardButton:
		pMask->KeyboardButtonMappings |= (1 << binding.index);
		break;
	case GamepadButton:
		pMask->GamepadButtonMappings |= (1 << binding.index);
		break;
	case MouseScrollButton:
		pMask->MouseScrollButtonMappings |= (1 << binding.index);
		break;
	}
}

void In_ActivateAxisBinding(struct AxisBinding binding, struct ActiveInputBindingsMask* pMask)
{
	EASSERT(binding.index <= 63);
	switch (binding.type)
	{
	case MouseAxis:
		pMask->MouseAxisMappings |= (1 << binding.index);
		break;
	case GamePadAxis:
		pMask->ControllerAxisMappings |= (1 << binding.index);
		break;
	case MouseScrollAxis:
		pMask->MouseScrollAxisMappings |= (1 << binding.index);
		break;
	}
}

void In_DeactivateButtonBinding(struct ButtonBinding binding, struct ActiveInputBindingsMask* pMask)
{
		EASSERT(binding.index <= 63);
	switch (binding.type)
	{
	case MouseButton:
		pMask->MouseButtonMappings &= ~(1 << binding.index);
		break;
	case KeyboardButton:
		pMask->KeyboardButtonMappings &= ~(1 << binding.index);
		break;
	case GamepadButton:
		pMask->GamepadButtonMappings &= ~(1 << binding.index);
		break;
	case MouseScrollButton:
		pMask->MouseScrollButtonMappings &= ~(1 << binding.index);
		break;
	}

}

void In_DeactivateAxisBinding(struct AxisBinding binding, struct ActiveInputBindingsMask* pMask)
{
	EASSERT(binding.index <= 63);
	switch (binding.type)
	{
	case MouseAxis:
		pMask->MouseAxisMappings &= ~(1 << binding.index);
		break;
	case GamePadAxis:
		pMask->ControllerAxisMappings &= ~(1 << binding.index);
		break;
	case MouseScrollAxis:
		pMask->MouseScrollAxisMappings &= ~(1 << binding.index);
		break;
	}
}


bool In_GetButtonPressThisFrame(InputContext* context, struct ButtonBinding binding)
{
	EASSERT(binding.index > -1);
	EASSERT(binding.type != UnknownButton);
	switch (binding.type)
	{
	case MouseButton:
		return context->buttonMappings.MouseButtonMappings.arr[binding.index].data.ButtonMapping.bPressThisFrame;
	case KeyboardButton:
		return context->buttonMappings.KeyboardButtonMappings.arr[binding.index].data.ButtonMapping.bPressThisFrame;
	case GamepadButton:
		return context->buttonMappings.GamepadMappings.arr[binding.index].data.ButtonMapping.bPressThisFrame;
	case MouseScrollButton:
		return context->buttonMappings.MouseScrollButtonMappings.arr[binding.index].data.ButtonMapping.bPressThisFrame;
	}
	return false;
}

bool In_GetButtonReleaseThisFrame(InputContext* context, struct ButtonBinding binding)
{
	EASSERT(binding.index > -1);
	EASSERT(binding.type != UnknownButton);
	switch (binding.type)
	{
	case MouseButton:
		return context->buttonMappings.MouseButtonMappings.arr[binding.index].data.ButtonMapping.bReleaseThisFrame;
	case KeyboardButton:
		return context->buttonMappings.KeyboardButtonMappings.arr[binding.index].data.ButtonMapping.bReleaseThisFrame;
	case GamepadButton:
		return context->buttonMappings.GamepadMappings.arr[binding.index].data.ButtonMapping.bReleaseThisFrame;
	case MouseScrollButton:
		return context->buttonMappings.MouseScrollButtonMappings.arr[binding.index].data.ButtonMapping.bReleaseThisFrame;
	}
	return false;
}

