#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <libintl.h>
#include "config.h"
#include "pxtypes.h"
#include "pxconvert.h"


/*************************
   taken from c't 1997 - 15
   ftp://ftp.heise.de/pub/ct/ct9715.zip
   
   input:   julian date
            (day since 1.1.4713 before Chr.)
   output:  year
            month (1=Jan, 2=Feb, ... 12 = Dec)
            day   (1...31)
   modified algorithm by R. G. Tantzen
*/
void gdate(long jd, int *jahr, int *monat, int *tag) {
	long j,m,t;
	jd -= 1721119L;

	j  = (4L*jd-1L) / 146097L;
	jd = (4L*jd-1L) % 146097L;
	t  = jd/4L;

	jd = (4L*t+3L) / 1461L;
	t  = (4L*t+3L) % 1461L;
	t  = (t+4L)/4L;

	m  = (5L*t-3L) / 153L;
	t  = (5L*t-3L) % 153L;
	t  = (t+5L)/5L;

	j  = 100L*j + jd;
	if ( m < 10L ) {
		m+=3;
	} else {
		m-=9;
		j++;
	}
	
	*jahr	= (int)j;
	*monat	= (int)m;
	*tag	= (int)t;
}
long jdatum(int jahr, int monat, int tag) {
	long c, y;
	if (monat > 2) {
		monat -= 3;
	} else {
		monat += 9;
		jahr--;
	}
	tag += (153*monat+2)/5;
	c = (146097L*(((long)jahr) / 100L))/4L;
	y =   (1461L*(((long)jahr) % 100L))/4L;
	return c+y+(long)tag+1721119L;
}
/*
   taken for c't 1997 - 15
**************************/

void copy_from_be(void *_dst, const char *src, int len) {
	int i;
	char *dst = _dst;
	for (i = 0; i < len; i++) {
#ifdef WORDS_BIGENDIAN
		dst[i] = src[i];
#else
		dst[len-i-1] = src[i];
#endif
	}
}

void copy_from_le(void *_dst, const char *src, int len) {
	int i;
	char *dst = _dst;
	for (i=0;i < len;i++) {
#ifdef WORDS_BIGENDIAN
		dst[len-i-1] = src[i];
#else
		dst[i] = src[i];
#endif
	} 
}

void fix_sign (char *dst, int len) {
#ifdef WORDS_BIGENDIAN
	dst[0] &= 0x7f;
#else
	dst[len-1]  &= 0x7f;
#endif
}

void set_sign (char *dst, int len) {
#ifdef WORDS_BIGENDIAN
	dst[0] |= 0x80;
#else
	dst[len-1]  |= 0x80;
#endif
}

int PXtoLong(unsigned long long number, unsigned long long *ret, int type) {
	unsigned long long retval = 0;
	char *s = (char *)&number;
	char *d = (char *)&retval;
	
	switch (type) {
		case PX_Field_Type_Logical:
			copy_from_be(d,s,1);
			
			if (s[0] & 0x80) {
				fix_sign(d, 1);
			} else if (retval == 0) {
				return VALUE_IS_NULL;
			} else {
				set_sign(d, 1);
				retval = (char )retval;
			}
			break;
		
		case PX_Field_Type_ShortInt: 
			copy_from_be(d,s,2);
			
			if (s[0] & 0x80) {
				fix_sign(d, 2);
			} else if (retval == 0) {
				return VALUE_IS_NULL;
			} else {
				set_sign(d, 2);
				retval = (short)retval;
			}
			break;
		case PX_Field_Type_Incremental:
		case PX_Field_Type_LongInt: 
			copy_from_be(d,s,4);
			
			if (s[0] & 0x80) {
				fix_sign(d, 4);
			} else if (retval == 0) {
				return VALUE_IS_NULL;
			} else {
				set_sign(d, 4);
				retval = (long )retval;
			}
			break;
		default:
			fprintf(stderr,"%s.%d: Can't convert type (%02x)!\n",
				__FILE__, __LINE__,
				type);
			return VALUE_ERROR;
		
	}
	*ret = retval;
	return VALUE_OK;
}

int PXtoDouble(unsigned long long number, double *ret, int type) {
	double retval = 0;
	unsigned char *s = (unsigned char *)&number;
	unsigned char *d = (unsigned char *)&retval;
	
	switch (type) {
	case PX_Field_Type_Currency: 
	case PX_Field_Type_Number: 
		copy_from_be(d,s,8);
		
		if (s[0] & 0x80) {
			/* positive */
			fix_sign(d, 8);
		} else if (retval == 0) {
			return VALUE_IS_NULL;
		} else {
			int i;
			for (i=0; i<8; i++)
				d[i] = ~d[i];
		}
		break;
	default:
		fprintf(stderr,"%s.%d: Can't convert type (%02x)!\n",
			__FILE__, __LINE__,
			type);
		return VALUE_ERROR;
	}
	*ret = retval;
	return VALUE_OK;
}

int PXtoTM (unsigned long long number, struct tm *tm, int type ) {
	unsigned long long retval = 0;
	time_t t = 0;
	char *s = (char *)&number;
	char *d = (char *)&retval;
	long jd;
	int y,m,dy;
	
	struct tm *_tm = tm;
	
	switch (type) {
		case PX_Field_Type_Date: 
			copy_from_be(d,s,4);
			
			if (s[0] & 0x80) {
				fix_sign(d, 4);
			} else if (retval == 0) {
				return VALUE_IS_NULL;
			} else {
				fprintf(stderr,"%s.%d: DATE can't be nagative\n",
					__FILE__, __LINE__);
				return VALUE_ERROR;
			}

			/* This is Y2K workaround !!!
			** if the date is before 1.1.1970 i add 100 years (365*100 + 24)
			** (seem not to be valid for paradox 7.0)
			*/
#ifdef Y2K_WORKAROUND			
			if (retval < 719528) {
				retval += 36524;
			}
#endif
#if 1
			jd = jdatum(1,1,1);
			jd += retval - 1;
			
			gdate(jd, &y, &m, &dy);
			
			/* if the date has more than letters 
			   we assume that it some inserted correctly 
			   (not as an 2 letter short cut.) */
			   
			if (y >= 100) 
				_tm->tm_year	= y - 1900;
			else 
				_tm->tm_year	= y;

			_tm->tm_mon	= m - 1;
			_tm->tm_mday	= dy;

#else
			t = (retval - 719528 + 365) * 24 * 60 * 60;
			
			_tm = localtime( &t );
#endif
			break;
		case PX_Field_Type_Time:
			copy_from_be(d,s,4);
			
			if (s[0] & 0x80) {
				fix_sign(d, 4);
				retval /= 1000; // lost miliseconds !!
				_tm->tm_sec = retval % 60;
				retval /= 60;
				_tm->tm_min = retval % 60;
				_tm->tm_hour = retval/60;
			} else if (retval == 0) {
				return VALUE_IS_NULL;
			} else {
				fprintf(stderr,"%s.%d: TIMESTAMP(%016llx -> %016llx) can't be nagative\n",
					__FILE__, __LINE__,
					number, retval);
				return VALUE_ERROR;
			} 
			break;
		case PX_Field_Type_Timestamp: 
			copy_from_be(d,s,8);
			
			if (s[0] & 0x80) {
				fix_sign(d, 8);
				
			/* the last byte is unused */
				retval >>= 8;
			
			/* the timestamp seems to have a resolution of 1/500s */
				retval /= 500;
				
			/* the adjustment that is neccesary to convert paradox
			** timestamp to unix-timestamp [-> time()] 
			**
			** FIXME: this value is guessed !! 
			** the garantued precission is +/- 1 second 
			*/
			
				retval -= 37603860709183;
			
				t = retval;
				
				_tm = gmtime( &t );
			} else if (retval == 0) {
				return VALUE_IS_NULL;
			} else {
				fprintf(stderr,"%s.%d: TIMESTAMP(%016llx -> %016llx) can't be nagative\n",
					__FILE__, __LINE__,
					number, retval);
				return VALUE_ERROR;
			} 
			break;
		default:
			fprintf(stderr,"%s.%d: Can't convert type (%02x)!\n",
				__FILE__, __LINE__,
				type);
			return -1;
	}
	
	memcpy(tm, _tm, sizeof(struct tm));
	return VALUE_OK;
}

int PXtoQuotedString(char *dst, const unsigned char *src, int type) {
	switch (type) {
	    case PX_Field_Type_Alpha:
	    case PX_Field_Type_MemoBLOB:
		break;
	    default:
		fprintf(stderr,"%s.%d: Can't convert type (%02x)!\n",
			__FILE__, __LINE__,
			type);
		return -1;
	}
	
	while(*src) {
		switch(*src) {
/* cp431(??) -> latin1 */
			case 0x81: *dst = 'ü'; break;
			case 0x84: *dst = 'ä'; break;
			case 0x8e: *dst = 'Ä'; break;
			case 0x94: *dst = 'ö'; break;
			case 0x99: *dst = 'Ö'; break;
			case 0x9a: *dst = 'Ü'; break;
			case 0xe1: *dst = 'ß'; break;
			default: *dst = *src;
		}
		dst++;
		src++;
	}
	*dst = '\0';
	return 0;
}

char *PXNametoQuotedName(char *str) {
	unsigned char *s = str;
	while(*s) {
		switch(*s) {
			case 0x81: *s = 'ü'; break;
			case 0x84: *s = 'ä'; break;
			case 0x8e: *s = 'Ä'; break;
			case 0x94: *s = 'ö'; break;
			case 0x99: *s = 'Ö'; break;
			case 0x9a: *s = 'Ü'; break;
			case 0xe1: *s = 'ß'; break;
			case 0x20: *s = '_'; break;
			case '-' : *s = '_'; break;
		}
		s++;
	}
	return str;
}

typedef struct {
	unsigned char type;
	unsigned short size_div_4k;
	unsigned long length;
	unsigned short mod_count;
} mb_type2_pointer;

typedef struct {
	unsigned char offset;
	unsigned char length_div_16;
	unsigned short mod_count;
	unsigned char length_mod_16;
} mb_type3_pointer;

char *PXMEMOtoString(void *blob, int size, char *blobname) {
	unsigned long offset = 0, length = 0;
	unsigned short mod_number = 0;
	unsigned char index = 0;
	char *string = NULL;
	int fd;
	
	/* if the MEMO field contains less than 10 bytes it is stored in the .DB file */
	if (size < 10) return NULL;	
	
	copy_from_le(&offset, (char *)blob+(size-10), 4);
	copy_from_le(&length, (char *)blob+(size-6), 4);
	copy_from_le(&mod_number, (char *)blob+(size-2), 2);
	
	copy_from_le(&index, (char *)blob+(size-10), 1);
	
	offset &= 0xffffff00;
#ifdef DEBUG	
	fprintf(stderr, "[BLOB] offset: %08lx, length: %08lx, mod_number: %04x, index: %02x\n", offset, length, mod_number, index);
#endif
	if (index == 0x00) return NULL;
	
	if (!blobname) {
		fprintf(stderr, "[BLOB] offset: %08lx, length: %08lx, mod_number: %04x, index: %02x - do I need a BLOB-filename '-b ...' ?\n", offset, length, mod_number, index);
		return NULL;
	}
#ifdef O_BINARY
	fd = open(blobname, O_RDONLY|O_BINARY);
#else
	fd = open(blobname, O_RDONLY);
#endif
	if (fd == -1) return NULL;
	
	if (index == 0xff) {
		/* type 02 block */
		mb_type2_pointer idx;
		char head[9];
		
		/* go to the right block */
		lseek(fd, offset, SEEK_SET);
		
		read(fd, head, sizeof(head));
		
		copy_from_le(&idx.type,	       head + 0, sizeof(idx.type));
		copy_from_le(&idx.size_div_4k, head + 1, sizeof(idx.size_div_4k));
		copy_from_le(&idx.length,      head + 3, sizeof(idx.length));
		copy_from_le(&idx.mod_count,   head + 7, sizeof(idx.mod_count));
		if (idx.type != 0x02) {
			fprintf(stderr, "%s.%d: expected a type 02 blob\n",
				__FILE__, __LINE__);
			return NULL;
		}
		
		if (length != idx.length) {
			fprintf(stderr, "%s.%d: type 02 blob length doesn't match with BLOB length\n",
				__FILE__, __LINE__);
			return NULL;
		}
#if 0
		fprintf(stderr, "%02d, %04d, %04ld, %04d, %04d\n",
			idx.type,
			idx.size_div_4k * 4096,
			idx.length,
			idx.mod_count,
			length);
#endif
		string = malloc(length + 1);
		
		if (read(fd, string, length) != length) {
			fprintf (stderr, "%s.%d: Read less than requested\n", 
				 __FILE__, __LINE__);
		}
		string[length] = '\0';
	} else {
		int _start, _length;
		char head[5];
		mb_type3_pointer idx;
		
		
		/* go to the right block and skip the header */
		lseek(fd, offset + (12 + ( 5 * index )), SEEK_SET);
		
		read(fd, head, sizeof(head));
		
		copy_from_le(&idx.offset,	 head + 0, sizeof(idx.offset));
		copy_from_le(&idx.length_div_16, head + 1, sizeof(idx.length_div_16));
		copy_from_le(&idx.mod_count,	 head + 2, sizeof(idx.mod_count));
		copy_from_le(&idx.length_mod_16, head + 4, sizeof(idx.length_mod_16));
		
		_start = idx.offset * 16;
		_length = idx.length_div_16 * 16 + idx.length_mod_16;
		
		lseek(fd, offset + _start, SEEK_SET);
		
		string = malloc(length + 1);
		
		if (read(fd, string, length) != length) {
			fprintf (stderr, "%s.%d: Read less than requested\n", 
				 __FILE__, __LINE__);
		}
		string[length] = '\0';
		
		if (strncmp((char *)blob, string, size-10) != 0) {
			fprintf (stderr, "%s.%d: Extract failed: %s != %s\n", 
				 __FILE__, __LINE__, (char *)blob, string);
			
			free(string);
			string = NULL;
		}
	}
	
	close(fd);
	return string;
}


int PXBLOBtoBinary(void *blob, int size, char *blobname, void ** binstorage, int *binsize) {
	unsigned long offset = 0, length = 0;
	unsigned short mod_number = 0;
	unsigned char index = 0;
	char *string = NULL;
	int fd;
	
	/* if the MEMO field contains less than 10 bytes it is stored in the .DB file */
	if (size < 10) return 0;	
	
	copy_from_le(&offset, (char *)blob+(size-10), 4);
	copy_from_le(&length, (char *)blob+(size-6), 4);
	copy_from_le(&mod_number, (char *)blob+(size-2), 2);
	
	copy_from_le(&index, (char *)blob+(size-10), 1);
	
	offset &= 0xffffff00;
#ifdef DEBUG	
	fprintf(stderr, "[BLOB] offset: %08lx, length: %08lx, mod_number: %04x, index: %02x\n", offset, length, mod_number, index);
#endif
	if (index == 0x00) return 0;
	
	if (!blobname) {
		fprintf(stderr, "[BLOB] offset: %08lx, length: %08lx, mod_number: %04x, index: %02x - do I need a BLOB-filename '-b ...' ?\n", offset, length, mod_number, index);
		return -1;
	}
#ifdef O_BINARY
	fd = open(blobname, O_RDONLY|O_BINARY);
#else
	fd = open(blobname, O_RDONLY);
#endif
	if (fd == -1) return -1;
	
	if (index == 0xff) {
		/* type 02 block */
		mb_type2_pointer idx;
		char head[9];
		
		/* go to the right block */
		lseek(fd, offset, SEEK_SET);
		
		read(fd, head, sizeof(head));
		
		copy_from_le(&idx.type,	       head + 0, sizeof(idx.type));
		copy_from_le(&idx.size_div_4k, head + 1, sizeof(idx.size_div_4k));
		copy_from_le(&idx.length,      head + 3, sizeof(idx.length));
		copy_from_le(&idx.mod_count,   head + 7, sizeof(idx.mod_count));
		if (idx.type != 0x02) {
			fprintf(stderr, "%s.%d: expected a type 02 blob\n",
				__FILE__, __LINE__);
			return -1;
		}
		
		if (length != idx.length) {
			fprintf(stderr, "%s.%d: type 02 blob length doesn't match with BLOB length\n",
				__FILE__, __LINE__);
			return -1;
		}
#if 0
		fprintf(stderr, "%02d, %04d, %04ld, %04d, %04d\n",
			idx.type,
			idx.size_div_4k * 4096,
			idx.length,
			idx.mod_count,
			length);
#endif
		string = malloc(length + 1);
		
		if (read(fd, string, length) != length) {
			fprintf (stderr, "%s.%d: Read less than requested\n", 
				 __FILE__, __LINE__);
		}
		string[length] = '\0';
		
	} else {
		int _start, _length;
		char head[5];
		mb_type3_pointer idx;
		
		
		/* go to the right block and skip the header */
		lseek(fd, offset + (12 + ( 5 * index )), SEEK_SET);
		
		read(fd, head, sizeof(head));
		
		copy_from_le(&idx.offset,	 head + 0, sizeof(idx.offset));
		copy_from_le(&idx.length_div_16, head + 1, sizeof(idx.length_div_16));
		copy_from_le(&idx.mod_count,	 head + 2, sizeof(idx.mod_count));
		copy_from_le(&idx.length_mod_16, head + 4, sizeof(idx.length_mod_16));
		
		_start = idx.offset * 16;
		_length = idx.length_div_16 * 16 + idx.length_mod_16;
		
		lseek(fd, offset + _start, SEEK_SET);
		
		string = malloc(length + 1);
		
		if (read(fd, string, length) != length) {
			fprintf (stderr, "%s.%d: Read less than requested\n", 
				 __FILE__, __LINE__);
		}
		string[length] = '\0';
		
		if (strncmp((char *)blob, string, size-10) != 0) {
			fprintf (stderr, "%s.%d: Extract failed: %s != %s\n", 
				 __FILE__, __LINE__, (char *)blob, string);
			
			free(string);
			string = NULL;
		}
	}
	
	close(fd);
	
	*binsize = length;
	*binstorage = string;
	
	return 0;
}
