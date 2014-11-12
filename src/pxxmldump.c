#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "pxtypes.h"
#include "pxparse.h"
#include "pxconvert.h"

const char *tags [] = {
  "alpha",
  "date",
  "shortint",
  "longint",
  "currency",
  "number",
  "memoblob",
  "logical",
  "timestamp",
  "incremental",
  "bcd"
  "time",
};

const char * get_xml_tag(px_fieldInfo **felder, int i) {
  switch (felder[i]->type) {
  case PX_Field_Type_Alpha:	return tags[0];
  case PX_Field_Type_Date:	return tags[1];
  case PX_Field_Type_ShortInt:	return tags[2];
  case PX_Field_Type_LongInt:	return tags[3];
  case PX_Field_Type_Currency:	return tags[4];
  case PX_Field_Type_Number:	return tags[5];
  case PX_Field_Type_MemoBLOB:	return tags[6];
  case PX_Field_Type_Logical:	return tags[7];
  case PX_Field_Type_Time:	return tags[11];
  case PX_Field_Type_Timestamp:	return tags[8];
  case PX_Field_Type_Incremental:	return tags[9];
  case PX_Field_Type_BCD:		return tags[10];
  default: printf("Unknown: %02x",felder[i]->type); break;
  }

  return "";
}
int create_xml_line(px_header *header, px_fieldInfo **felder, px_records block, char *blobname) {
	int i, block_index = 0;

#define BLOCK_COPY(x,size) \
	memcpy(x,   block + block_index, size); \
	/*printf("\nBl-IDX1: %04x - ",block_index);*/ \
	block_index += size; 
	
	printf("  <row>\n");
	for (i = 0; i < header->numFields; i++ ) {
		printf("    <%s name=\"%s\">", get_xml_tag(felder, i), felder[i]->name);
		if (felder[i]->type == PX_Field_Type_Alpha) {
			char *str = malloc(felder[i]->size + 1);
			int j;
			BLOCK_COPY(str, felder[i]->size);
			str[felder[i]->size] = '\0';
			
			for (j = 0; str[j] != '\0'; j++) {
				switch(str[j]) {
				case '<': puts("&lt;"); break;
				case '>': puts("&gt;"); break;
				case '&': puts("&amp;"); break;
					
				default:  putchar(str[j]);
				}
			}
			free(str);
    } else if (felder[i]->type == PX_Field_Type_BCD) {
      char *str = malloc(felder[i]->size + 1);
			
      fprintf(stderr,"aslhföasdhfsadbf\n"),
			
	BLOCK_COPY(str, felder[i]->size);
			
      printf("%s", str);
			
      free(str);
    } else if (felder[i]->type == PX_Field_Type_ShortInt) {
      unsigned short s;
      unsigned long long d;
			
      BLOCK_COPY(&s, felder[i]->size);
			
      switch(PXtoLong(s, &d, felder[i]->type)) {
      case VALUE_OK:
	printf("%Ld", d );
	break;
      case VALUE_IS_NULL:
	break;
      default:
	break;
      }
			
    } else if (felder[i]->type == PX_Field_Type_Logical) {
      unsigned char s;
      unsigned long long d;
			
      BLOCK_COPY(&s, felder[i]->size);
			
      switch(PXtoLong(s, &d, felder[i]->type)) {
      case VALUE_OK:
	printf("%Lx", d);
	break;
      case VALUE_IS_NULL:
	break;
      default:
	break;
      }
    } else if (felder[i]->type == PX_Field_Type_LongInt) {
      unsigned long s;
      unsigned long long d;
			
      BLOCK_COPY(&s, felder[i]->size);
			
      switch(PXtoLong(s, &d, felder[i]->type)) {
      case VALUE_OK:
	printf("%Ld", d );
	break;
      case VALUE_IS_NULL:
	break;
      default:
	break;
      }
    } else if (felder[i]->type == PX_Field_Type_Incremental) {
      unsigned long s;
      unsigned long long d;
			
      BLOCK_COPY(&s, felder[i]->size);
			
      switch(PXtoLong(s, &d, felder[i]->type)) {
      case VALUE_OK:
	printf("%Ld", d );
	break;
      case VALUE_IS_NULL:
	break;
      default:
	break;
      }
    } else if (felder[i]->type == PX_Field_Type_Number) {
      unsigned long long s;
      double d;
			
      BLOCK_COPY(&s, felder[i]->size);
			
      switch(PXtoDouble(s, &d, felder[i]->type)) {
      case VALUE_OK:
	printf("%f", d );
	break;
      case VALUE_IS_NULL:
	break;
      default:
	break;
      }
			
    } else if (felder[i]->type == PX_Field_Type_Currency) {
      unsigned long long s;
      double d;
			
      BLOCK_COPY(&s, felder[i]->size);
			
      switch(PXtoDouble(s, &d, felder[i]->type)) {
      case VALUE_OK:
	printf("%f", d );
	break;
      case VALUE_IS_NULL:
	break;
      default:
	break;
      }
    } else if (felder[i]->type == PX_Field_Type_Date) {
      unsigned long long s;
      struct tm _tm;
			
      BLOCK_COPY(&s, felder[i]->size);

      switch (PXtoTM(s, &_tm, felder[i]->type)) {
      case VALUE_OK:
	printf("'%04i-%02i-%02i'", _tm.tm_year+1900, _tm.tm_mon+1, _tm.tm_mday);
	break;
      case VALUE_IS_NULL:
	break;
      default:
	break;
      }

    } else if (felder[i]->type == PX_Field_Type_Time) {
      unsigned long long s;
      struct tm _tm;
			
      BLOCK_COPY(&s, felder[i]->size);

      switch (PXtoTM(s, &_tm, felder[i]->type)) {
      case VALUE_OK:
	printf("%02i:%02i:%02i",_tm.tm_hour, _tm.tm_min, _tm.tm_sec);
	break;
      case VALUE_IS_NULL:
	break;
      default:
	break;
      }
    } else if (felder[i]->type == PX_Field_Type_Timestamp) {
      unsigned long long s;
      struct tm _tm;
			
      BLOCK_COPY(&s, felder[i]->size);

      switch (PXtoTM(s, &_tm, felder[i]->type)) {
      case VALUE_OK:
	printf("%04i-%02i-%02i ", _tm.tm_year+1900, _tm.tm_mon+1, _tm.tm_mday);
	printf("%02i:%02i:%02i",_tm.tm_hour, _tm.tm_min, _tm.tm_sec);
	break;
      case VALUE_IS_NULL:
	break;
      default:
	break;
      }
    } else if (felder[i]->type == PX_Field_Type_MemoBLOB) {
      void *blob = malloc(felder[i]->size);
      char *s = NULL;
			
      BLOCK_COPY(blob, felder[i]->size);
			
      s = PXMEMOtoString(blob, felder[i]->size, blobname);
			
			
      if (s != NULL) {
	printf("%s",s);
	free(s);
      } else {
	printf("%s",(char *)blob);
      }
      free(blob);
    } else {
      printf("\nUnknown: %d\n",felder[i]->size);
      block_index += felder[i]->size;
      printf("Name: %s\n", felder[i]->name);
    }
    printf("</%s>\n", get_xml_tag(felder, i));
  }
#undef BLOCK_COPY
  printf("  </row>\n");
	
  return 0;
}
int create_xml_dump(px_header *header, px_fieldInfo **felder, px_blocks **blocks, char *blobname) {
  int n,f,c = 0;

  printf("<?xml version=\"1.0\"?>\n");
  printf("<!DOCTYPE dump PUBLIC \"http://jan.kneschke.de/projects/pxtools/pxtools.dtd\" \"pxtools.dtd\">\n");

  printf("<dump>\n");
  n = header->firstBlock - 1;
  while (n != -1) {
    if (c >= header->usedBlocks) {
      fprintf(stderr, 
	      "%s.%d: Leaving here as are trying to use more blocks "
	      "then registered in the header (header->usedBlocks\n"
	      "Tell me if I'm wrong\n", __FILE__, __LINE__);
      break;
    }
    for (f = 0; f < blocks[n]->numRecsInBlock; f++) {
      create_xml_line(header, felder, blocks[n]->records[f], blobname);
    }
    n = blocks[n]->nextBlock - 1;
    c++;
  }
  printf("</dump>\n");
  return 0;
}

void display_help () {
  printf(
"PXXMLdump - (%s - %s)\n"
"Usage: pxxmldump [OPTION]...\n"
"\n"
"Create a XML-dump from a Paradox-database-file\n"
"\n"
"Options:\n"
"  -b, --blobname=<name>  Name of the .MB-file <name>\n"
"  -f, --filename=<name>  Name of the .DB-file <name>\n"
"  -h, --help             Display this help and exit\n"
"  -n, --tablename=<name> Replace the tablename by <name>\n"
"  -V, --version          Output version information and exit\n"
"\n"
"\n", PACKAGE, VERSION);
}

void display_version() {
  printf(
"PXXMLdump - (%s - %s)\n"
"Written by Jan Kneschke.\n"
"\n"
"\n", PACKAGE, VERSION);
}

int main ( int argc, char **argv) {
  px_header header;
  px_fieldInfo **felder;
  px_blocks **blocks;
	
  int f,i,settablename = 0;
  char tablename[255] = "";
  char *blobname = NULL, *filename = NULL;
#ifdef HAVE_GETOPT_LONG
  static struct option long_options[] = 
    {
      {"blobname", required_argument, 0, 'b'},
      {"filename", required_argument, 0, 'f'},
      {"help", no_argument, 0, 'h'},
      {"tablename", required_argument, 0, 'n'},
      {"version", no_argument, 0, 'V'},
      {0, 0, 0, 0}
    };
#endif
  while ( 
#ifdef HAVE_GETOPT_LONG
	  (i = getopt_long(argc, argv, "b:f:hn:V", long_options, (int *) 0)) != EOF 
#else
	  (i = getopt(argc, argv, "b:f:hn:V")) != EOF 
#endif
	  ) {	
    switch (i) {
    case 'n' : 
      settablename = 1;
      if (strlen(optarg) > sizeof(tablename)-1) {
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
	
  if (strlen(tablename) > 75) {
    printf("error: specified tablename is too long\n");
    exit(-1);
  }

  if (filename) {
#ifdef O_BINARY
    f = open (filename , O_RDONLY|O_BINARY);
#else
    f = open (filename , O_RDONLY);
#endif
    if (f == -1) {
      fprintf(stderr, "Can't open file %s: %s\n", filename, strerror(errno));
      exit(-1);
    }
  } else {
    display_help();
    exit(-1);
  }

  if (!(felder = PXparseCompleteHeader(f, &header))){
    printf("File '%s' is not a paradox-file\n", filename);
    return -1;
  }
	
  blocks = PXparseBlocks(f, &header);
	
  close (f);
  if (settablename) strcpy(header.tableName,tablename);
	
  if (!blocks) {
    printf("Keine Spalten\n");
    return(-1);
  }
	
  create_xml_dump (&header, felder, blocks, blobname);
	
  return 0;
}

