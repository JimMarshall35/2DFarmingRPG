#include "AnimatedSprite.h"
#include "Entities.h"
#include "GameFramework.h"
#include "Game2DLayer.h"
#include "DynArray.h"
#include "Atlas.h"
#include "AssertLib.h"

void AnimatedSprite_OnInit(struct AnimatedSprite* pAnimatedSprite, struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{
    struct GameLayer2DData* pData = pLayer->userData;
    struct AtlasAnimation* pAnim = At_FindAnim(pData->hAtlas, pAnimatedSprite->animationName);
    EASSERT(pAnim);
    pAnimatedSprite->pSprites = pAnim->frames;
    pAnimatedSprite->numSprites = VectorSize(pAnim->frames);
    pAnimatedSprite->fps = pAnim->fps;

}

void AnimatedSprite_OnDestroy(struct Entity2D* pEnt)
{

}

void AnimatedSprite_OnUpdate(struct AnimatedSprite* pAnimatedSprite, struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, float deltaT)
{
    if(pAnimatedSprite->bIsAnimating)
    {
        pAnimatedSprite->timer += deltaT;
        if(pAnimatedSprite->timer >= 1.0f / pAnimatedSprite->fps)
        {
            pAnimatedSprite->onSprite++;
            if(pAnimatedSprite->onSprite == pAnimatedSprite->numSprites)
            {
                if(pAnimatedSprite->bRepeat)
                {
                    pAnimatedSprite->onSprite = 0;
                }
                else
                {
                    pAnimatedSprite->onSprite = pAnimatedSprite->numSprites - 1;
                    pAnimatedSprite->bIsAnimating = false;
                }
            }
            pAnimatedSprite->timer = 0.0f;
        }
    }
}

void AnimatedSprite_GetBoundingBox(struct Entity2D* pEnt, struct AnimatedSprite* pAnimatedSprite, struct GameFrameworkLayer* pLayer, vec2 outTL, vec2 outBR)
{
    struct GameLayer2DData* pLayerData = pLayer->userData;
    AtlasSprite* pSprite = At_GetSprite(pAnimatedSprite->pSprites[pAnimatedSprite->onSprite], pLayerData->hAtlas);
    vec2 tl = {pEnt->transform.position[0], pEnt->transform.position[1]};
    vec2 br;
    vec2 size = {
        pSprite->widthPx  * pEnt->transform.scale[0] * pAnimatedSprite->transform.scale[0],
        pSprite->heightPx * pEnt->transform.scale[1] * pAnimatedSprite->transform.scale[1]
    };
    glm_vec2_add(pEnt->transform.position, size, br);
    glm_vec2_add(tl, pAnimatedSprite->transform.position, tl);
    glm_vec2_add(br, pAnimatedSprite->transform.position, br);

	outTL[0] = tl[0];
	outTL[1] = tl[1];
	outBR[0] = br[0];
	outBR[1] = br[1];

}

void AnimatedSprite_Draw(struct AnimatedSprite* pSpriteComp, struct Entity2D* pEnt, struct GameFrameworkLayer* pLayer, struct Transform2D* pCam, VECTOR(Worldspace2DVert)* outVerts, VECTOR(VertIndexT)* outIndices, VertIndexT* pNextIndex)
{
    vec2 tl, br;
    AnimatedSprite_GetBoundingBox(pEnt, pSpriteComp, pLayer, tl, br);
    struct GameLayer2DData* pLayerData = pLayer->userData;
    AtlasSprite* pSprite = At_GetSprite(pSpriteComp->pSprites[pSpriteComp->onSprite], pLayerData->hAtlas);
    OutputSpriteVerticesBase(pSprite, outVerts, outIndices, pNextIndex, tl, br);
}