#ifndef _PXCONVERT_H_
#define _PXCONVERT_H_

#include <time.h>

#define VALUE_IS_NULL	10
#define VALUE_OK	0
#define VALUE_ERROR	-1

int	PXtoLong(unsigned long long number, unsigned long long *ret, int type);
int 	PXtoDouble(unsigned long long number, double *ret, int type);
int	PXtoTM (unsigned long long number, struct tm *tm, int type);
int	PXtoQuotedString(char *dst, const unsigned char *src, int type);
char *	PXNametoQuotedName(char *str);
char *	PXMEMOtoString(void *blob, int size, char *blobname);
int     PXBLOBtoBinary(void *blob, int size, char *blobname, void ** binstorage, int *binsize);

void copy_from_le(void *dst, const char *src, int len);
void copy_from_be(void *dst, const char *src, int len);

#endif
