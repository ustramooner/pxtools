#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#include "pxtypes.h"
#include "pxparse.h"
#include "pxconvert.h"

int show_header_info (px_header *header)
{
	printf("%-20s",_("File-Version: "));
	switch (header->fileVersionID)
	{
	    case 0x03: printf("Paradox 3.0"); break;
	    case 0x04: printf("Paradox 3.5"); break;
	    case 0x05:
	    case 0x06:
	    case 0x07:
	    case 0x08:
	    case 0x09: printf("Paradox 4.x"); break;
	    case 0x0a:
	    case 0x0b: printf("Paradox 5.x"); break;
	    case 0x0c: printf("Paradox 7.x"); break;
	    default: printf("%s %02x", _("Unknown"), header->fileVersionID); break;
	}
	printf("\n");
	printf("%-20s",_("Filetype: "));
	switch (header->fileType)
	{
	    case 0x00: printf("%s",_("indexed .DB")); break;
	    case 0x01: printf("%s",_("primary index .PX")); break;
	    case 0x02: printf("%s",_("non indexed .DB")); break;
	    case 0x03: printf("%s",_("non-incrementing secondary index .Xnn")); break;
	    case 0x04: printf("%s",_("secondary index .Ynn (inc/non-inc)")); break;
	    case 0x05: printf("%s",_("incrementing secondary index .Xnn")); break;
	    case 0x06: printf("%s",_("non-incrementing secondary index .XGn")); break;
	    case 0x07: printf("%s",_("secondary index .YGn (inc/non-inc)")); break;
	    case 0x08: printf("%s",_("incrementing secondary index .XGn")); break;
	    default: printf("%s %02x",_("Unknown"), header->fileType); break;
	}
	printf("\n");

	printf("%-20s%s\n",_("Tablename:"),header->tableName);

	printf("%-20s",_("Sort-Order:"));
	switch (header->sortOrder)
	{
	    case 0x00: printf("%s",_("ASCII")); break;
	    case 0xb7: printf("%s",_("International")); break;
	    case 0x82:
	    case 0xe6: printf("%s",_("Norwegian/Danish")); break;
	    case 0x0b: printf("%s",_("Swedish/Finnish")); break;
	    case 0x5d: printf("%s",_("Spanish")); break;
	    case 0x62: printf("%s",_("PDX ANSI intl")); break;
	    default: printf("%s %02x",_("Unknown"), header->sortOrder); break;
	}
	printf("\n");
	printf("%-20s",_("Write-Protection:"));
	switch (header->writeProtected)
	{
	    case 0x00: printf("%s",_("off")); break;
	    case 0x01: printf("%s",_("on")); break;
	    default: printf("%s %02x",_("Unknown"), header->writeProtected); break;
	}
	printf("\n");

	if (	header->fileVersionID >= 0x05 &&
		header->fileType != 0x1 &&
		header->fileType != 0x4 &&
		header->fileType != 0x7 )
	{

		printf("%-20s",_("Codepage:"));
		switch (header->dosGlobalCodePage)
		{
		    case 0x01b5: printf("%s",_("United States")); break;
		    case 0x04e4: printf("%s",_("Spain")); break;
		    default: printf("%s %04x", _("Unknown"), header->dosGlobalCodePage); break;
		}
		printf("\n");
	}

	printf("%-20s%i\n","Number of Blocks:",header->fileBlocks);
	printf("%-20s%i\n","Used Blocks:",header->usedBlocks);
	printf("%-20s%i\n","First Block:",header->firstBlock);
	printf("%-20s%i\n","Number of Records:",header->numRecords);
	printf("%-20s%i\n","Max. Tablesize:",header->maxTableSize);
	printf("%-20s%i\n","Recordsize:",header->recordSize);

	if (header->fileType == PX_Filetype_PX)
	{
		printf("%-20s%i\n","Index-root:",header->indexRootBlock);
		printf("%-20s%i\n","Index-levels:",header->indexLevels);
	}

	return 0;
}

int showFieldInfo(px_fieldInfo *fi)
{
	char fType = fi->type;
	unsigned char fSize = fi->size;

	printf("Name: %-20s",fi->name);

	printf("Type: ");
	switch (fType)
	{
	    case PX_Field_Type_Alpha:	printf("%-15s","Alpha"); break;
	    case PX_Field_Type_Date:	printf("%-15s","Date"); break;
	    case PX_Field_Type_ShortInt:	printf("%-15s","Short Integer"); break;
	    case PX_Field_Type_LongInt:	printf("%-15s","Long Integer"); break;
	    case PX_Field_Type_Currency:	printf("%-15s","Currency"); break;
	    case PX_Field_Type_Number:	printf("%-15s","Number"); break;
	    case PX_Field_Type_MemoBLOB:	printf("%-15s","Memo BLOB"); break;
	    case PX_Field_Type_Graphic:	printf("%-15s","Graphic"); break;
	    case PX_Field_Type_BinBLOB:	printf("%-15s","BLOB"); break;
	    case PX_Field_Type_Logical:	printf("%-15s","Logical"); break;
	    case PX_Field_Type_Time:	printf("%-15s","Time"); break;
	    case PX_Field_Type_Timestamp:	printf("%-15s","Timestamp"); break;
	    case PX_Field_Type_Incremental:	printf("%-15s","Incremental"); break;
	    default: printf("Unknown: %02x",fType); break;
	}
	printf(" Size: %i\n",fSize);
	return 0;
}

int showRecord (char *block, px_header *header, px_fieldInfo **felder)
{
	int i, block_index = 0;

#define BLOCK_COPY(x,size) \
		memcpy(x,   block + block_index, size); \
		/*printf("Bl-IDX1: %i\n",block_index);*/ \
		block_index += size;

	for (i = 0; i < header->numFields; i++ )
	{
		if (felder[i]->type == PX_Field_Type_Alpha)
		{
			char *str = malloc(felder[i]->size + 1);
			char *qstr = malloc(felder[i]->size*2 + 1);
			BLOCK_COPY(str, felder[i]->size);
			str[felder[i]->size] = '\0';

			PXtoQuotedString(qstr, str, felder[i]->type);
			printf("\'%s\'", qstr);

			free(qstr);
			free(str);
		}
		else if (felder[i]->type == PX_Field_Type_ShortInt)
		{
			unsigned short s;
			unsigned long long d;

			BLOCK_COPY(&s, felder[i]->size);

			switch(PXtoLong(s, &d, felder[i]->type))
			{
			    case VALUE_OK:
				printf("%Li", d );
				break;
			    case VALUE_IS_NULL:
				printf("NULL");
				break;
			    default:
				break;
			}

		}
		else if (felder[i]->type == PX_Field_Type_Logical)
		{
			unsigned char s;
			unsigned long long d;

			BLOCK_COPY(&s, felder[i]->size);

			switch(PXtoLong(s, &d, felder[i]->type))
			{
			    case VALUE_OK:
				printf("%Lx", d);
				break;
			    case VALUE_IS_NULL:
				printf("NULL");
				break;
			    default:
				break;
			}
		}
		else if (felder[i]->type == PX_Field_Type_Number)
		{
			unsigned long long s;
			double d;

			BLOCK_COPY(&s, felder[i]->size);

			switch(PXtoDouble(s, &d, felder[i]->type))
			{
			    case VALUE_OK:
				printf("%f", d );
				break;
			    case VALUE_IS_NULL:
				printf("NULL");
				break;
			    default:
				break;
			}

		}
		else if (felder[i]->type == PX_Field_Type_Currency)
		{
			unsigned long long s;
			double d;

			BLOCK_COPY(&s, felder[i]->size);

			switch(PXtoDouble(s, &d, felder[i]->type))
			{
			    case VALUE_OK:
				printf("%f", d );
				break;
			    case VALUE_IS_NULL:
				printf("NULL");
				break;
			    default:
				break;
			}
		}
		else if (felder[i]->type == PX_Field_Type_Date)
		{
			unsigned long long s;
			struct tm _tm;

			BLOCK_COPY(&s, felder[i]->size);

			switch (PXtoTM(s, &_tm, felder[i]->type))
			{
			    case VALUE_OK:
				printf("'%04i-%02i-%02i'", _tm.tm_year+1900, _tm.tm_mon+1, _tm.tm_mday);
				break;
			    case VALUE_IS_NULL:
				printf("NULL");
				break;
			    default:
				break;
			}

		}
		else if (felder[i]->type == PX_Field_Type_Time)
		{
			unsigned long long s;
			struct tm _tm;

			BLOCK_COPY(&s, felder[i]->size);

			switch (PXtoTM(s, &_tm, felder[i]->type))
			{
			    case VALUE_OK:
				printf("%02i:%02i:%02i'",_tm.tm_hour, _tm.tm_min, _tm.tm_sec);
				break;
			    case VALUE_IS_NULL:
				printf("NULL");
				break;
			    default:
				break;
			}
		}
		else if (felder[i]->type == PX_Field_Type_Timestamp)
		{
			unsigned long long s;
			struct tm _tm;

			BLOCK_COPY(&s, felder[i]->size);

			switch (PXtoTM(s, &_tm, felder[i]->type))
			{
			    case VALUE_OK:
				printf("'%04i-%02i-%02i ", _tm.tm_year+1900, _tm.tm_mon+1, _tm.tm_mday);
				printf("%02i:%02i:%02i'",_tm.tm_hour, _tm.tm_min, _tm.tm_sec);
				break;
			    case VALUE_IS_NULL:
				printf("NULL");
				break;
			    default:
				break;
			}
		}
		else if (felder[i]->type == PX_Field_Type_MemoBLOB ||
			 felder[i]->type == PX_Field_Type_Graphic)
		{
			void *blob = malloc(felder[i]->size);

			BLOCK_COPY(blob, felder[i]->size);

			printf("\"%s\"",(char *)blob);

		}
		else
		{
			printf("\nUnknown, please report: %i\n",felder[i]->size);
			block_index += felder[i]->size;
			printf("Name: %s\n", felder[i]->name);
		}
	}

	if (header->fileType == PX_Filetype_PX)
	{
		short s;
		unsigned long long d;

		BLOCK_COPY(&s,2);
		PXtoLong(s, &d, PX_Field_Type_ShortInt);
		printf("BlockNumber: %Li\n", d);

		BLOCK_COPY(&s,2);
		PXtoLong(s, &d, PX_Field_Type_ShortInt);
		printf("Statistics: %Li\n", d);
	}
#undef BLOCK_COPY
	return 0;
}

void display_help ()
{
	printf(
	       "PXInfo - (%s - %s)\n"
	       "Usage: pxinfo [OPTION]...\n"
	       "\n"
	       "Display the header-information of a Paradox-database-file\n"
	       "\n"
	       "Options:\n"
	       "  -f, --filename=<name> Name of the .DB-file <name>\n"
	       "  -h, --help            Display this help and exit\n"
	       "  -V, --version         Output version information and exit\n"
	       "\n"
	       "\n", PACKAGE, VERSION);
}

void display_version()
{
	printf(
	       "PXInfo - (%s - %s)\n"
	       "Written by Jan Kneschke.\n"
	       "\n"
	       "\n", PACKAGE, VERSION);
}

int main ( int argc, char **argv)
{
	px_header header;
	px_fieldInfo **felder;
	px_blocks **blocks;

	int f,i;
	char *filename = NULL;
#ifdef HAVE_GETOPT_LONG
	static struct option long_options[] =
	{
		{"filename", required_argument, 0, 'f'},
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'V'},
		{0, 0, 0, 0}
	};
#endif

#ifdef HAVE_GETTEXT
	setlocale(LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);
#endif

	while ( 
#ifdef HAVE_GETOPT_LONG
		(i = getopt_long(argc, argv, "f:hV", long_options, (int *) 0)) != EOF
#else
		(i = getopt(argc, argv, "f:hV")) != EOF
#endif		
		)
		
	{
		switch(i)
		{
		    case 'f' :
			filename = optarg;
			break;
		    case 'V':
			display_version();
			exit(-1);
			break;
		    default:
			display_help();
			exit(-1);
			break;
		}
	}

	if (filename)
	{
#ifdef O_BINARY
		f = open (filename , O_RDONLY|O_BINARY);
#else
		f = open (filename , O_RDONLY);
#endif

		if (f == -1)
		{
			fprintf(stderr, "Can't open file %s: %s\n", filename, strerror(errno));
			exit(-1);
		}
	}
	else
	{
		display_help();
		exit(-1);
	}

	if (!(felder = PXparseCompleteHeader(f, &header)))
	{
		if (PXparseMBHeader(f) == 0)
		{
			return -1;
		}
		else
		{
			printf("File '%s' is not a paradox-file\n",argv[1]);
			return -1;
		}
	}

	blocks = PXparseBlocks(f, &header);

	close (f);

	printf("== File-header ==\n");
	show_header_info(&header);

	printf("== Fields ==\n");
  /* show field-information */
	for ( i = 0; i < header.numFields;i++)
	{
		showFieldInfo(felder[i]);
	}

	if (!blocks)
	{
		printf("Keine Spalten\n");
		return(-1);
	}

  /* set to 1 to dump block-info */
#if 0
	printf("== Blocks ==\n");
	n = header.firstBlock - 1;
	while (n != -1)
	{
		for (f = 0; f < blocks[n]->numRecsInBlock; f++)
			showRecord(blocks[n]->records[f], &header, felder);
		n = blocks[n]->nextBlock - 1;
	}
#endif
	return 0;
}

