#ifndef STARDEW_XML_HELPERS_H
#define STARDEW_XML_HELPERS_H
#include <libxml/parser.h>
#include <libxml/tree.h>

xmlNode* XMLFindChild(xmlNode *parent, const char *name);
xmlNode* GetNthChild(xmlNode* node, unsigned int index);

#endif