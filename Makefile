
BIBLE := $(HOME)/src/oglsuperbible5-read-only

INC_DIRS := -I$(BIBLE)/Src/GLTools/include -I/usr/include
LIB_DIRS := -L/usr/X11R6/lib -L.

PACKAGES := glew OpenEXR

LIBS := -lX11 -lglut -lGL -lGLU -lm

CFLAGS  := -I/usr/include/GL `pkg-config --cflags $(PACKAGES)` $(INC_DIRS) -Wall 
LDFLAGS := `pkg-config --libs $(PACKAGES)` $(LIB_DIRS) $(LIBS)
LD_GLTOOLS := -lgltools -Wl,-rpath,$(PWD)

GLTOOLS_SRC := GLBatch.cpp GLShaderManager.cpp GLTools.cpp GLTriangleBatch.cpp math3d.cpp 

GLTOOLS_SBM := $(BIBLE)/Src/Models/Ninja/sbm.cpp

all: triangle

libgltools.so:
	cd $(BIBLE)/Src/GLTools/src && \
          g++ -fPIC -c $(CFLAGS) -o $(shell basename $(GLTOOLS_SBM:.cpp=.o)) $(GLTOOLS_SBM) && \
          g++ -fPIC -c $(CFLAGS) $(GLTOOLS_SRC) && \
          ld -G $(GLTOOLS_SRC:.cpp=.o) $(shell basename $(GLTOOLS_SBM:.cpp=.o)) -o $(PWD)/$@

triangle: triangle.cpp libgltools.so
	g++ -o $@ $(CFLAGS) $(LDFLAGS) $(LD_GLTOOLS) $<

own1: own1.cpp ShaderProgram.cpp VertexBuffer.cpp
	g++-4.5 -I. -Wall -o $@ $(CFLAGS) $(LDFLAGS) $^

clean:
	- rm libgltools.so triangle

.PHONY: clean
