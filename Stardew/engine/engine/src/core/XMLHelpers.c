#include "XMLHelpers.h"
#include "AssertLib.h"

xmlNode* GetNthChild(xmlNode* node, unsigned int index)
{
	int onChild = 0;
	EASSERT(xmlChildElementCount(node) > index);
	xmlNode* pChild = xmlFirstElementChild(node);
	while (pChild)
	{
		if (pChild->type != XML_ELEMENT_NODE)
		{
			pChild = pChild->next;
			continue;
		}
		if (onChild++ == index)
		{
			return pChild;
		}
		pChild = pChild->next;
	}
	return NULL;
}

xmlNode* XMLFindChild(xmlNode *parent, const char *name)
{
    for (xmlNode *cur = parent->children; cur; cur = cur->next)
    {
        if (cur->type == XML_ELEMENT_NODE &&
            xmlStrcmp(cur->name, (const xmlChar *)name) == 0)
        {
            return cur;
        }
    }
    return NULL;
}