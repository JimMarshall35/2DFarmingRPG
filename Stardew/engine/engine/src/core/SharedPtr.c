#include "SharedPtr.h"
#include <stdlib.h>
#include "IntTypes.h"
#include "PlatformDefs.h"

struct SharedPtrHeader
{
#if GAME_PTR_SIZE == 8
	SharedPtrDestuctorFn pDtor;
	i64 reference;
#elif GAME_PTR_SIZE == 4
	SharedPtrDestuctorFn pDtor;
	i64 reference;
	i32 padding;
#endif 

};

void* Sptr_New(size_t size, SharedPtrDestuctorFn dtor)
{
	struct SharedPtrHeader* ptr = malloc(size + sizeof(struct SharedPtrHeader));
	ptr->reference = 1;
	ptr->pDtor = dtor;
	return ptr + 1;;
}

void Sptr_AddRef(void* pointer)
{
	struct SharedPtrHeader* ptr = (struct SharedPtrHeader*)pointer - 1;
	ptr->reference++;
}

i64 Sptr_GetRefCount(void* pointer)
{
	struct SharedPtrHeader* ptr = (struct SharedPtrHeader*)pointer - 1;
	return ptr->reference;
}

void Sptr_RemoveRef(void* pointer)
{

	struct SharedPtrHeader* ptr = (struct SharedPtrHeader*)pointer - 1;
	ptr->reference--;
	if (ptr->reference <= 0)
	{
		if (ptr->pDtor)
		{
			ptr->pDtor(ptr + 1);
		}
		free(ptr);
	}
}
