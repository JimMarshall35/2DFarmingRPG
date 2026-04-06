#include "XMLSchema.h"

#include <libxml/parser.h>
#include <libxml/xmlschemas.h>
#include "Log.h"

bool XS_XSDValidate(const char* schemaPath, const char* dataFilePath)
{
    // Parse XML document
    xmlDocPtr doc = xmlReadFile(dataFilePath, NULL, 0);
    if (doc == NULL)
    {
        Log_Error("Failed to parse XML file: %s\n", dataFilePath);
        return false;
    }

    // Load the XSD schema
    xmlSchemaParserCtxtPtr schema_parser_ctxt = xmlSchemaNewParserCtxt(schemaPath);
    if (schema_parser_ctxt == NULL)
    {
        Log_Error("Failed to create schema parser context for %s\n", schemaPath);
        xmlFreeDoc(doc);
        return false;
    }

    xmlSchemaPtr schema = xmlSchemaParse(schema_parser_ctxt);
    if (schema == NULL)
    {
        Log_Error("Failed to parse schema: %s\n", schemaPath);
        xmlSchemaFreeParserCtxt(schema_parser_ctxt);
        xmlFreeDoc(doc);
        return false;
    }

    // Create validation context
    xmlSchemaValidCtxtPtr valid_ctxt = xmlSchemaNewValidCtxt(schema);
    if (valid_ctxt == NULL)
    {
        Log_Error("Failed to create validation context");
        xmlSchemaFree(schema);
        xmlSchemaFreeParserCtxt(schema_parser_ctxt);
        xmlFreeDoc(doc);
        return false;
    }

    // Validate the document
    int ret = xmlSchemaValidateDoc(valid_ctxt, doc);
    if (ret == 0)
    {
        Log_Info("XML is valid against the schema.");
    }
    else if (ret > 0)
    {
        Log_Info("XML is invalid against the schema.");
    } 
    else 
    {
        Log_Info("Validation generated an internal error.");
    }

    // Cleanup
    xmlSchemaFreeValidCtxt(valid_ctxt);
    xmlSchemaFree(schema);
    xmlSchemaFreeParserCtxt(schema_parser_ctxt);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return (ret == 0) ? true : false;
}