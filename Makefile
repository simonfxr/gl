
BIBLE := $(HOME)/src/oglsuperbible5-read-only

INC_DIRS := -I$(BIBLE)/Src/GLTools/include -I/usr/include
LIB_DIRS := -L/usr/X11R6/lib -L.

LIBS := -lgltools -lX11 -lglut -lGL -lGLU -lm

CFLAGS  := `pkg-config --cflags glew` $(INC_DIRS) -Wall 
LDFLAGS := $(LIB_DIRS) $(LIBS) `pkg-config --libs glew`

GLTOOLS_SRC := GLBatch.cpp GLShaderManager.cpp GLTools.cpp GLTriangleBatch.cpp math3d.cpp

libgltools.so:
	cd $(BIBLE)/Src/GLTools/src && \
          g++ -fPIC -c $(CFLAGS)  $(GLTOOLS_SRC) && \
          ld -G $(GLTOOLS_SRC:.cpp=.o) -o $(PWD)/$@

triangle: libgltools.so triangle.cpp
	g++ -o $@ $(CFLAGS) $(LDFLAGS) triangle.cpp -Wl,-rpath,$(PWD)




.PHONY: libgltools.so


