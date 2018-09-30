CROSS=	arm926ejs-wrswrap-linux-gnueabi-
CC=  	$(CROSS)gcc
LD=  	$(CROSS)ld
STRIP=	$(CROSS)strip

SRCS := conn.c
LIBS := libconn.so
LIB_CFLAGS  += -shared -fPIC $(CFLAGS) -lpthread

all: $(LIBS)

$(LIBS): 
	$(CC) $(LDFLAGS) -o ${LIBS} ${SRCS} ${LIB_CFLAGS}

distclean clean:
	$(RM) $(LIBS) *.o

