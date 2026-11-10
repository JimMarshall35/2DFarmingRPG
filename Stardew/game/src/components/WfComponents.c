#include "components/WfComponents.h"
#include "Components.h"

void Game_InitComponents(struct Component2D* pComponent, struct Entity2D* entity, struct GameFrameworkLayer* pLayer)
{

}

void Game_UpdateComponents(struct Component2D* pComponent, struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{

}

void Game_Entity2DUpdatePostPhysicsFn(struct Component2D* pComponent, struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{

}

void Game_InputComponents(struct Component2D* pComponent, struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, InputContext* context)
{

}

void Game_DestroyComponents(struct Component2D* pComponent, struct Entity2D* entity, struct GameFrameworkLayer* pLayer)
{

}

void Game_DrawComponents(
    struct Component2D* pComponent, 
    struct Entity2D* entity, 
    struct GameFrameworkLayer* pLayer,
    struct Transform2D* pCam,
    VECTOR(Worldspace2DVert)* outVerts,
    VECTOR(VertIndexT)* outIndices,
    VertIndexT* pNextIndex)
{

}

void WfRegisterGameComponentHandlers()
{
    Co_RegisterGameComponentHandlers(
        &Game_InitComponents,
        &Game_UpdateComponents,
        &Game_Entity2DUpdatePostPhysicsFn,
        &Game_InputComponents,
        &Game_DestroyComponents,
        &Game_DrawComponents
    );
}