macro(do_target_link_options target mode)
  if(COMMAND target_link_options)
    target_link_options(${target} ${mode} ${ARGN})
  else()
    target_link_libraries(${target} ${mode} ${ARGN})
  endif()
endmacro()

macro(do_configure_target target)
  target_compile_definitions(${target} PUBLIC ${GLOBAL_DEFINES})
  target_compile_options(${target} PUBLIC ${GLOBAL_FLAGS})
  do_target_link_options(${target} PUBLIC ${GLOBAL_LINK_FLAGS})
  target_link_libraries(${target} PUBLIC ${ARGN})
  if(ENABLE_IPO)
    set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
  endif()
  if(BUILD_SHARED_LIBS)
    get_target_property(export_name ${target} EXPORT_NAME)
    if(NOT export_name)
      set(export_name "${target}")
    endif()
    string(TOUPPER "${export_name}" export_name)
    message(STATUS "${target}-export-name: ${export_name}")
    set_target_properties(${target}
                          PROPERTIES C_VISIBILITY_PRESET hidden
                                     CXX_VISIBILITY_PRESET hidden
                                     VISIBILITY_INLINES_HIDDEN True
                                     EXPORT_NAME "${export_name}"
                                     )
  endif()
endmacro()

macro(def_lib target)
  cmake_parse_arguments(THIS "" "" "SOURCES;DEPEND" ${ARGN})
  add_library(${target} ${THIS_SOURCES})
  do_configure_target(${target} ${THIS_DEPEND})
endmacro()

macro(def_program target)
  cmake_parse_arguments(THIS "" "" "SOURCES;DEPEND" ${ARGN})
  add_executable(${target} ${THIS_SOURCES})
  do_configure_target(${target} ${THIS_DEPEND})
  target_compile_definitions(
    ${target} PRIVATE "-DCMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
endmacro()
