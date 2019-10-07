sources  = main.cpp
sources += index.cpp
objects = $(sources:.cpp=.o)
depends = $(sources:.cpp=.d)
CC = g++
DEFINES  = -Wall -O3 -march=native
#DEFINES  = -Wall -O0 -g -pg
#DEFINES += -DNDEBUG

touchdown_db: $(objects) Makefile
	$(CC) -o $@ $(DEFINES) $(objects)
	mv $@ $(HOME)/bin

%.d: %.cpp Makefile
	set -e; $(CC) -M $(CPPFLAGS) $(DEFINES) $(INCLUDE_DIRS) $< \
		| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
		[ -s $@ ] || rm -f $@

include $(depends)

%.o :
	$(CC) $(DEFINES) $(INCLUDE_DIRS) -c $< -o $@

clean: Makefile
	-rm $(objects)
	-rm $(depends)

