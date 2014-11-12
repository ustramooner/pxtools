#ifndef _PXPARSE_H_
#define _PXPARSE_H_

#include "pxtypes.h"

px_fieldInfo**	PXparseCompleteHeader (int fd, px_header *header);
px_blocks**	PXparseBlocks (int fd, px_header *header);
int PXparseMBHeader(int fd);
#endif
