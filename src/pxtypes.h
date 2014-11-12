#ifndef _PARADOX_H_
#define _PARADOX_H_

#include "config.h"

#ifdef HAVE_GETTEXT
#include <libintl.h>
#include <locale.h>
#define _(String) gettext(String)
#else
#define _(String) String
#endif

/* void * are escaped for the 64bit machines 
 * otherwise they are too long (8bytes)
 */

typedef struct {
	signed short recordSize;		/* 0x00 */
	signed short headerSize;		/* 0x02 */
	unsigned char fileType;			/* 0x04 */
	unsigned char maxTableSize;		/* 0x05 */
	unsigned int numRecords;		/* 0x06 */ 
	unsigned short usedBlocks;		/* 0x0a */
	unsigned short fileBlocks;		/* 0x0c */
	unsigned short firstBlock;		/* 0x0e */
	unsigned short lastBlock;		/* 0x10 */
	unsigned short dummy_1;			/* 0x12 */
	unsigned char modifiedFlags1;		/* 0x14 */
	unsigned char IndexFieldNumber;		/* 0x15 */
/* void * */	unsigned int primaryIndexWorkspace;		/* 0x16 */
/* void * */	unsigned int dummy_2;				/* 0x1a */
	unsigned short indexRootBlock;		/* 0x1e */
	unsigned char indexLevels;		/* 0x20 */
	signed short numFields;			/* 0x21 */
	signed short primaryKeyFields;		/* 0x23 */	
	unsigned int encryption1;		/* 0x25 */
	unsigned char sortOrder;		/* 0x29 */
	unsigned char modifiedFlags2;		/* 0x2a */
	unsigned short dummy_5;			/* 0x2b */
	unsigned char changeCount1;		/* 0x2d */
	unsigned char changeCount2;		/* 0x2e */
	unsigned char dummy_6;			/* 0x2f */
/* char ** */ 	unsigned int tableNamePtr;			/* 0x30 */
/* void * */	unsigned int fieldInfo;				/* 0x34 */
	unsigned char writeProtected;		/* 0x38 */
	unsigned char fileVersionID;		/* 0x39 */
	unsigned short maxBlocks;		/* 0x3a */
	unsigned char dummy_7;			/* 0x3c */
	unsigned char auxPasswords;		/* 0x3d */
	unsigned short dummy_8;			/* 0x3e */
/* void * */	unsigned int cryptInfoStart;			/* 0x40 */
/* void * */	unsigned int cryptInfoEnd;			/* 0x44 */
	unsigned char dummy_9;			/* 0x48 */
	unsigned int autoInc;			/* 0x49 */
	unsigned short dummy_a;			/* 0x4d */
	unsigned char indexUpdateRequired;	/* 0x4f */
	unsigned int dummy_b;			/* 0x50 */
	unsigned char dummy_c;			/* 0x54 */
	unsigned char refIntegrity;		/* 0x55 */
	unsigned short dummy_d;			/* 0x56 */
	unsigned short fileVersionID2;		/* 0x58 */
	unsigned short fileVersionID3;		/* 0x5a */
	unsigned int encryption2;		/* 0x5c */
	unsigned int fileUpdateTime;		/* 0x60 */
	unsigned short hiFieldID;		/* 0x64 */
	unsigned short hiFieldIDInfo;		/* 0x66 */
	unsigned short sometimesNumFields;	/* 0x68 */
	unsigned short dosGlobalCodePage;	/* 0x6a */
	unsigned int dummy_e;			/* 0x6c */
	unsigned short changeCount4;		/* 0x70 */
	unsigned int dummy_f;			/* 0x72 */
	unsigned short dummy_10;		/* 0x76 */
	
	char tableName[79];			/* ---- */
} px_header;

typedef struct  {
	char name[80];
	int type;
	int size;
} px_fieldInfo;

typedef char*	px_records;

typedef struct {
	int prevBlock;
	int nextBlock;
	int numRecsInBlock;
	px_records *records;
} px_blocks;

#define PX_Field_Type_Alpha	0x01
#define PX_Field_Type_Date	0x02
#define PX_Field_Type_ShortInt	0x03
#define PX_Field_Type_LongInt	0x04
#define PX_Field_Type_Currency	0x05
#define PX_Field_Type_Number	0x06
#define PX_Field_Type_Logical	0x09
#define PX_Field_Type_MemoBLOB	0x0c
#define PX_Field_Type_BinBLOB	0x0d
#define PX_Field_Type_Graphic	0x10
#define PX_Field_Type_Time	0x14
#define PX_Field_Type_Timestamp	0x15
#define PX_Field_Type_Incremental 0x16
#define PX_Field_Type_BCD	0x17


#define PX_Filetype_DB_Indexed		0x00
#define PX_Filetype_PX			0x01
#define PX_Filetype_DB_Not_indexed	0x02
#define PX_Filetype_Xnn_NonInc		0x03
#define PX_Filetype_Ynn			0x04
#define PX_Filetype_Xnn_Inc		0x05
#define PX_Filetype_XGn_NonInc		0x06
#define PX_Filetype_YGn			0x07
#define PX_Filetype_XGn_Inc		0x08


#endif
