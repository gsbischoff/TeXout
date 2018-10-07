#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gumbo-parser/src/gumbo.h"

#define ArrayCount(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct
{
	unsigned int Size;
	char *Contents;
} entire_file;

// Stores the tag and attribute/value pair of the element we wish to find
// Can create a list of these to represent a desired treversal of the parsetree
// If the AttributeValue is NULL, we ignore it and only match the tag and name.
typedef struct
{
	GumboTag Tag;
	char *AttributeName;
	char *AttributeValue;
} UniqueElement;

UniqueElement WorkPath[] =
{
	{ GUMBO_TAG_BODY, "class" , "logged-out"},
	{ GUMBO_TAG_DIV , "id" , "outer"},
	{ GUMBO_TAG_DIV , "id" , "inner"},
	{ GUMBO_TAG_DIV , "id" , "main"},
	{ GUMBO_TAG_DIV , "class" , "work"},
};

// All of the following paths are relative to WorkPath

UniqueElement NextChapterPath[] =
{
	{ GUMBO_TAG_UL , "class" , "work navigation actions"},
	{ GUMBO_TAG_LI , "class" , "chapter next"},
	{ GUMBO_TAG_A , "href" , 0},
};

UniqueElement NextWorkPath[] =
{
	{ GUMBO_TAG_DIV , "class" , "wrapper"},
	{ GUMBO_TAG_DL , "class" , "work meta group"},
	{ GUMBO_TAG_DD , "class" , "series"},
	{ GUMBO_TAG_SPAN , "class" , "series"},
	{ GUMBO_TAG_A , "href" , 0},
};

UniqueElement Chapter[] =
{
	{ GUMBO_TAG_DIV , "id" , "workskin"},
	{ GUMBO_TAG_DIV , "id" , "chapters"},
	{ GUMBO_TAG_DIV , "class" , "chapter"},
	{ GUMBO_TAG_DIV , "class" , "userstuff module"},
};

GumboNode *
FindNode(GumboNode *Root, UniqueElement *PathList, int Length);

const char *
FindNextChapter(GumboNode *Root)
{
	GumboNode *NextChapter = FindNode(Root, NextChapterPath, ArrayCount(NextChapterPath));
	if(!NextChapter)
	{
		return(0);
	}

	GumboAttribute *Hyperlink = gumbo_get_attribute(&NextChapter->v.element.attributes, "href");
	if(!Hyperlink)
	{
		return(0);
	}

	return Hyperlink->value;
}

GumboNode *
GetElementByAttribute(GumboNode *Node, UniqueElement Identifier)
{
	if(!Node || Node->type != GUMBO_NODE_ELEMENT)
	{
		return(0);
	}

	GumboVector Children = Node->v.element.children;
	for(int Index = 0;
		Index < Children.length;
		++Index)
	{
		GumboNode *Child = Children.data[Index];
		if(Child->type == GUMBO_NODE_ELEMENT &&
		   Child->v.element.tag == Identifier.Tag)
		{
			GumboAttribute *Attribute = gumbo_get_attribute(&Child->v.element.attributes, Identifier.AttributeName);
			if(!Attribute)
			{
				continue;
			}
			else if(Identifier.AttributeValue == 0 ||
					strcmp(Attribute->value, Identifier.AttributeValue) == 0)
			{
				return Child;
			}
		}
	}

	return(0);
}


GumboNode *
FindNode(GumboNode *Root, UniqueElement *PathList, int Length)
{
	if(0 == Root)
	{
		return(0);
	}
	else
	{
		// Look at a root's children, see if it is the next tag
		// we want, if so, set him as our root, check his...
		GumboNode *Node = Root;
		for(int Index = 0;
			Index < Length;
			++Index)
		{
			Node = GetElementByAttribute(Node, PathList[Index]);

			if(!Node)
			{
				printf("Could not find tag %s with attribute %s=\"%s\" at level %u!\n",
			       gumbo_normalized_tagname(PathList[Index].Tag), 
				   PathList[Index].AttributeName, 
				   PathList[Index].AttributeValue,
				   Index + 1);
				return(0);
			}
		}

		if(Node)
		{
			#if 0
			GumboElement *Element = &Node->v.element;
			printf("Found element!\n");
			printf("  ELEMENT\n");
			printf("    Start: %u\n", Element->start_pos.offset);
			printf("    End: %u\n", Element->end_pos.offset);
			printf("    Children: %u\n", Element->children.length);
			printf("    Tag: %.*s\n", (int)Element->original_tag.length, Element->original_tag.data);
			#endif
			return Node;
		}
	}

	return(0);
}

int
main(int ArgCount, char **Args)
{
	entire_file EntireFile = {0};

	// Read/download the entire HTML file into a buffer here
	FILE *Input = fopen(Args[1], "r");

	if(Input)
	{
		fseek(Input, 0, SEEK_END);
		EntireFile.Size = ftell(Input);
		fseek(Input, 0, SEEK_SET);

		EntireFile.Contents = malloc(EntireFile.Size + 1);

		fread(EntireFile.Contents, 1, EntireFile.Size, Input);
		EntireFile.Contents[EntireFile.Size] = 0;

		printf("Read in %d bytes\n", EntireFile.Size);
	}
	else
	{
		fprintf(stderr, "%s\n", ArgCount == 2 ? "Could not open file!" : "usage: TeXout file.html");
		return(0);
	}

	GumboOutput *ParseTree = gumbo_parse(EntireFile.Contents);

	GumboNode *Work = FindNode(ParseTree->root, WorkPath, ArrayCount(WorkPath));
	const char *NextChapterLink = FindNextChapter(Work);
	printf("The link to the next chapter is archiveofourown.org%s\n", NextChapterLink);

	GumboNode *Chap = FindNode(Work, Chapter, ArrayCount(Chapter));
	printf("Chap has %u children!\n", Chap->v.element.children.length);

	for(int i = 0; i < Chap->v.element.children.length; ++i)
	{
		GumboNode *Node = Chap->v.element.children.data[i];
		if(Node->type == GUMBO_NODE_ELEMENT)
		{
			GumboElement *Element = &Node->v.element;
			printf("[%u]ELEMENT\n", i);
			printf("    Start: %u\n", Element->start_pos.offset);
			printf("    End: %u\n", Element->end_pos.offset);
			printf("    Children: %u\n", Element->children.length);
			printf("    Tag: %.*s\n", (int)Element->original_tag.length, Element->original_tag.data);

			for(int j = 0; j < Element->children.length; ++j)
			{
				GumboNode *Child = Element->children.data[j];
				if(Child->type == GUMBO_NODE_ELEMENT)
				{
					GumboElement *CC = &Child->v.element;
					printf("    [%u]CHILD\n", j);
					printf("      Start: %u\n", CC->start_pos.offset);
					printf("      End: %u\n", CC->end_pos.offset);
					printf("      Children: %u\n", CC->children.length);
					printf("      Tag: %.*s\n", (int)CC->original_tag.length, CC->original_tag.data);
				}
				else if(Child->type != GUMBO_NODE_DOCUMENT && Child->type != GUMBO_NODE_TEMPLATE)
				{
					GumboText *Text = &Child->v.text;
					printf("    [%u]CTEXT\n", j);
					printf("      Data: \"%.*s\"\n", (int)Text->original_text.length, Text->original_text.data);
				}
			}
		}
		else if(Node->type != GUMBO_NODE_DOCUMENT && Node->type != GUMBO_NODE_TEMPLATE)
		{
			GumboText *Text = &Node->v.text;
			printf("[%u]TEXT\n", i);
			printf("    Data: \"%.*s\"\n", (int)Text->original_text.length, Text->original_text.data);
		}

		if(i > 2) break;
	}


	gumbo_destroy_output(&kGumboDefaultOptions, ParseTree);

	// "Entire work" link, make tex; then append Next Work (entire work); and so on...
	//char *NextWork = 0;
	//if((NextWork = strtok(EntireFile.Contents, "<a class=\"next\" href=\"")) > 0)
	//{
	//	strtok(0, "\">");
	//}



	// Use information around tokens of different types (from html <>'s')
	// to put into TeX notation (<></> to {\cmd })

	// Turn >1 newlines into 2 newlines. (2 to 2, even!)

	// \bye

	// TeX dat
}