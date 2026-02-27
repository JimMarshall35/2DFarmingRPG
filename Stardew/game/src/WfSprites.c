#include "WfSprites.h"
#include "Game2DLayer.h"
#include "Atlas.h"

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


void WfInitSprites(struct WfSprites* pSprites, struct GameLayer2DData* pLayerData)
{
    WfGetTreeSprites(pSprites->treeSpritesPerSeason, pLayerData->hAtlas);
}