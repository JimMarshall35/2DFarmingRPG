#ifndef ASSERT_LIB_H
#define ASSERT_LIB_H

#include <assert.h>

#define EASSERT(e) assert(e)
#ifdef _DEBUG
#define EVERIFY(e) EASSERT(e)	
#else
#define EVERIFY(e) e
#endif


#endif