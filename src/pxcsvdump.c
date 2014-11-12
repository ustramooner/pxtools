#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "config.h"
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#include "pxtypes.h"
#include "pxparse.h"
#include "pxconvert.h"

static int delim = '\t';

int PXtoCSVString(char *dst, const unsigned char *src, int type)
{
	switch (type)
	{
	    case PX_Field_Type_Alpha:
	    case PX_Field_Type_MemoBLOB:
		break;
	    default:
		fprintf(stderr, "Can't convert type (%d)!\n",type);
		return -1;
	}

	while(*src)
	{
		switch(*src)
		{
			/* " -> "" */
		    case '"': *dst++ = '"'; *dst = *src; break;
		    default: *dst = *src;
		}
		dst++;
		src++;
	}
	*dst = '\0';
	return 0;
}

int create_csv_line(px_header *header, px_fieldInfo **felder, px_records block, char *blobname)
{
	int i, block_index = 0;

#define BLOCK_COPY(x,size) \
		memcpy(x,   block + block_index, size); \
		/*fprintf(stderr, "\nBl-IDX1: %04x - ",block_index);*/ \
		block_index += size;

	for (i = 0; i < header->numFields; i++ )
	{
		//		fprintf(stderr, "%02d: %d - ", felder[i]->type, felder[i]->size);
		if (i > 0)
			putchar(delim);
		if (felder[i]->type == PX_Field_Type_Alpha)
		{
			char *str = malloc(felder[i]->size + 1);
			char *qstr = malloc(felder[i]->size*2 + 1);
			BLOCK_COPY(str, felder[i]->size);
			str[felder[i]->size] = '\0';

			PXtoCSVString(qstr, str, felder[i]->type);
			PXtoQuotedString(qstr, qstr, felder[i]->type);
			printf("\"%s\"", qstr);

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
				printf("%Ld", d );
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
		else if (felder[i]->type == PX_Field_Type_LongInt)
		{
			unsigned long s;
			unsigned long long d;

			BLOCK_COPY(&s, felder[i]->size);

			switch(PXtoLong(s, &d, felder[i]->type))
			{
			    case VALUE_OK:
				printf("%Ld", d);
				break;
			    case VALUE_IS_NULL:
				printf("NULL");
				break;
			    default:
				break;
			}
		}
		else if (felder[i]->type == PX_Field_Type_Incremental)
		{
			unsigned long s;
			unsigned long long d;

			BLOCK_COPY(&s, felder[i]->size);

			switch(PXtoLong(s, &d, felder[i]->type))
			{
			    case VALUE_OK:
				printf("%Ld", d);
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
		else if (felder[i]->type == PX_Field_Type_MemoBLOB)
		{
			void *blob = malloc(felder[i]->size);
			char *s = NULL;

			BLOCK_COPY(blob, felder[i]->size);

			s = PXMEMOtoString(blob, felder[i]->size, blobname);

			if (s != NULL)
			{
				char *qstr = malloc(strlen(s)*2 + 1);
				PXtoCSVString(qstr, s, felder[i]->type);
				PXtoQuotedString(qstr, qstr, felder[i]->type);
				printf("\"%s\"",qstr);
				free(s);
				free(qstr);
			}
			else
			{
				char *qstr = malloc(strlen((char *)blob)*2 + 1);
				PXtoCSVString(qstr, (char *)blob, felder[i]->type);
				PXtoQuotedString(qstr, qstr, felder[i]->type);
				printf("\"%s\"",qstr);
				free(qstr);
			}
			free(blob);
		}
		else if (felder[i]->type == PX_Field_Type_Graphic ||
			 felder[i]->type == PX_Field_Type_BinBLOB)
		{
			void *blob = malloc(felder[i]->size);
			int s_size;
			void *s = NULL;

			BLOCK_COPY(blob, felder[i]->size);

			PXBLOBtoBinary(blob, felder[i]->size, blobname, &s, &s_size);

			if (s)
			{
				int fd;
				char fn[255];
				static int gra_cnt = 0;
				sprintf(fn, "/tmp/pxtools.%d-%d.bin", getpid(), gra_cnt++);
				if ((fd = open(fn,
					       O_WRONLY | O_CREAT,
					       S_IRWXU | S_IRGRP | S_IROTH
					       )) == -1)
				{
					fprintf(stderr, "%s.%d: can't open %s: %s\n",
						__FILE__, __LINE__,
						fn,
						strerror(errno));
				}
				else
				{
					write(fd, s, s_size);
					close(fd);
				}
				printf("\"file:/%s\"", fn);
			}
			else
			{
				fprintf(stderr, "%s.%d: block is empty !!\n",
					__FILE__, __LINE__);
			}

			free(blob);
		}
		else
		{
			fprintf(stderr, "\nUnknown: type: %d, size: %d\n",felder[i]->type, felder[i]->size);
			block_index += felder[i]->size;
			fprintf(stderr, "Name: %s\n", felder[i]->name);
		}
		//		fprintf(stderr, "\n");
	}
#undef BLOCK_COPY

	printf("\n");

	return 0;
}
int create_csv_dump(px_header *header, px_fieldInfo **felder, px_blocks **blocks, char *blobname)
{
	int n,f,c = 0;

	n = header->firstBlock - 1;
	while (n != -1)
	{
		if (c >= header->usedBlocks)
		{
			fprintf(stderr,
				"%s.%d: Leaving here as we are trying to use more blocks "
				"then registered in the header (header->usedBlocks\n"
				"Tell me if I'm wrong\n", __FILE__, __LINE__);
			return -1;
		}

		for (f = 0; f < blocks[n]->numRecsInBlock; f++)
			create_csv_line(header, felder, blocks[n]->records[f], blobname);
		n = blocks[n]->nextBlock - 1;
		c++;
	}

	return 0;
}

void display_help ()
{
	printf(
	       "PXCSVdump - (%s - %s)\n"
	       "Usage: pxcsvdump [OPTION]...\n"
	       "\n"
	       "Creates a CSV-dump from a Paradox-database-file\n"
	       "\n"
	       "Options:\n"
	       "  -b, --blobname=<name>   Name of the .MB-file <name>\n"
	       "  -D, --delimiter=<delim> Field delimiter to use instead of comma   \n"
	       "  -f, --filename=<name>   Name of the .DB-file <name>\n"
	       "  -h, --help              Display this help and exit\n"
	       "  -V, --version           Output version information and exit\n"
	       "\n"
	       "\n", PACKAGE, VERSION);
}

void display_version()
{
	printf(
	       "PXCVSdump - (%s - %s)\n"
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
	char *blobname = NULL, *filename = NULL;
#ifdef HAVE_GETOPT_LONG
	static struct option long_options[] =
	{
		{"blobname", required_argument, 0, 'b'},
		{"delimiter", required_argument, 0, 'D'},
		{"filename", required_argument, 0, 'f'},
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'V'},
		{0, 0, 0, 0}
	};
#endif
	
	while ( 
#ifdef HAVE_GETOPT_LONG
		(i = getopt_long(argc, argv, "b:D:f:hV", long_options, (int *) 0)) != EOF
#else
		(i = getopt(argc, argv, "b:D:f:hV")) != EOF
#endif
		)
	{
		switch (i)
		{
		    case 'b' :
			blobname = optarg;
			break;
		    case 'D':
			if (optarg[0] != '\0' && optarg[1] != '\0')
			{
				fprintf(stderr, "%s: the delimiter must be a single character\nTry `%s' for more information.\n", PACKAGE, PACKAGE);
				exit(-1);
			}
			delim = (unsigned char) optarg[0];
			break;
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
			fprintf(stderr, "%s: Can't open file: %s\n", PACKAGE, filename);
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
		fprintf(stderr, "File '%s' is not a paradox-file\n", argv[optind]);
		exit(-1);
	}

	blocks = PXparseBlocks(f, &header);

	close (f);

	if (!blocks)
	{
		fprintf(stderr, "no rows found\n");
		return(-1);
	}

	create_csv_dump (&header, felder, blocks, blobname);

	return 0;
}
