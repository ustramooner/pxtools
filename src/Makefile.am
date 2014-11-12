bin_PROGRAMS = pxinfo pxsqldump pxcsvdump pxxmldump

pxinfo_SOURCES = pxparse.c pxconvert.c pxinfo.c 
pxsqldump_SOURCES = pxparse.c pxconvert.c pxsqldump.c 
pxcsvdump_SOURCES = pxparse.c pxconvert.c pxcsvdump.c 
pxxmldump_SOURCES = pxparse.c pxconvert.c pxxmldump.c 

noinst_HEADERS = pxtypes.h pxconvert.h pxparse.h

CFLAGS = -g -Wall -O2 #-DDEBUG -DDEBUG_HEAD_COPY
DEFS=@DEFS@ -DLOCALEDIR=\"$(localedir)\" 

pxinfo_LDADD=$(INTLLIBS)
pxsqldump_LDADD=$(INTLLIBS)
pxcsvdump_LDADD=$(INTLLIBS)
pxxmldump_LDADD=$(INTLLIBS)
