#include "BinarySerializer.h"
#include "Atlas.h"
#include "DrawContext.h"
#include "ImageFileRegstry.h"
#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "Log.h"
#include "main.h"

struct Args
{
	char* xmlPath;
	char* outPath;
	struct EndAtlasOptions atlasOptions;
}args;

const char* defaultOutPath = "out.atlas";

void PrintHelpMsg()
{
	printf(
		"Atlas Tool\n"
		"Function:\n"
		"          Convert an xml description of an atlas into a binary atlas file\n"
		"          The binary file contains the pre-made atlas image for fast loading\n"
		"Args:\n"
		"          AtlasTool[inXMLPath][Args]\n"
		"          -o                             output .atlas file path\n"
		"          -bmp                           optional path to output bitmap\n"
		"          -iw                            initial atlas width, will grow as sprites are added if there is no room. defaults to 512\n"
		"          -ih                            initial atlas height, will grow as sprites are added if there is no room. defaults to 512\n"
		"          -initial-dims-from-sprites     take the atlases initial dims from the larges sprite\n"
	);
}

int ParseArgs(int argc, char** argv)
{

	memset(&args, 0, sizeof(struct Args));
	if (argc < 2)
	{
		Log_Error("At least one arg expected");
		return 1;
	}
	args.xmlPath = argv[1];
	args.atlasOptions.initialAtlasHeight = 512;
	args.atlasOptions.initialAtlasWidth = 512;
	for (int i = 2; i < argc; i++)
	{
		if (strcmp(argv[i], "-o") == 0)
		{
			args.outPath = argv[i + 1];
		}
		else if (strcmp(argv[i], "-h") == 0)
		{
			PrintHelpMsg();
		}
		else if (strcmp(argv[i], "-bmp") == 0)
		{
			args.atlasOptions.outDebugBitmapPath = argv[i + 1];
		}
		else if (strcmp(argv[i], "-iw") == 0)
		{
			args.atlasOptions.initialAtlasWidth = atoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "-ih") == 0)
		{
			args.atlasOptions.initialAtlasHeight = atoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "-initial-dims-from-sprites") == 0)
		{
			args.atlasOptions.bUseBiggestSpriteForInitialAtlasSize = true;
		}
	}
	if (!args.outPath)
	{
		args.outPath = defaultOutPath;
	}
	return 0;
}

static hTexture UploadTextureMock(void* src, int channels, int pxWidth, int pxHeight)
{
	return 0;
}

int main(int argc, char** argv)
{
	Log_Init();
	gCmdArgs.assetsDir = "./WfAssets";
	int r = ParseArgs(argc, argv);
	Log_Info("input file: %s", args.xmlPath);
	Log_Info("output file: %s", args.outPath);
	fflush(stdout);
	if (r)
	{
		return r;
	}

	IR_InitImageRegistry(NULL);
	At_Init();
	xmlDoc* pXMLDoc = xmlReadFile(args.xmlPath, NULL, 0);
	xmlNode* root = xmlDocGetRootElement(pXMLDoc);
	struct DrawContext dc;
	dc.UploadTexture = &UploadTextureMock;
	hAtlas atlas = At_LoadAtlasEx(root, &dc, &args.atlasOptions);
	if (atlas == NULL_HANDLE)
	{
		return 1;
	}
	struct BinarySerializer bs;
	BS_CreateForSave(args.outPath, &bs);
	At_SerializeAtlas(&bs, &atlas, &dc);
	BS_Finish(&bs);
	return 0;
}