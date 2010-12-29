
BIBLE := $(HOME)/src/oglsuperbible5-read-only

CC  := clang
CXX := clang++

INC_DIRS := -I$(BIBLE)/Src/GLTools/include -I/usr/include -I. -Imath
LIB_DIRS := -L/usr/X11R6/lib -L.

PACKAGES := glew OpenEXR

LIBS := -lX11 -lglut -lGL -lGLU -lm

OPT_FLAGS := -O3 -march=native $(OPT_FLAGS)

PKG_CFLAGS := $(shell pkg-config --cflags $(PACKAGES))

CFLAGS  := -I/usr/include/GL $(PKG_CFLAGS) $(INC_DIRS) -Wall -Wextra $(OPT_FLAGS) $(EXTRA_CFLAGS) 

ICC_CFLAGS := $(CFLAGS) -xHOST -O3 -ipo -no-prec-div

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
	$(CXX) -o $@ $(CFLAGS) $(LDFLAGS) $^

SIM_SFML_SOURCES := sim-sfml.cpp GameLoop.cpp gltools.cpp ShaderProgram.cpp GenBatch.cpp GameWindow.cpp

sim-sfml: $(SIM_SFML_SOURCES) libgltools.so
	$(CXX) $(SIM_SFML_SOURCES) -o $@  `pkg-config --libs --cflags glew` -I/usr/include/GL $(OPT_FLAGS) -L. $(LD_GLTOOLS) -Wall -Wextra $(INC_DIRS) `sfml-config --libs --cflags window graphics` -Wno-switch-enum -Wno-invalid-offsetof $(EXTRA_CFLAGS)

cube: cube.cpp GameLoop.cpp gltools.cpp ShaderProgram.cpp Batch.cpp
	$(CXX) -o $@ $^ $(CFLAGS) `sfml-config --libs --cflags window graphics` -Wno-switch-enum

clean:
	- rm libgltools.so triangle own1 sim-sfml cube

.PHONY: clean
