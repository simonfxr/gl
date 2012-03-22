# most of the following code is adopted from SFML

macro(sfml_static_add_libraries target)
  if(WINDOWS AND COMP_GCC)
    # Windows - gcc
    foreach(lib ${ARGN})
      if(NOT ${lib} MATCHES ".*/.*")
        string(REGEX REPLACE "(.*)/bin/.*\\.exe" "\\1" STANDARD_LIBS_PATH "${CMAKE_CXX_COMPILER}")
        set(lib "${STANDARD_LIBS_PATH}/lib/lib${lib}.a")
      endif()
      string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
      get_target_property(TARGET_FILENAME ${target} ${BUILD_TYPE}_LOCATION)
      add_custom_command(TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_AR} x ${lib}
        COMMAND ${CMAKE_AR} rcs ${TARGET_FILENAME} *.o
        COMMAND del *.o /f /q
        VERBATIM)
    endforeach()
  elseif(COMP_CL)
    # Visual C++
    set(LIBRARIES "")
    foreach(lib ${ARGN})
      if(NOT ${lib} MATCHES ".*\\.lib")
        set(lib ${lib}.lib)
      endif()
      if(MSVC_IDE AND MSVC_VERSION LESS 2010)
        # for Visual Studio projects < 2010, we must add double quotes
        # around paths because they may contain spaces
        set(LIBRARIES "${LIBRARIES} &quot\\;${lib}&quot\\;")
      else()
        set(LIBRARIES "${LIBRARIES} \"${lib}\"")
      endif()
    endforeach()
    set_target_properties(${target} PROPERTIES STATIC_LIBRARY_FLAGS ${LIBRARIES})
  else()
    # All other platforms
    target_link_libraries(${target} ${ARGN})
  endif()
endmacro()

# check if a value is contained in a list
# sets ${var} to TRUE if the value is found
macro(sfml_list_contains var value)
  set(${var})
  foreach(value2 ${ARGN})
    if(${value} STREQUAL ${value2})
      set(${var} TRUE)
    endif()
  endforeach()
endmacro()

# parse a list of arguments and options
# ex: sfml_parse_arguments(THIS "SOURCES;DEPENDS" "FLAG" FLAG SOURCES s1 s2 s3 DEPENDS d1 d2)
# will define the following variables:
# - THIS_SOURCES (s1 s2 s3)
# - THIS_DEPENDS (d1 d2)
# - THIS_FLAG TRUE
macro(sfml_parse_arguments prefix arg_names option_names)
  foreach(arg_name ${arg_names})
    set(${prefix}_${arg_name})
  endforeach()
  foreach(option_name ${option_names})
    set(${prefix}_${option_name} FALSE)
  endforeach()
  set(current_arg_name)
  set(current_arg_list)
  foreach(arg ${ARGN})
    sfml_list_contains(is_arg_name ${arg} ${arg_names})
    if(is_arg_name)
      set(${prefix}_${current_arg_name} ${current_arg_list})
      set(current_arg_name ${arg})
      set(current_arg_list)
    else()
      sfml_list_contains(is_option ${arg} ${option_names})
      if(is_option)
        set(${prefix}_${arg} TRUE)
      else()
        set(current_arg_list ${current_arg_list} ${arg})
      endif()
    endif()
  endforeach()
  set(${prefix}_${current_arg_name} ${current_arg_list})
endmacro()

macro(def_lib target)

  # parse the arguments
  sfml_parse_arguments(THIS "SOURCES;DEPEND;LIB_DEPEND" "" ${ARGN})

  # create the target
  add_library(${target} ${THIS_SOURCES})

  # define the export symbol of the module
  string(REPLACE "-" "_" NAME_UPPER "${target}")
  string(TOUPPER "${NAME_UPPER}" NAME_UPPER)
  set_target_properties(${target} PROPERTIES DEFINE_SYMBOL ${NAME_UPPER}_EXPORTS)
  # adjust the output file prefix/suffix to match our conventions
  if(BUILD_SHARED_LIBS)
    if (SYS_WINDOWS AND COMP_GCC)
      # on Windows/gcc get rid of "lib" prefix for shared libraries,
      # and transform the ".dll.a" suffix into ".a" for import libraries
      set_target_properties(${target} PROPERTIES PREFIX "")
      set_target_properties(${target} PROPERTIES IMPORT_SUFFIX ".a")
    endif()
  endif()

  # for gcc >= 4.0 on Windows, apply the STATIC_STD_LIBS option if it is enabled
  if(SYS_WINDOWS AND COMP_GCC AND STATIC_STD_LIBS)
    if(NOT GCC_VERSION VERSION_LESS "4")
      set_target_properties(${target} PROPERTIES LINK_FLAGS "-static-libgcc -static-libstdc++")
    endif()
  endif()

  # If using gcc >= 4.0 or clang >= 3.0 on a non-Windows platform, we must hide public symbols by default
  # (exported ones are explicitely marked)
  if(NOT SYS_WINDOWS AND (COMP_GCC OR COMP_CLANG OR COMP_ICC))
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS -fvisibility=hidden)
  endif()

  # link the target to its SFML dependencies
  if(THIS_DEPEND)
    target_link_libraries(${target} ${THIS_DEPEND})
  endif()

  # link the target to its external dependencies
  if(THIS_LIB_DEPEND)
    if(BUILD_SHARED_LIBS)
      # in shared build, we use the regular linker commands
      target_link_libraries(${target} ${THIS_LIB_DEPEND})
    else()
      # in static build there's no link stage, but with some compilers it is possible to force
      # the generated static library to directly contain the symbols from its dependencies
      sfml_static_add_libraries(${target} ${THIS_LIB_DEPEND})
    endif()
  endif()

endmacro()

macro(def_program target)

  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
  # parse the arguments
  sfml_parse_arguments(THIS "SOURCES;DEPEND;LIB_DEPEND" "NO_GUI_APP" ${ARGN})

  # create the target
  if(THIS_GUI_APP AND SYS_WINDOWS)
    add_executable(${target} WIN32 ${THIS_SOURCES})
    target_link_libraries(${target} win-main)
  else()
    add_executable(${target} ${THIS_SOURCES})
  endif()

  # for gcc >= 4.0 on Windows, apply the STATIC_STD_LIBS option if it is enabled
  if(SYS_WINDOWS AND COMP_GCC AND STATIC_STD_LIBS)
    if(NOT GCC_VERSION VERSION_LESS "4")
      set_target_properties(${target} PROPERTIES LINK_FLAGS "-static-libgcc -static-libstdc++")
    endif()
  endif()

  # link the target to its SFML dependencies
  if(THIS_DEPEND)
    target_link_libraries(${target} ${THIS_DEPEND})
  endif()
endmacro()
