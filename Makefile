INCLUDE_DIRS = 
LIB_DIRS = 
CC=g++

CDEFS=
CFLAGS= -O0 -Wall -pg -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= -lrt
CPPLIBS= -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video

HFILES= capture.h
CFILES= 
CPPFILES= motion_detection.cpp compress_frame.cpp convert_frame.cpp edit_hdr_frame.cpp time_logger.cpp capture.cpp
LINK= motion_detection.cpp compress_frame.cpp convert_frame.cpp edit_hdr_frame.cpp time_logger.cpp

SRCS= ${HFILES} ${CFILES}
CPPOBJS= ${CPPFILES:.cpp=.o}

all:	capture 

clean:
	-rm -f *.o *.d
	-rm -f capture

distclean:
	-rm -f *.o *.d

capture: ${CPPOBJS}
	$(CC) $(LDFLAGS) $(CFLAGS) $(LINK) -lz -o $@ $@.o `pkg-config --libs opencv` $(CPPLIBS)

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<

.cpp.o:
	$(CC) $(CFLAGS) -c $<
