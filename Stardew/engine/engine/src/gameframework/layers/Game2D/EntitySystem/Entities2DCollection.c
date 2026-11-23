#include "Entity2DCollection.h"
#include "Profile.h"

HDynamicEntityListItem DynL_AddEntity(struct DynamicEnt2DList* pDynList, HEntity2D hEnt)
{
    ZONE(ctx)
    HDynamicEntityListItem hItem = NULL_HANDLE;
    pDynList->pDynamicListItemPool = GetObjectPoolIndex(pDynList->pDynamicListItemPool, &hItem);
    struct DynamicEntityListItem* pItem = &pDynList->pDynamicListItemPool[hItem];
    pItem->hNext = NULL_HANDLE;
    pItem->hPrev = NULL_HANDLE;
    pItem->hEnt = hEnt;
    if(pDynList->nDynamicListSize == 0)
    {
        pDynList->hDynamicListHead = hItem;
        pDynList->hDynamicListTail = hItem;
    }
    else
    {
        struct DynamicEntityListItem* pTail = &pDynList->pDynamicListItemPool[pDynList->hDynamicListTail];
        pTail->hNext = hItem;
        pItem->hPrev = pDynList->hDynamicListTail;
        pDynList->hDynamicListTail = hItem;
    }
    pDynList->nDynamicListSize++;
    ZONE_END(ctx)
    return hItem;
}

void DynL_RemoveItem(struct DynamicEnt2DList* pDynList, HDynamicEntityListItem hListItem)
{
    ZONE(ctx)
    struct DynamicEntityListItem* pItem = &pDynList->pDynamicListItemPool[hListItem];
    if(pDynList->hDynamicListHead == hListItem)
    {
        pDynList->hDynamicListHead = pItem->hNext;
    }
    if(pDynList->hDynamicListTail == hListItem)
    {
        pDynList->hDynamicListTail = pItem->hPrev;
    }
    if(pItem->hPrev != NULL_HANDLE)
    {
        pDynList->pDynamicListItemPool[pItem->hPrev].hNext = pItem->hNext;
    }
    if(pItem->hNext != NULL_HANDLE)
    {
        pDynList->pDynamicListItemPool[pItem->hNext].hPrev = pItem->hPrev;
    }
    pDynList->nDynamicListSize--;
    ZONE_END(ctx)
}

int DynL_GetSize(struct DynamicEnt2DList* pDynList)
{
    return pDynList->nDynamicListSize;
}
