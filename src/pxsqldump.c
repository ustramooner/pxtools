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

#define DB_UNSET 0
#define DB_MYSQL 1
#define DB_PGSQL 2
#define DB_OTHER 3

int dbtype = DB_UNSET;

int name_quoting = -1;
#define OPT_MYSQL "mysql"
#define OPT_PGSQL "pgsql"

char * str_to_sql(const unsigned char *src);
char * binary_to_sql(const unsigned char *src, int src_len, int *dst_len);
char * quote(const unsigned char *src, const unsigned int name_quoting);

int create_sql_CREATE(px_header *header, px_fieldInfo **felder)
{
	int i;
	char *name = NULL;

	name = quote(PXNametoQuotedName(header->tableName), name_quoting);
	printf ("CREATE TABLE %s (\n", name);
	free(name);

	for ( i = 0; i < header->numFields;i++)
	{
		if (i > 0) printf(",\n");
		name = quote(PXNametoQuotedName(felder[i]->name), name_quoting);
		printf("\t%s ", name);
		free(name);

		switch (felder[i]->type)
		{
		    case PX_Field_Type_Alpha:	printf("VARCHAR"); break;
		    case PX_Field_Type_Date:	printf("DATE"); break;
		    case PX_Field_Type_ShortInt:	printf("INTEGER"); break;
		    case PX_Field_Type_LongInt:	printf("INTEGER"); break;
		    case PX_Field_Type_Currency:	printf("DECIMAL"); break;
		    case PX_Field_Type_Number:	printf("DOUBLE PRECISION"); break;
		    case PX_Field_Type_MemoBLOB:	printf("BLOB"); break;
		    case PX_Field_Type_BinBLOB:	printf("BLOB"); break;
		    case PX_Field_Type_Graphic:	printf("BLOB"); break;
		    case PX_Field_Type_Logical:	printf("INTEGER"); break;
		    case PX_Field_Type_Time:	printf("TIME"); break;
		    case PX_Field_Type_Timestamp:	printf("TIMESTAMP"); break;
		    case PX_Field_Type_Incremental:	printf("INTEGER"); break;
		    case PX_Field_Type_BCD:		printf("INTEGER"); break;
		    default: printf("Unknown: %02x",felder[i]->type); break;
		}

		switch (felder[i]->type)
		{
		    case PX_Field_Type_Logical:
		    case PX_Field_Type_Alpha:
			if ((dbtype == DB_PGSQL)&&(felder[i]->size == 1))
				break;
			printf("(%d)", felder[i]->size);
			break;
		    case PX_Field_Type_Currency:
			printf("(12,2)");
			break;
		    case PX_Field_Type_Incremental:
			printf(" NOT NULL /* auto_increment */");
			break;
		}

	}
	printf("\n);\n");

	return 0;
}

int create_sql_INSERT(px_header *header, px_fieldInfo **felder, px_records block, char *blobname)
{
	int i, block_index = 0;
	char *name = NULL;

	name = quote(header->tableName, name_quoting);
	printf("INSERT INTO %s VALUES (", name);
	free(name);

#define BLOCK_COPY(x,size) \
		memcpy(x,   block + block_index, size); \
		/*printf("\nBl-IDX1: %04x - ",block_index);*/ \
		block_index += size;

	for (i = 0; i < header->numFields; i++ )
	{
		if (i > 0) printf(",");
		if (felder[i]->type == PX_Field_Type_Alpha)
		{
			char *str = malloc(felder[i]->size + 1);
			char *qstr = malloc(felder[i]->size*2 + 1);
			char *qqstr = NULL;
			BLOCK_COPY(str, felder[i]->size);
			str[felder[i]->size] = '\0';

			PXtoQuotedString(qstr, str, felder[i]->type);
			qqstr = str_to_sql(qstr);
			printf("\'%s\'", qqstr);

			free(qqstr);
			free(qstr);
			free(str);
		}
		else if (felder[i]->type == PX_Field_Type_BCD)
		{
			char *str = malloc(felder[i]->size + 1);

			BLOCK_COPY(str, felder[i]->size);

			printf("-\'%s\'-", str);

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
				printf("%Ld", d );
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
				printf("%Ld", d );
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
				printf("'%02d:%02d:%02d'",_tm.tm_hour, _tm.tm_min, _tm.tm_sec);
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
				printf("'%04d-%02d-%02d ", _tm.tm_year+1900, _tm.tm_mon+1, _tm.tm_mday);
				printf("%02d:%02d:%02d'",_tm.tm_hour, _tm.tm_min, _tm.tm_sec);
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
			char *qqstr = NULL;

			BLOCK_COPY(blob, felder[i]->size);

			s = PXMEMOtoString(blob, felder[i]->size, blobname);

			/* if the MEMO is in the external BLOB s will be set,
			 * otherwise we use blob from the .DB file
			 */
			qqstr = str_to_sql(s ? s : (char *)blob);
			printf("\'%s\'",qqstr);

			if (s) free(s);
			free(qqstr);
			free(blob);
		}
		else if (felder[i]->type == PX_Field_Type_Graphic ||
			 felder[i]->type == PX_Field_Type_BinBLOB)
		{
			void *blob = malloc(felder[i]->size);
			int s_size, d_size;
			void *s = NULL, *qqstr;

			BLOCK_COPY(blob, felder[i]->size);

			PXBLOBtoBinary(blob, felder[i]->size, blobname, &s, &s_size);

			qqstr = binary_to_sql(s ? s : (char *)blob, s_size, &d_size);
			fwrite("'", 1, 1, stdout);
			fwrite(qqstr, d_size, 1, stdout);
			fwrite("'", 1, 1, stdout);

			if (s) free(s);
			free(qqstr);
			free(blob);
		}
		else
		{
			printf("\nUnknown: %d\n",felder[i]->size);
			block_index += felder[i]->size;
			printf("Name: %s\n", felder[i]->name);
		}
	}
#undef BLOCK_COPY
	printf(");\n");

	return 0;
}
int create_sql_dump(px_header *header, px_fieldInfo **felder, px_blocks **blocks, char *blobname, int create_table)
{
	int n,f,c = 0;

	if ( create_table == 1 )
		create_sql_CREATE(header, felder);

	n = header->firstBlock - 1;
	while (n != -1)
	{
		if (c >= header->usedBlocks)
		{
			fprintf(stderr,
				"%s.%d: Leaving here as are trying to use more blocks "
				"then registered in the header (header->usedBlocks\n"
				"Tell me if I'm wrong\n", __FILE__, __LINE__);
			return -1;
		}
		for (f = 0; f < blocks[n]->numRecsInBlock; f++)
		{
			create_sql_INSERT(header, felder, blocks[n]->records[f], blobname);
		}
		n = blocks[n]->nextBlock - 1;
		c++;
	}
	return 0;
}
	
char * str_to_sql(const unsigned char *src)
{
	unsigned int i, add;
	char * dst = NULL;

  /* count the numbers of ' */
	for (i = 0, add = 0; src[i]; i++)
	{
		if (src[i] == '\'' ||
		    src[i] == '\\') add++;
	}

	dst = malloc((i + add + 1) * sizeof(char));

	for (i = 0; *src; src++)
	{
		unsigned int c = 0;

		if (*src == '\'')
		{
			switch (dbtype)
			{
			    case DB_PGSQL: c = '\''; break;
			    case DB_MYSQL: c = '\\'; break;
			}
		}
		if (*src == '\\')
		{
			switch (dbtype)
			{
			    case DB_PGSQL: c = '\\'; break;
			    case DB_MYSQL: c = '\\'; break;
			}
		}

		if (c)
			dst[i++] = c;

		dst[i++] = *src;
	}
	dst[i] = '\0';
	
	return dst;
}
	
char * binary_to_sql(const unsigned char *src, int src_len, int *dst_len)
{
	unsigned int i, add;
	char * dst = NULL;

  /* count the numbers of ' */
	for (i = 0, add = 0; i < src_len; i++)
	{
		if (src[i] == '\'' ||
		    src[i] == '\\') add++;
	}

	dst = malloc((i + add) * sizeof(char));
	*dst_len = i + add;

	for (i = 0; i < src_len; src++)
	{
		unsigned int c = 0;

		if (*src == '\'')
		{
			switch (dbtype)
			{
			    case DB_PGSQL: c = '\''; break;
			    case DB_MYSQL: c = '\\'; break;
			}
		}
		if (*src == '\\')
		{
			switch (dbtype)
			{
			    case DB_PGSQL: c = '\\'; break;
			    case DB_MYSQL: c = '\\'; break;
			}
		}

		if (c)
			dst[i++] = c;

		dst[i++] = *src;
	}

	return dst;
}

char *quote(const unsigned char *src, const unsigned int name_quoting)
{
	unsigned int len;
	char *dst = NULL;

	len = (name_quoting) ? strlen(src) + 2 : strlen(src);
	dst = malloc((len + 1) * sizeof(char));
	(name_quoting) ? (void) sprintf(dst, "\"%s\"", src) : strcpy(dst, src);

	return dst;
}

void display_help ()
{
	printf(
	       "PXSQLdump - (%s - %s)\n"
	       "Usage: pxsqldump [OPTION]...\n"
	       "\n"
	       "Create a SQL-dump from a Paradox-database-file\n"
	       "\n"
	       "Options:\n"
	       "  -b, --blobname=<name>        Name of the .MB-file <name>\n"
	       "  -d, --database=<mysql|pgsql> Database compatible SQL\n"
	       "  -f, --filename=<name>        Name of the .DB-file <name>\n"
	       "  -h, --help                   Display this help and exit\n"
	       "  -n, --tablename=<name>       Replace the tablename by <name>\n"
	       "  -q, --no_namequoting         Force name quoting deactivation\n"
	       "  -Q, --namequoting            Force name quoting activation\n"
	       "  -s, --no_create              Skip table creation (insert data only)\n"
	       "  -V, --version                Output version information and exit\n"
	       "\n"
	       "\n", PACKAGE, VERSION);
}

void display_version()
{
	printf(
	       "PXSQLdump - (%s - %s)\n"
	       "Written by Jan Kneschke.\n"
	       "\n"
	       "\n", PACKAGE, VERSION);
}

int main ( int argc, char **argv)
{
	px_header header;
	px_fieldInfo **felder;
	px_blocks **blocks;

	int f,i,settablename = 0;
	int create_table = 1;
	char tablename[255] = "";
	char *blobname = NULL, *filename = NULL;
#ifdef HAVE_GETOPT_LONG
	static struct option long_options[] =
	{
		{"blobname", required_argument, 0, 'b'},
		{"database", required_argument, 0, 'd'},
		{"filename", required_argument, 0, 'f'},
		{"help", no_argument, 0, 'h'},
		{"tablename", required_argument, 0, 'n'},
		{"no_namequoting", no_argument, 0, 'q'},
		{"namequoting", no_argument, 0, 'Q'},
		{"no_create", no_argument, 0, 's'},
		{"version", no_argument, 0, 'V'},
		{0, 0, 0, 0}
	};
#endif
	while ( 
#ifdef HAVE_GETOPT_LONG
		(i = getopt_long(argc, argv, "b:d:f:hn:qQsV", long_options, (int *) 0)) != EOF
#else
		(i = getopt(argc, argv, "b:d:f:hn:qQsV")) != EOF
#endif
		)
	{
		switch (i)
		{
		    case 'Q' :
			name_quoting = 1;
			break;
		    case 'q' :
			name_quoting = 0;
			break;
		    case 's' :
			create_table = 0;
			break;
		    case 'n' :
			settablename = 1;
			if (strlen(optarg) > sizeof(tablename)-1)
			{
				fprintf(stderr, "tablename too longn\n");
				exit (-1);
			}
			strcpy(tablename, optarg);
			break;
		    case 'b' :
			blobname = optarg;
			break;
		    case 'f' :
			filename = optarg;
			break;
		    case 'd' :
			if (strcmp(optarg, OPT_MYSQL)&&strcmp(optarg, OPT_PGSQL))
			{
				fprintf(stderr, "database type must be in {mysql, pgsql}\n");
				exit(-1);
			}
			if (!strcmp(optarg, OPT_MYSQL))
				dbtype = DB_MYSQL;
			else if (!strcmp(optarg, OPT_PGSQL))
				dbtype = DB_PGSQL;
			else
				dbtype = DB_OTHER;
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

	if (strlen(tablename) > 75)
	{
		printf("specified tablename is too long\n");
		return -1;
	}

	switch(dbtype)
	{
	    case DB_MYSQL:
		if (name_quoting == -1) name_quoting = 0;
		break;
	    case DB_PGSQL:
		if (name_quoting == -1) name_quoting = 1;
		break;
	    case DB_OTHER:
		if (name_quoting == -1) name_quoting = 1;
		break;
	    default:
		printf("you have to specify a database -> -d\n");
		return -1;
	}

	if (filename)
	{
#ifdef O_BINARY
		f = open (filename, O_RDONLY|O_BINARY);
#else
		f = open (filename, O_RDONLY);
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
		printf("File '%s' is not a paradox-file\n", filename);
		return -1;
	}

	blocks = PXparseBlocks(f, &header);

	close (f);
	if (settablename) strcpy(header.tableName,tablename);

	if (!blocks)
	{
		printf("Keine Spalten\n");
		return(-1);
	}

	create_sql_dump (&header, felder, blocks, blobname, create_table);

	return 0;
}

