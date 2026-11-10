#ifndef GAMECOMPONENTS_H
#define GAMECOMPONENTS_H

#include "Entities.h"
#include "components/WfHealthComponent.h"



#define COMPONENTS_UNION \
    COMPONENTS_UNION_BEGIN \
    ENGINE_BUILTIN_COMPONENTS \
    struct WfHealthComponent health;\
    COMPONENTS_UNION_END
    
#endif

