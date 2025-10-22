#include "WfTree.h"
#include "BinarySerializer.h"
#include "Entities.h"
#include "Game2DLayer.h"
#include "Components.h"
#include "Atlas.h"

struct WfTreeEntityData
{
    struct WfTreeDef def;
    vec2 groundContactPoint;
};

static OBJECT_POOL(struct WfTreeEntityData) gTreeDataObjectPool;

void WfTreeInit()
{
    gTreeDataObjectPool = NEW_OBJECT_POOL(struct WfTreeEntityData, 512);
}

void WfDeSerializeTreeEntity(struct BinarySerializer* bs, struct Entity2D* pOutEnt, struct GameLayer2DData* pData)
{
    
    struct WfTreeSprites sprites;
    WfGetTreeSprites(&sprites, pData->hAtlas);
    u32 version = 0;
    BS_DeSerializeU32(&version, bs); // version
    switch (version)
    {
    case 1:
        /* code */
        {
            struct WfTreeEntityData entData;
            BS_DeSerializeI32((i32*)&entData.def.season, bs);
            BS_DeSerializeI32((i32*)&entData.def.type, bs);
            BS_DeSerializeI32((i32*)&entData.def.subtype, bs);
            BS_DeSerializeFloat(&entData.groundContactPoint[0], bs);
            BS_DeSerializeFloat(&entData.groundContactPoint[1], bs);
            // HEntity2D hTree = WfAddTreeBasedAt(entData.groundContactPoint[0], entData.groundContactPoint[1], &entData.def, &sprites, &pData->entities);
            // struct Entity2D* pTree = Et2D_GetEntity(&pData->entities, hTree);
            // pTree->init(pTree, pData->pLayer);
        }
        break;
    
    default:
        break;
    }
}

void WfSerializeTreeEntity(struct BinarySerializer* bs, struct Entity2D* pInEnt, struct GameLayer2DData* pData)
{
    struct WfTreeEntityData* pEntData = &gTreeDataObjectPool[pInEnt->user.hData];
    BS_SerializeU32(1, bs); // version
    BS_SerializeI32((i32)pEntData->def.season, bs);
    BS_SerializeI32((i32)pEntData->def.type, bs);
    BS_SerializeI32((i32)pEntData->def.subtype, bs);
    BS_SerializeFloat(pEntData->groundContactPoint[0], bs);
    BS_SerializeFloat(pEntData->groundContactPoint[1], bs);
}

static void TreeOnDestroy(struct Entity2D* pEnt, struct GameFrameworkLayer* pData)
{
    FreeObjectPoolIndex(gTreeDataObjectPool, pEnt->user.hData);
    Entity2DOnDestroy(pEnt, pData);
}

static float TreeGetPreDrawSortValue(struct Entity2D* pEnt)
{
    struct WfTreeEntityData* pData = &gTreeDataObjectPool[pEnt->user.hData];
    return pData->groundContactPoint[1];
}



HEntity2D WfAddTreeBasedAt(float x, float y, struct WfTreeDef* def, struct WfTreeSprites* spritesPerSeason, struct Entity2DCollection* pEntityCollection)
{
    struct Entity2D ent;
    memset(&ent, 0, sizeof(struct Entity2D));
    const float trunkOffsetPx = 64.0f; /* Y offset from the top of the tree top sprite to the top of the trunk sprite */
    const float spriteHeight = 96.0f;
    const float combinedTreeSpriteHeight = trunkOffsetPx + spriteHeight;
    const float combinedSpriteWidth = 96.0f;
    const float bottomOfTrunkSpriteToBase = 34.0f;

    // xPos and YPos are where the base of the tree is
    ent.transform.position[0] = x - combinedSpriteWidth / 2.0f; // center it
    ent.transform.position[1] = y - (combinedTreeSpriteHeight - bottomOfTrunkSpriteToBase);
    ent.transform.scale[0] = 1.0f;
    ent.transform.scale[1] = 1.0f;
    ent.transform.rotation = 0.0f;
    ent.bKeepInQuadtree = true;

    struct Component2D* pComponent1 = &ent.components[ent.numComponents++];
    struct Component2D* pComponent2 = &ent.components[ent.numComponents++];
    struct WfTreeSprites* pFoundSeason = &spritesPerSeason[def->season];
    hSprite topSprite = NULL_HANDLE;
    hSprite trunkSprite = NULL_HANDLE;
    switch (def->type)
    {
    case Coniferous:
        topSprite = def->subtype == 0 ? pFoundSeason->coniferousTop1 : pFoundSeason->coniferousTop2;
        trunkSprite = pFoundSeason->trunk2;
        break;
    case Deciduous:
        topSprite = def->subtype == 0 ? pFoundSeason->deciduousTop1 : pFoundSeason->deciduousTop2;
        trunkSprite = pFoundSeason->trunk1;
        break;
    default:
        break;
    }

    /* order is important as we want the tree trunk to be drawn first and the top on top of that */
    pComponent2->type = ETE_Sprite;
    pComponent2->data.sprite.sprite = topSprite;
    memset(&pComponent2->data.sprite.transform, 0, sizeof(struct Transform2D));
    pComponent2->data.sprite.transform.scale[0] = 1.0f;
    pComponent2->data.sprite.transform.scale[1] = 1.0f;

    pComponent1->type = ETE_Sprite;
    pComponent1->data.sprite.sprite = trunkSprite;
    memset(&pComponent1->data.sprite.transform, 0, sizeof(struct Transform2D));
    pComponent1->data.sprite.transform.position[1] = trunkOffsetPx;
    pComponent1->data.sprite.transform.scale[0] = 1.0f;
    pComponent1->data.sprite.transform.scale[1] = 1.0f;

    HGeneric hTreeData = NULL_HANDLE;
    gTreeDataObjectPool = GetObjectPoolIndex(gTreeDataObjectPool, &hTreeData);
    gTreeDataObjectPool[hTreeData].def = *def;
    gTreeDataObjectPool[hTreeData].groundContactPoint[0] = x;
    gTreeDataObjectPool[hTreeData].groundContactPoint[1] = y;
    ent.user.hData = hTreeData;
    Et2D_PopulateCommonHandlers(&ent);
    ent.onDestroy = &TreeOnDestroy;
    ent.getSortPos = &TreeGetPreDrawSortValue;

    return Et2D_AddEntity(pEntityCollection, &ent);
}

void WfGetTreeSprites(struct WfTreeSprites* spritesPerSeason, hAtlas atlas)
{
    
    spritesPerSeason[Spring].coniferousTop1 = At_FindSprite("conif_tree_sum_top_1", atlas);
    spritesPerSeason[Spring].coniferousTop2 = At_FindSprite("conif_tree_sum_top_2", atlas);
    spritesPerSeason[Spring].deciduousTop1 = At_FindSprite("decid_tree_sum_top_1", atlas);
    spritesPerSeason[Spring].deciduousTop2 = At_FindSprite("decid_tree_sum_top_2", atlas);
    spritesPerSeason[Spring].trunk1 = At_FindSprite("tree_trunk_sum_1", atlas);
    spritesPerSeason[Spring].trunk2 = At_FindSprite("tree_trunk_sum_2", atlas);

    spritesPerSeason[Summer].coniferousTop1 = At_FindSprite("conif_tree_sum_top_1", atlas);
    spritesPerSeason[Summer].coniferousTop2 = At_FindSprite("conif_tree_sum_top_2", atlas);
    spritesPerSeason[Summer].deciduousTop1 = At_FindSprite("decid_tree_sum_top_1", atlas);
    spritesPerSeason[Summer].deciduousTop2 = At_FindSprite("decid_tree_sum_top_2", atlas);
    spritesPerSeason[Summer].trunk1 = At_FindSprite("tree_trunk_sum_1", atlas);
    spritesPerSeason[Summer].trunk2 = At_FindSprite("tree_trunk_sum_2", atlas);

    spritesPerSeason[Autumn].coniferousTop1 = At_FindSprite("conif_tree_aut_top_1", atlas);
    spritesPerSeason[Autumn].coniferousTop2 = At_FindSprite("conif_tree_aut_top_2", atlas);
    spritesPerSeason[Autumn].deciduousTop1 = At_FindSprite("decid_tree_aut_top_1", atlas);
    spritesPerSeason[Autumn].deciduousTop2 = At_FindSprite("decid_tree_aut_top_2", atlas);
    spritesPerSeason[Autumn].trunk1 = At_FindSprite("tree_trunk_sum_1", atlas);
    spritesPerSeason[Autumn].trunk2 = At_FindSprite("tree_trunk_sum_2", atlas);

    spritesPerSeason[Autumn].coniferousTop1 = At_FindSprite("conif_tree_wint_top_1", atlas);
    spritesPerSeason[Autumn].coniferousTop2 = At_FindSprite("conif_tree_wint_top_2", atlas);
    spritesPerSeason[Autumn].deciduousTop1 = At_FindSprite("decid_tree_wint_top_1", atlas);
    spritesPerSeason[Autumn].deciduousTop2 = At_FindSprite("decid_tree_wint_top_2", atlas);
    spritesPerSeason[Autumn].trunk1 = At_FindSprite("tree_trunk_sum_1", atlas);
    spritesPerSeason[Autumn].trunk2 = At_FindSprite("tree_trunk_sum_2", atlas);

}
