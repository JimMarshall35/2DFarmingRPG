#ifndef GAMEFRAMEWORKEVENT_H
#define GAMEFRAMEWORKEVENT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
	@file GameFrameworkEvent.h
	@brief system used for sending messages between game layers.
*/

struct GameFrameworkEventListener;

typedef void (*EventListenerFn)(void* pUserData, void* pEventData);

struct GameFrameworkEventListener* Ev_SubscribeEvent(char* eventName, EventListenerFn listenerFn, void* pUser);
bool Ev_UnsubscribeEvent(struct GameFrameworkEventListener* pListener);
void* Ev_GetUserData(struct GameFrameworkEventListener* pListener);
void Ev_FireEvent(char* eventName, void* eventArgs);
void Ev_Init();

#ifdef __cplusplus
}
#endif

#endif


