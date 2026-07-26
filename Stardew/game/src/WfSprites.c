#include "WfSprites.h"
#include "Game2DLayer.h"
#include "Atlas.h"

void WfGetTreeSprites(struct WfTreeSprites* sprites, hAtlas atlas)
{
    sprites->coniferousTop1 = At_FindSprite("conif_tree_sum_top_1", atlas);
    sprites->coniferousTop2 = At_FindSprite("conif_tree_sum_top_2", atlas);
    sprites->deciduousTop1 = At_FindSprite("decid_tree_sum_top_1", atlas);
    sprites->deciduousTop2 = At_FindSprite("decid_tree_sum_top_2", atlas);
    sprites->trunk1 = At_FindSprite("tree_trunk_sum_1", atlas);
    sprites->trunk2 = At_FindSprite("tree_trunk_sum_2", atlas);
    sprites->stump1 = At_FindSprite("tree_stump_sum_1", atlas);
    sprites->stump2 = At_FindSprite("tree_stump_sum_2", atlas);
}

static void GetDebrisSprites(struct WfDebrisSprites* sprites, hAtlas atlas)
{
    sprites->debrisWood = At_LookupNamedTile(atlas, "environment_stump");
    sprites->rock1      = At_LookupNamedTile(atlas, "rock1");
    sprites->rock2      = At_LookupNamedTile(atlas, "rock2");
}


void WfInitSprites(struct WfSprites* pSprites, struct GameLayer2DData* pLayerData)
{
    WfGetTreeSprites(&pSprites->treeSprites, pLayerData->hAtlas);
    GetDebrisSprites(&pSprites->debrisSprites, pLayerData->hAtlas);
}