#include "Entity2DCollection.h"

HDynamicEntityListItem DynL_AddEntity(struct DynamicEnt2DList* pDynList, HEntity2D hEnt)
{
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
    return hItem;
}

void DynL_RemoveItem(struct DynamicEnt2DList* pDynList, HDynamicEntityListItem hListItem)
{
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
}

int DynL_GetSize(struct DynamicEnt2DList* pDynList)
{
    return pDynList->nDynamicListSize;
}
