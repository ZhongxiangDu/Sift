CC=g++
CFLAGS = -g -Wall

SOURCES = ds_debug.cpp \
					ds_pgm.cpp \
					ds_main.cpp \
          ds_feature.cpp \
					ds_match.cpp \
					ds_scalespace.cpp \
					ds_sift.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=sift

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) -c $<

clear:
	rm -f *.o sift
