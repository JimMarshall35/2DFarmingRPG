#ifndef WFENUMS_H
#define WFENUMS_H

enum WfSeason
{
    Spring,
    Summer,
    Autumn,
    Winter,
    NumSeasons
};

enum WfDirection
{
    Up,
    Down,
    Left,
    Right,
    NumDirections
};


/// @brief an animation that plays and then finishes, returning back to the normal movement animations
enum WfActionAnimation
{
    WfNoActionAnim,
    WfSlashAnim,
    WfThrustAnim
};


#endif