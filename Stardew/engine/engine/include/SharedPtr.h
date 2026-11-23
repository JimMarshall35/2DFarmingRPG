#ifndef  SHAREDPTR_H
#define SHAREDPTR_H


//called just before the memory is freed
#include <stddef.h>
#include "IntTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*SharedPtrDestuctorFn)(void* data);

void* Sptr_New(size_t size, SharedPtrDestuctorFn dtor);

void Sptr_AddRef(void* pointer);

void Sptr_RemoveRef(void* pointer);

i64 Sptr_GetRefCount(void* pointer);

#define SHARED_PTR(p) p*

#define SHARED_PTR_NEW(a, dtor) Sptr_New(sizeof(a), dtor)


#ifdef __cplusplus
}
#endif

#endif // ! SHAREDPTR_H
