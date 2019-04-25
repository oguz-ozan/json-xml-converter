//gcc -I/usr/include/libxml2 -I/usr/local/include -L/usr/local/lib -ljson-c -lxml2 -Wall 2013510126.c -o 2013510126.out
// can be used like
// ./2013510126 file1 file2 number
// where number between 1 and 7
// 1:csv-xml, 4:json-xml, 5:csv-json, 7:xml-xsd
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <json-c/json.h>
#include <libxml/xmlschemastypes.h>


#define REC_COUNT 13
#define REC_SIZE sizeof(struct _Student)
typedef struct _Student Student;

static void xml_validation(char *a1, char *a2)
{
    xmlDocPtr doc;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;
    //for TASK 2
    // char *XMLFileName = "shipto.xml";
    // char *XSDFileName = "shipto.xsd";
    //uncomment for TASK 3
    char *XMLFileName = a1;
    char *XSDFileName = a2;

    xmlLineNumbersDefault(1);                   //set line numbers, 0> no substitution, 1>substitution
    ctxt = xmlSchemaNewParserCtxt(XSDFileName); //create an xml schemas parse context
    schema = xmlSchemaParse(ctxt);              //parse a schema definition resource and build an internal XML schema
    xmlSchemaFreeParserCtxt(ctxt);              //free the resources associated to the schema parser context

    doc = xmlReadFile(XMLFileName, NULL, 0); //parse an XML file
    if (doc == NULL)
    {
        fprintf(stderr, "Could not parse %s\n", XMLFileName);
    }
    else
    {
        xmlSchemaValidCtxtPtr ctxt; //structure xmlSchemaValidCtxt, not public by API
        int ret;

        ctxt = xmlSchemaNewValidCtxt(schema);  //create an xml schemas validation context
        ret = xmlSchemaValidateDoc(ctxt, doc); //validate a document tree in memory
        if (ret == 0)                          //validated
        {
            printf("%s validates\n", XMLFileName);
        }
        else if (ret > 0) //positive error code number
        {
            printf("%s fails to validate\n", XMLFileName);
        }
        else //internal or API error
        {
            printf("%s validation generated an internal error\n", XMLFileName);
        }
        xmlSchemaFreeValidCtxt(ctxt); //free the resources associated to the schema validation context
        xmlFreeDoc(doc);
    }
    // free the resource
    if (schema != NULL)
        xmlSchemaFree(schema); //deallocate a schema structure

    xmlSchemaCleanupTypes(); //cleanup the default xml schemas types library
    xmlCleanupParser();      //cleans memory allocated by the library itself
    xmlMemoryDump();         //memory dump
}
static void json_parse(json_object *jobj, xmlNodePtr xmlRoot)
{
    enum json_type type;
    json_object_object_foreach(jobj, key, val)
    {
        json_object *firstJobj;
        xmlNodePtr newNode;
        type = json_object_get_type(val);
        switch (type)
        {
        case json_type_array:
            newNode = xmlNewChild(xmlRoot, NULL, BAD_CAST key, NULL);
            json_object *jsonArray = json_object_new_object();
            if (key)
            {
                jsonArray = json_object_object_get(jobj, key);
            }
            int arrayLength = json_object_array_length(jsonArray);
            int i;
            json_object *arrayObjects = json_object_new_object();
    
            for (i = 0; i < arrayLength; i++)
            {
                arrayObjects = json_object_array_get_idx(jsonArray, i);
                xmlNewChild(newNode, NULL, BAD_CAST "Student", NULL);
                json_parse(arrayObjects, newNode);
            }
            break;
        case json_type_object:
            firstJobj = json_object_object_get(jobj, key);
            newNode = xmlNewChild(xmlRoot, NULL, BAD_CAST key, NULL);
            json_parse(firstJobj, newNode);
            break;
        default:
            xmlNewChild(xmlRoot, NULL, BAD_CAST key, BAD_CAST json_object_get_string(val));
            break;
        }
    }
}

// here is preparation for json_parse method above. 
static void json_to_xml(char *a1, char *a2)
{
    xmlDocPtr jsonToXML_DOC = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr xmlRoot = xmlNewNode(NULL, BAD_CAST "root");
    xmlDocSetRootElement(jsonToXML_DOC, xmlRoot);

    char *buffer = 0;
    long length;
    FILE *f = fopen(a1, "rb");
    // here we take the file as a string in memory.
    if (f)
    {
        printf("ftell is: %lu", ftell(f));
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        printf("ftell is: %lu", ftell(f));
        fseek(f, 0, SEEK_SET);
        buffer = malloc(length + 1);
        if (buffer)
        {
            fread(buffer, 1, length, f);
        }
        fclose(f);
        buffer[length] = '\0';
    }

    if (buffer)
    {
        json_object *jobj = json_tokener_parse(buffer);
        json_parse(jobj, xmlRoot);
        xmlSaveFormatFile(a2, jsonToXML_DOC, 1);
    }
}
static void csvToxml(char *a1, char *a2)
{
    xmlDocPtr doc = NULL;                                   /* document pointer */
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL; /* node pointers */
    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "Students");
    xmlDocSetRootElement(doc, root_node);

    char *CSVFileName = a1;
    FILE *fp;
    fp = fopen(CSVFileName, "r");
    if (fp == NULL)
        printf("File not opened");
    else
    {
        char buff[1024];
        fseek(fp, 0L, SEEK_SET);
        char *token;
        int count = 0;
        int recordcnt = 1;
        token = strtok(fgets(buff, 1024, fp), ",");
        while (token != NULL)
        {
            printf("%s ", token);
            token = strtok(NULL, ",");
            count++;
        }
        fseek(fp, 0L, SEEK_SET);
        // for attribute namings, will be used in line 191. these attributes will be the name of node for xml element
        const char *a[count];
        token = strtok(fgets(buff, 1024, fp), ",");
        //cntr will be used for attribute number. there are 13 attributes and each one of them will be assigned to a array.
        int cntr = 0;
        while (token != NULL)
        {
            a[cntr] = strdup(token);
            printf("%s,%d", a[cntr], cntr);
            token = strtok(NULL, ",");
            cntr++;
        }
        printf("\n");
        while (fgets(buff, 1024, fp) != NULL)
        {
            char *line = buff;
            char *field;
            int index = 0;
            node = xmlNewNode(NULL, BAD_CAST "Student");
            xmlAddChild(root_node, node);
            printf("record number: %d\n", recordcnt);
            while ((field = strsep(&line, ",")) != NULL)
            {
                node1 = xmlNewChild(node, NULL, BAD_CAST a[index], BAD_CAST field);
                printf("element %d = %s\n", index, field);
                index++;
            }
            recordcnt++;
        }
        fclose(fp);
    }
    xmlSaveFormatFileEnc(a2, doc, "UTF-8", 1);
    //xmlSaveFormatFile
    /*free the document */
    xmlFreeDoc(doc);
    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();
    xmlMemoryDump();
}

static void csvTojson(char *a1, char *a2)
{

    char *CSVFileName = a1;
    FILE *fp;
    fp = fopen(CSVFileName, "r");
    if (fp == NULL)
        printf("File not opened");
    else
    {
        char buff[1024];
        fseek(fp, 0L, SEEK_SET);
        char *token;
        int count = 0;
        int recordcnt = 1;

        token = strtok(fgets(buff, 1024, fp), ",");
        while (token != NULL)
        {
            printf("%s ", token);
            token = strtok(NULL, ",");
            count++;
        }
        fseek(fp, 0L, SEEK_SET);
        const char *a[count];
        token = strtok(fgets(buff, 1024, fp), ",");
        int cntr = 0;
        while (token != NULL)
        {
            a[cntr] = strdup(token);
            printf("%s,%d", a[cntr], cntr);
            token = strtok(NULL, ",");
            cntr++;
        }
        printf("\n");
        json_object *jobj = json_object_new_object();
        json_object *jarray = json_object_new_array();
        while (fgets(buff, 1024, fp) != NULL)
        {
            char *line = buff;
            char *field;
            int index = 0;

            json_object *jobject = json_object_new_object();
            printf("record number: %d\n", recordcnt);
            while ((field = strsep(&line, ",")) != NULL)
            {
                json_object *jstring1 = json_object_new_string(field);
                json_object_object_add(jobject, a[index], jstring1);
                printf("element %d = %s\n", index, field);
                index++;
            }
            json_object_array_add(jarray, jobject);
            recordcnt++;
        }
        fclose(fp);
        json_object_object_add(jobj, "Students", jarray);
        printf("The json object created: %s\n", json_object_to_json_string(jobj));
        FILE *fp;
        fp = fopen(a2, "w");
        fprintf(fp, "%s", json_object_to_json_string(jobj));
    }
}

int main(int argc, char *argv[])
{
    char csv_xml[] = "1";
    //char xml_csv[] = "2";
    //char xml_json[] = "3";
    char json_xml[] = "4";
    char csv_json[] = "5";
    // char json_csv[] = "6";
    char xml_xsd[] = "7";

    if (strcmp(argv[3], xml_xsd) == 0)
    {
        xml_validation(argv[1], argv[2]);
    }
    else if (strcmp(argv[3], json_xml) == 0)
    {
        json_to_xml(argv[1], argv[2]);
    }
    else if (strcmp(argv[3], csv_xml) == 0)
    {
        csvToxml(argv[1], argv[2]);
    }
    else if (strcmp(argv[3], csv_json) == 0)
    {
        csvTojson(argv[1], argv[2]);
    }
}
