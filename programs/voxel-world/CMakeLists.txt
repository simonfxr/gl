set(VOXEL_LIB_DEPEND)
set(VOXEL_DEFINES)
set(VOXEL_WORLD_ENABLED True)

if(USE_VOXEL_OPENCL)
  include_directories(${PROJECT_SOURCE_DIR}/ext/include)
  link_directories(${PROJECT_SOURCE_DIR}/ext/lib)
  set(VOXEL_LIB_DEPEND voxel)
  set(VOXEL_DEFINES -DHS_WORLD_GEN)
elseif(OpenMP_FOUND)
  set(VOXEL_LIB_DEPEND OpenMP::OpenMP_CXX)
else()
  message(WARNING "Neither OpenCL nor OpenMP found: disabling target voxel-world")
  set(VOXEL_WORLD_ENABLED False)
endif()

if(VOXEL_WORLD_ENABLED)
  def_program(voxel-world
    SOURCES voxel-world.cpp
    DEPEND ge ${VOXEL_LIB_DEPEND})
  target_compile_definitions(voxel-world PRIVATE ${VOXEL_DEFINES})
endif()
