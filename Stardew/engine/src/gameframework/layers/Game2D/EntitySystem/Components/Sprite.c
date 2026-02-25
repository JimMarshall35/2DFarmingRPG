#include "Sprite.h"
#include "Entities.h"
#include "GameFramework.h"
#include "InputContext.h"
#include "DrawContext.h"
#include "Atlas.h"
#include "Game2DLayer.h"
#include "Game2DVertexOutputHelpers.h"

void SpriteComp_OnInit(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{

}

void SpriteComp_Update(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{
}

void SpriteComp_UpdatePostPhysics(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{
    
}

void SpriteComp_Draw(
    struct Sprite* pSpriteComp,
    struct Entity2D* pEnt,
    struct GameFrameworkLayer* pLayer,
    struct Transform2D* pCam,
    VECTOR(Worldspace2DVert)* outVerts,
    VECTOR(VertIndexT)* outIndices,
    VertIndexT* pNextIndex
)
{
    if(!pSpriteComp->bDraw)
        return;

    HSprite_DrawBase(pSpriteComp->sprite, &pSpriteComp->transform, pEnt, pLayer, outVerts, outIndices, pNextIndex);
}

void SpriteComp_Input(struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, InputContext* context)
{

}

void SpriteComp_OnDestroy(struct Entity2D* pEnt)
{

}

void SpriteComp_GetBoundingBox(struct Entity2D* pEnt, struct Sprite* pSpriteComp, struct GameFrameworkLayer* pLayer, vec2 outTL, vec2 outBR)
{
    if(!pSpriteComp->bDraw) return;
    struct GameLayer2DData* pLayerData = pLayer->userData;
    AtlasSprite* pSprite = At_GetSprite(pSpriteComp->sprite, pLayerData->hAtlas);
    vec2 tl = {pEnt->transform.position[0], pEnt->transform.position[1]};
    vec2 br;
    vec2 size = {
        pSprite->widthPx  * pEnt->transform.scale[0] * pSpriteComp->transform.scale[0],
        pSprite->heightPx * pEnt->transform.scale[1] * pSpriteComp->transform.scale[1]
    };
    glm_vec2_add(pEnt->transform.position, size, br);
    glm_vec2_add(tl, pSpriteComp->transform.position, tl);
    glm_vec2_add(br, pSpriteComp->transform.position, br);

	outTL[0] = tl[0];
	outTL[1] = tl[1];
	outBR[0] = br[0];
	outBR[1] = br[1];
}

