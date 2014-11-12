#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <libintl.h>
#include <string.h>
#include "pxtypes.h"
#include "pxparse.h"
#include "pxconvert.h"

/* #define DEBUG */

int PXparseHeader(char *unp_head, px_header *header) {
	int i = 0;
#ifdef DEBUG_HEAD_COPY
#define HEAD_COPY(x) \
	copy_from_le(&(header->x), unp_head + i, sizeof(header->x)); \
	if (sizeof(header->x) == 1) \
		printf("Index: 0x%02x + %i (%s):\t %02x\n",i,sizeof(header->x),#x, header->x); \
	else if (sizeof(header->x) == 2) \
		printf("Index: 0x%02x + %i (%s):\t %04x\n",i,sizeof(header->x),#x, header->x); \
	else if (sizeof(header->x) == 4) \
		printf("Index: 0x%02x + %i (%s):\t %08x\n",i,sizeof(header->x),#x, header->x); \
	else \
		printf("Index: 0x%02x + %i (%s):\t %08x\n",i,sizeof(header->x),#x, header->x); \
	i += sizeof(header->x); 
#else
#define HEAD_COPY(x) \
	copy_from_le(&(header->x), unp_head + i, sizeof(header->x)); \
	i += sizeof(header->x); 
#endif
		
	HEAD_COPY(recordSize);
	HEAD_COPY(headerSize);
	HEAD_COPY(fileType);
	HEAD_COPY(maxTableSize);
	HEAD_COPY(numRecords);
	HEAD_COPY(usedBlocks);
	HEAD_COPY(fileBlocks);
	HEAD_COPY(firstBlock);
	HEAD_COPY(lastBlock);
	HEAD_COPY(dummy_1);
	HEAD_COPY(modifiedFlags1);
	HEAD_COPY(IndexFieldNumber);
	HEAD_COPY(primaryIndexWorkspace);	/* pointer */
	HEAD_COPY(dummy_2);			/* pointer */
	HEAD_COPY(indexRootBlock);
	HEAD_COPY(indexLevels);
	HEAD_COPY(numFields);
	HEAD_COPY(primaryKeyFields);
	HEAD_COPY(encryption1);
	HEAD_COPY(sortOrder);
	HEAD_COPY(modifiedFlags2);
	HEAD_COPY(dummy_5);
	HEAD_COPY(changeCount1);
	HEAD_COPY(changeCount2);
	HEAD_COPY(dummy_6);
	HEAD_COPY(tableNamePtr);		/* pointer */
	HEAD_COPY(fieldInfo);			/* pointer */
	HEAD_COPY(writeProtected);
	HEAD_COPY(fileVersionID);
	HEAD_COPY(maxBlocks);
	HEAD_COPY(dummy_7);
	HEAD_COPY(auxPasswords);
	HEAD_COPY(dummy_8);
	HEAD_COPY(cryptInfoStart);		/* pointer */
	HEAD_COPY(cryptInfoEnd);		/* pointer */
	HEAD_COPY(dummy_9);
	HEAD_COPY(autoInc);
	HEAD_COPY(dummy_a);
	HEAD_COPY(indexUpdateRequired);
	HEAD_COPY(dummy_b);
	HEAD_COPY(dummy_c);
	HEAD_COPY(refIntegrity);
	HEAD_COPY(dummy_d);
	
	return 0;
	#undef HEAD_COPY
}

int PXparseHeaderV4(char *unp_head, px_header *header) {
	int i = 0;
#ifdef DEBUG_HEAD_COPY
#define HEAD_COPY(x) \
	copy_from_le(&(header->x), unp_head + i, sizeof(header->x)); \
	if (sizeof(header->x) == 1) \
		printf("Index: 0x%02x + %i (%s):\t %02x\n",i+0x58,sizeof(header->x),#x, header->x); \
	else if (sizeof(header->x) == 2) \
		printf("Index: 0x%02x + %i (%s):\t %04x\n",i+0x58,sizeof(header->x),#x, header->x); \
	else if (sizeof(header->x) == 4) \
		printf("Index: 0x%02x + %i (%s):\t %08x\n",i+0x58,sizeof(header->x),#x, header->x); \
	else \
		printf("Index: 0x%02x + %i (%s):\t %08x\n",i+0x58,sizeof(header->x),#x, header->x); \
	i += sizeof(header->x); 
#else
#define HEAD_COPY(x) \
	copy_from_le(&(header->x), unp_head + i, sizeof(header->x)); \
	i += sizeof(header->x); 
#endif
		
	HEAD_COPY(fileVersionID2);
	HEAD_COPY(fileVersionID3);
	HEAD_COPY(encryption2);
	HEAD_COPY(fileUpdateTime);
	HEAD_COPY(hiFieldID);
	HEAD_COPY(hiFieldIDInfo);
	HEAD_COPY(sometimesNumFields);
	HEAD_COPY(dosGlobalCodePage);
	HEAD_COPY(dummy_e);
	HEAD_COPY(changeCount4);
	HEAD_COPY(dummy_f);
	HEAD_COPY(dummy_10);
	
	#undef HEAD_COPY
	return 0;
}

int isHeaderSupported (px_header *header) {

	if (!header) return 0;

	switch (header->fileVersionID) {
		case 0x03: 
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c: break;
		default: 
			fprintf(stderr, "unknown Fileversion ID\n");
			return 0;
	}

	switch (header->fileType) {
		case 0x00: 
		case 0x01: 
		case 0x02: 
		case 0x03: 
		case 0x04: 
		case 0x05: 
		case 0x06: 
		case 0x07: 
		case 0x08: break;
		default: 
			fprintf(stderr, "unknown FileType ID\n");
			return 0;
	}
	if (header->numRecords > 0 && header->firstBlock != 1) {
		fprintf(stderr, "warning: numRecords > 0 (%d) && firstBlock != 1 (%d)\n",
			header->numRecords,
			header->firstBlock);
/*		return 0;*/
	}

	return 1;
}

px_fieldInfo **PXparseCompleteHeader (int fd, px_header *header) {
	int file_index = 0;
	char unp_head[0x58];
	char unp_head4[0x20];
	unsigned char d[2];
	void *ptr;
	
	int i;
	char c;
	
	px_fieldInfo **felder;
	
	file_index += read(fd, unp_head, sizeof(unp_head));
	PXparseHeader( unp_head, header );
	
	if (! isHeaderSupported(header)) {
		fprintf(stderr, "unsupported Header\n");
		return 0;
	}
	
	if (	header->fileVersionID >= 0x05 &&
		header->fileType != 0x1 &&
		header->fileType != 0x4 &&
		header->fileType != 0x7 ) {
		
		file_index += read(fd, unp_head4, sizeof(unp_head4));
		PXparseHeaderV4 ( unp_head4, header );
	}
	
	
	felder = malloc(header->numFields * sizeof(px_fieldInfo *));
	
/* FieldType/Size (0x78) */
	for ( i = 0; i < header->numFields; i++) {
		felder[i] = malloc(sizeof(px_fieldInfo));
		file_index += read(fd,&d,sizeof(d));
		
		felder[i]->type = d[0];
		felder[i]->size = d[1];
	}
/* tablenameptr - skipped */
	file_index += read(fd, &ptr, sizeof(ptr));
	
	
/* fieldnameptr - skipped */
	if (	header->fileType == 0x0 || 
		header->fileType == 0x2 || 
		header->fileType == 0x8 ||
		header->fileType == 0x6
		) {
		for ( i = 1; i <= header->numFields;i++) {
			file_index += read(fd, &ptr, sizeof(ptr));
		}
	}
	
/* tablename */
	file_index += read(fd, header->tableName, sizeof(header->tableName));
	
	/* fix for tablename longer then 79 chars (PX 7.0)*/
	c = '\0';
	while (!c) {
		file_index += read(fd, &c, sizeof(c));
	}
	file_index--;
	lseek(fd, -1, SEEK_CUR );
	
	if (	header->fileVersionID >= 0x04 &&
		header->fileType != 0x1 &&
		header->fileType != 0x4 &&
		header->fileType != 0x7 ) {

/* FieldNames*/
		for ( i = 1; i <= header->numFields;i++) {
			int j = 0;
			c = ' ';
			while (c) {
				file_index += read(fd,&c,sizeof(c));
				felder[i-1]->name[j++] = c;
#ifdef DEBUG
				if (c) printf("%c",c);
#endif
			}
#ifdef DEBUG
			printf("\n");
#endif
		}
		if (header->auxPasswords) {
			char str[256];
			file_index += read(fd,&str,sizeof(str));
		}
/* field-position */
		for ( i = 1; i <= header->numFields;i++) {
			unsigned short s;
			file_index += read(fd,&s,sizeof(s));
#ifdef DEBUG
			printf("%i\n",s);
#endif
		}
		c = ' ';
#ifdef DEBUG
		printf("SortOrderID: ");
#endif
		while (c) {
			file_index += read(fd,&c,sizeof(c));
#ifdef DEBUG
			if (c) printf("%c",c);
#endif
		}
#ifdef DEBUG
		printf("\n");
#endif
		if (header->fileType == PX_Filetype_XGn_Inc ||
		    header->fileType == PX_Filetype_XGn_NonInc ) {
			c = ' ';
#ifdef DEBUG
			printf("IndexName: ");
#endif
			while (c) {
				file_index += read(fd,&c,sizeof(c));
#ifdef DEBUG
				if (c) printf("%c",c);
#endif
			}
#ifdef DEBUG
			printf("\n");
#endif
		}
		
	}
	
#ifdef DEBUG
	printf( ">HeaderSize: %04x<\n",header->headerSize);
#endif
	while (file_index < header->headerSize) {
		file_index += read(fd,&c,sizeof(c));
	}
	
	return felder;
}

px_blocks **PXparseBlocks (int fd, px_header *header) {
	int n,i;
	/* support for maxTableSize == 16 */
	unsigned char block[4096*8];
	px_blocks **blocks = NULL;
	
	int block_nr = 0;
	int file_index = lseek(fd, 0, SEEK_CUR);
	
	unsigned short nextBlock, prevBlock;

#ifdef DEBUG	
	printf("FD-IDX: %x\n",file_index);
#endif
	blocks = malloc( sizeof (px_blocks *) * header->fileBlocks);
	
	while ((n = read(fd,&block,0x400 * header->maxTableSize )) > 0) {
		signed short addDataSize, numRecsInBlock;
		int block_index = 0;
		
		file_index += n;

#ifdef DEBUG	
		printf("FD-IDX: %x\n",file_index);
	#define BLOCK_COPY_S(x,size) \
		copy_from_le(x,   block + block_index, size); \
		printf("Bl-IDX1: %04x\n",block_index); \
		block_index += size; 
	#define BLOCK_COPY(x,size) \
		memcpy(x,   block + block_index, size); \
		printf("Bl-IDX1: %04x\n",block_index); \
		block_index += size; 
#else
	#define BLOCK_COPY_S(x,size) \
		copy_from_le(x,   block + block_index, size); \
		block_index += size; 
	#define BLOCK_COPY(x,size) \
		memcpy(x,   block + block_index, size); \
		block_index += size; 
#endif
		
		BLOCK_COPY_S(&nextBlock, sizeof(nextBlock));
		BLOCK_COPY_S(&prevBlock, sizeof(prevBlock));
		BLOCK_COPY_S(&addDataSize, sizeof(addDataSize));
		
		numRecsInBlock = (addDataSize / header->recordSize) + 1;
#ifdef DEBUG		
		printf("NextBlock:\t%04x\n",nextBlock);
		printf("PrevBlock:\t%04x\n",prevBlock);
		printf("AddDataSize:\t%d\n",addDataSize);
		printf("numRecsInBlock:\t%d\n",numRecsInBlock);
#endif
		blocks[block_nr] = malloc( sizeof (px_blocks) );
		blocks[block_nr]->prevBlock = prevBlock;
		blocks[block_nr]->nextBlock = nextBlock;
		blocks[block_nr]->records = malloc(numRecsInBlock * sizeof(px_records *));
		blocks[block_nr]->numRecsInBlock = numRecsInBlock;
		
		for (i = 0; i < numRecsInBlock; i++ ) {
			blocks[block_nr]->records[i] = (px_records)malloc(header->recordSize);
			BLOCK_COPY(blocks[block_nr]->records[i], header->recordSize);
		}
#ifdef DEBUG
		printf("Curr: %04x. Prev: %04x; Next: %04x\n",block_nr+1,prevBlock,nextBlock);
#endif
		block_nr++;
	}
	
	if (block_nr != header->fileBlocks) {
		printf("Berr: %i -> %i\n",block_nr, header->fileBlocks);
	}
	
	if (n == -1) {
		perror("Fehler beim Lesen der Spalten");
	}
#ifdef DEBUG	
	n = header->firstBlock - 1;

	for (i = 0; i < header->usedBlocks; i++) {
		printf("%04x <- %04x -> %04x\n",blocks[n]->prevBlock, n+1, blocks[n]->nextBlock);
		n = blocks[n]->nextBlock - 1;
	}
#endif
	#undef BLOCK_COPY
	#undef BLOCK_COPY_S
	
	return blocks;
}

typedef struct {
	unsigned char type;
	unsigned short num_blocks;
	unsigned short mod_count;
	unsigned short size_block;
	unsigned short size_sub_block;
	unsigned char size_sub_chunk;
	unsigned short num_sub_chunk;
	unsigned short thres_sub_chunk;
} mb_type0;

int PXparseMBType0 (const char *unp_head) {
	int i = 0;
	mb_type0 *header = NULL;
	
	header = malloc (sizeof(mb_type0));
#ifdef DEBUG_HEAD_COPY
#define HEAD_COPY(x) \
	copy_from_le(&(header->x), unp_head + i, sizeof(header->x)); \
	if (sizeof(header->x) == 1) \
		printf("Index: 0x%02x + %i (%s):\t %02x\n",i,sizeof(header->x),#x, header->x); \
	else if (sizeof(header->x) == 2) \
		printf("Index: 0x%02x + %i (%s):\t %04x\n",i,sizeof(header->x),#x, header->x); \
	else if (sizeof(header->x) == 4) \
		printf("Index: 0x%02x + %i (%s):\t %08x\n",i,sizeof(header->x),#x, header->x); \
	else \
		printf("Index: 0x%02x + %i (%s):\t %08x\n",i,sizeof(header->x),#x, header->x); \
	i += sizeof(header->x); 
#else
#define HEAD_COPY(x) \
	copy_from_le(&(header->x), unp_head + i, sizeof(header->x)); \
	i += sizeof(header->x); 
#endif	
	
	HEAD_COPY(type);
	HEAD_COPY(num_blocks);
	HEAD_COPY(mod_count);
	
	/* jump to 0xb */
	i = 0xb;
	
	HEAD_COPY(size_block);
	HEAD_COPY(size_sub_block);
	
	/* jump to 0x10 */
	i = 0x10;
	
	HEAD_COPY(size_sub_chunk);
	HEAD_COPY(num_sub_chunk);
	HEAD_COPY(thres_sub_chunk);
	
#ifdef DEBUG
	fprintf(stderr, "type: %d\n", header->type);
	fprintf(stderr, "num_blocks: %d\n", header->num_blocks);
	fprintf(stderr, "mod_count: %d\n", header->mod_count);
	fprintf(stderr, "size_blocks: %04x\n", header->size_block);
	fprintf(stderr, "size_sub_blocks: %04x\n", header->size_sub_block);
	fprintf(stderr, "size_sub_chunk: %04x\n", header->size_sub_chunk);
	fprintf(stderr, "num_sub_chunk: %04x\n", header->num_sub_chunk);
	fprintf(stderr, "thres_sub_chunk: %04x\n", header->thres_sub_chunk);
#endif
	free(header);
#undef HEAD_COPY

	return 0;
}

typedef struct {
	unsigned char offset;
	unsigned char length_div_16;
	unsigned short mod_count;
	unsigned char length_mod_16;
} mb_type3_pointer;

typedef struct {
	unsigned char type;
	unsigned short num_blocks;
	
	mb_type3_pointer blob_pointer[64];
} mb_type3;

int PXparseMBType3 (const char *unp_head) {
	int i = 0, j;
	mb_type3 *header = NULL;
	
	header = malloc (sizeof(mb_type3));
#ifdef DEBUG_HEAD_COPY
#define HEAD_COPY(x) \
	copy_from_le(&(header->x), unp_head + i, sizeof(header->x)); \
	if (sizeof(header->x) == 1) \
		printf("Index: 0x%02x + %i (%s):\t %02x\n",i,sizeof(header->x),#x, header->x); \
	else if (sizeof(header->x) == 2) \
		printf("Index: 0x%02x + %i (%s):\t %04x\n",i,sizeof(header->x),#x, header->x); \
	else if (sizeof(header->x) == 4) \
		printf("Index: 0x%02x + %i (%s):\t %08x\n",i,sizeof(header->x),#x, header->x); \
	else \
		printf("Index: 0x%02x + %i (%s):\t %08x\n",i,sizeof(header->x),#x, header->x); \
	i += sizeof(header->x); 
#else
#define HEAD_COPY(x) \
	copy_from_le(&(header->x), unp_head + i, sizeof(header->x)); \
	i += sizeof(header->x); 
#endif	
	
	HEAD_COPY(type);
	HEAD_COPY(num_blocks);
	
	i = 12;
	
	for (j = 0; j < 64; j++) {
#ifdef DEBUG
		fprintf(stderr, "Index (%d): %x\n", j, i);
#endif
		HEAD_COPY(blob_pointer[j].offset);
		HEAD_COPY(blob_pointer[j].length_div_16);
		HEAD_COPY(blob_pointer[j].mod_count);
		HEAD_COPY(blob_pointer[j].length_mod_16);
	}
#ifdef DEBUG
	fprintf(stderr, "Index: %x\n", i);

	fprintf(stderr, "type: %d\n", header->type);
	fprintf(stderr, "num_blocks: %d\n", header->num_blocks);
#endif
	/* jump to 0x150. start of the chunks */
	
	i = 0x150;
	
	for (j = 0; j < 64; j++) {
		int start = header->blob_pointer[j].offset * 16;
		int length = header->blob_pointer[j].length_div_16 * 16 + header->blob_pointer[j].length_mod_16;
		char *string;
#ifdef DEBUG
		fprintf(stderr, "offset      [%d]: %d\n", j, header->blob_pointer[j].offset);
		fprintf(stderr, "length / 16 [%d]: %d\n", j, header->blob_pointer[j].length_div_16);
		fprintf(stderr, "mod_count   [%d]: %d\n", j, header->blob_pointer[j].mod_count);
		fprintf(stderr, "length %% 16 [%d]: %d\n", j, header->blob_pointer[j].length_mod_16);
#endif		
		string = malloc(length + 1);
		
		memcpy(string, unp_head + start, length);
		string[length] = '\0';
		
		if (length > 0) {
			fprintf(stderr, "Offset: %d, Length: %d\n", start, length);
			fprintf(stderr, "String      [%d]: %s\n", j, string);
		}
		
		i += length;
		
		free(string);
	}
	
	free(header);
#undef HEAD_COPY

	return 0;
}
int PXparseMBHeader(int fd) {
	int file_index = 0, n;
	unsigned char mb_type = 0;
	unsigned short mb_num_blocks = 0;
	
	char unp_header[4096];
	
/* rewind to the first byte */
	lseek(fd, 0, SEEK_SET);
	
	while ((n = read(fd, unp_header, sizeof(unp_header)) ) > 0) {
		file_index += n;
	
		copy_from_le(&mb_type, unp_header, 1);
		copy_from_le(&mb_num_blocks, unp_header+1, 2);
	
		switch (mb_type) {
			case 00: /* mb header */
				PXparseMBType0(unp_header);
				break;
			case 02: break;
			case 03: 
				PXparseMBType3(unp_header);
				break;
			case 04: break;
			default:
				return -1;
		}
	}
	
	return 0;
}
