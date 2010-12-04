
BIBLE := $(HOME)/src/oglsuperbible5-read-only

CC := icc
CXX := icpc

INC_DIRS := -I$(BIBLE)/Src/GLTools/include -I/usr/include -I. -Imath
LIB_DIRS := -L/usr/X11R6/lib -L.

PACKAGES := glew OpenEXR

LIBS := -lX11 -lglut -lGL -lGLU -lm

OPT_FLAGS := -O3 -march=native $(OPT_FLAGS)

PKG_CFLAGS := $(shell pkg-config --cflags $(PACKAGES))

CFLAGS  := -I/usr/include/GL $(PKG_CFLAGS) $(INC_DIRS) -Wall -Wextra $(OPT_FLAGS) $(EXTRA_CFLAGS)

PKG_LDFLAGS := $(shell pkg-config --libs $(PACKAGES))

LDFLAGS := $(PKG_LDFLAGS) $(LIB_DIRS) $(LIBS) $(LDFLAGS)
LD_GLTOOLS := -lgltools -Wl,-rpath,$(PWD)

GLTOOLS_SRC := GLBatch.cpp GLShaderManager.cpp GLTools.cpp GLTriangleBatch.cpp math3d.cpp

GLTOOLS_SBM := $(BIBLE)/Src/Models/Ninja/sbm.cpp

all: triangle

libgltools.so:
	cd $(BIBLE)/Src/GLTools/src && \
          $(CXX) -fPIC -c $(CFLAGS) -o $(shell basename $(GLTOOLS_SBM:.cpp=.o)) $(GLTOOLS_SBM) && \
          $(CXX) -fPIC -c $(CFLAGS) $(GLTOOLS_SRC) && \
          ld -G $(GLTOOLS_SRC:.cpp=.o) $(shell basename $(GLTOOLS_SBM:.cpp=.o)) -o $(PWD)/$@

triangle: triangle.cpp libgltools.so
	$(CXX) -o $@ $(CFLAGS) $(LDFLAGS) $(LD_GLTOOLS) $<

own1: own1.cpp ShaderProgram.cpp VertexBuffer.cpp
	$(CXX) -o $@ -flto $(CFLAGS) $(LDFLAGS) $^

sim: sim.cpp libgltools.so
	$(CXX) -o $@ $(CFLAGS) $(LDFLAGS) $(LD_GLTOOLS) $<

clean:
	- rm libgltools.so triangle own1

.PHONY: clean
