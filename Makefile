# this is the shortest way to get the fastest code (on a K6/2 400)
# seems to work fairly well on a P3-300 too
CC = cc -O3 -funroll-all-loops -I ./portable -I ./portable/pixels/ \
	-I ./portable/generators/ -I ./unix/ -g -D__USE_EXTERN_INLINES
LDFLAGS = -L/usr/X11R6/lib
LIBS = -lm -lX11 -lpng
VPATH = ./portable/:./portable/pixels/:./portable/generators/:./unix/
OBJECTS = 	starfish-engine.o generators.o genutils.o\
		bufferxform.o greymap.o pixmap.o starfish-rasterlib.o \
		coswave-gen.o spinflake-gen.o rangefrac-gen.o \
		bubble-gen.o flatwave-gen.o setdesktop.o makepng.o

starfish: $(OBJECTS) unix/starfish.o
	$(CC) -o starfish $(LDFLAGS) $(OBJECTS) unix/starfish.o $(LIBS)

starfish-engine.o: starfish-engine.c starfish-engine.h generators.h \
	starfish-rasterlib.h

setdesktop.o: setdesktop.c genutils.h setdesktop.h

makepng.o: makepng.c makepng.h starfish-engine.h

generators.o: generators.c generators.h greymap.h \
	coswave-gen.h spinflake-gen.h rangefrac-gen.h \
	bubble-gen.h flatwave-gen.h

genutils.o: genutils.c genutils.h
 
bufferxform.o: bufferxform.c bufferxform.h pixmap.h greymap.h

greymap.o: greymap.c greymap.h

pixmap.o: pixmap.c pixmap.h starfish-rasterlib.h

starfish-rasterlib.o: starfish-rasterlib.c starfish-rasterlib.h \
	rasterliberrs.h pixmap.h greymap.h bufferxform.h

coswave-gen.o: coswave-gen.c coswave-gen.h genutils.h

spinflake-gen.o: spinflake-gen.c spinflake-gen.h genutils.h

rangefrac-gen.o: rangefrac-gen.c rangefrac-gen.h genutils.h

bubble-gen.o: bubble-gen.c bubble-gen.h genutils.h

flatwave-gen.o: flatwave-gen.c flatwave-gen.h genutils.h

clean: 
	rm -f $(OBJECTS) starfish

install:
	cp ./starfish /usr/local/bin/xstarfish
