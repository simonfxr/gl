
BIBLE := $(HOME)/src/oglsuperbible5-read-only

CC  := clang
CXX := clang++

INC_DIRS := -I$(BIBLE)/Src/GLTools/include -I/usr/include -I.
LIB_DIRS := -L/usr/X11R6/lib -L.

PACKAGES := glew OpenEXR

LIBS := -lX11 -lglut -lGL -lGLU -lm

OPT_FLAGS := -O3 -march=native $(OPT_FLAGS)

PKG_CFLAGS := $(shell pkg-config --cflags $(PACKAGES))

CFLAGS  := $(PKG_CFLAGS) $(INC_DIRS) -Wall -Wextra $(OPT_FLAGS) $(EXTRA_CFLAGS) 

CXXFLAGS := $(CFLAGS)

ICC_CFLAGS := $(CFLAGS) -xHOST -O3 -ipo -no-prec-div

PKG_LDFLAGS := $(shell pkg-config --libs $(PACKAGES))

LDFLAGS := $(PKG_LDFLAGS) $(LIB_DIRS) $(LDFLAGS)
LD_GLTOOLS := -lgltools -Wl,-rpath,$(PWD) $(LIBS)

GLTOOLS_SRC := GLBatch.cpp GLShaderManager.cpp GLTools.cpp GLTriangleBatch.cpp math3d.cpp

GLTOOLS_SBM := $(BIBLE)/Src/Models/Ninja/sbm.cpp

all: triangle

libgltools.so:
	cd $(BIBLE)/Src/GLTools/src && \
          $(CXX) -fPIC -c $(CXXFLAGS) -o $(shell basename $(GLTOOLS_SBM:.cpp=.o)) $(GLTOOLS_SBM) && \
          $(CXX) -fPIC -c $(CXXFLAGS) $(GLTOOLS_SRC) && \
          ld -G $(GLTOOLS_SRC:.cpp=.o) $(shell basename $(GLTOOLS_SBM:.cpp=.o)) -o $(PWD)/$@

triangle: triangle.cpp libgltools.so
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $(LD_GLTOOLS) $<

own1: own1.cpp glt/ShaderManager.cpp glt/ShaderProgram.cpp glt/include_proc.cpp glt/utils.cpp VertexBuffer.cpp 
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $^ -lglut

MATH_LIB_SOURCES := math/vec3/impl.cpp math/vec4/impl.cpp math/math/impl.cpp math/mat3/impl.cpp math/mat4/impl.cpp math/plane.cpp

GE_SOURCES := ge/GameLoop.cpp ge/GameWindow.cpp

GLT_SOURCES := glt/utils.cpp glt/ShaderManager.cpp glt/ShaderProgram.cpp glt/Uniforms.cpp glt/GenBatch.cpp  glt/Preprocessor.cpp glt/GLSLPreprocessor.cpp glt/Frame.cpp

SIM_SFML_SOURCES := sim-sfml.cpp $(MATH_LIB_SOURCES) $(GE_SOURCES) $(GLT_SOURCES)

sim-sfml: $(SIM_SFML_SOURCES) libgltools.so
	$(CXX) $(SIM_SFML_SOURCES) -o $@ $(CXXFLAGS) $(LDFLAGS) $(LD_GLTOOLS) `sfml-config --libs --cflags window graphics` -Wno-switch-enum -Wno-invalid-offsetof

cube: cube.cpp GameLoop.cpp gltools.cpp ShaderProgram.cpp Batch.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS) `sfml-config --libs --cflags window graphics` -Wno-switch-enum

clean:
	- rm libgltools.so triangle own1 sim-sfml cube

.PHONY: clean
