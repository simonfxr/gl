
set(NEED_OPENMP TRUE)

include(${PROJECT_SOURCE_DIR}/cmake/program.cmake)

set(VOXEL_LIB_DEPEND)

if(USE_VOXEL_OPENCL)
  include_directories(${PROJECT_SOURCE_DIR}/ext/include)
  link_directories(${PROJECT_SOURCE_DIR}/ext/lib)
  set(VOXEL_LIB_DEPEND voxel)
  add_definitions(-DHS_WORLD_GEN)
endif()

def_program(voxel-world 
  SOURCES voxel-world.cpp 
  DEPEND ge
  LIB_DEPEND ${VOXEL_LIB_DEPEND})
