#ifndef PROFILE_H
#define PROFILE_H

#ifdef TRACY_ENABLE
    #include "tracy/TracyC.h"
    #define ZONE(ctx)         TracyCZone(ctx, 1)
    #define ZONE_N(ctx, name) TracyCZoneN(ctx, name, 1)
    #define ZONE_END(ctx)     TracyCZoneEnd(ctx)
    #define FRAME_MARK        TracyCFrameMark
#else
    #define ZONE(ctx)
    #define ZONE_N(ctx, name)
    #define ZONE_END(ctx)
    #define FRAME_MARK
#endif

#endif