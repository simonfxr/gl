add_library(ctest INTERFACE)
target_include_directories(ctest INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/ctest)

def_program(nb-server SOURCES nb-server.cpp DEPEND ge)
def_program(loop SOURCES loop.cpp DEPEND ge)
def_program(
  enum_to_string
  SOURCES
  enum_to_string.cpp
  DEPEND
  ge
  sys
  glt)
def_program(math_test SOURCES math_test.cpp DEPEND sys)
def_program(err_calls SOURCES err_calls.cpp DEPEND sys)

def_program(
  vec_test
  SOURCES
  vec_test.cpp
  DEPEND
  sys
  ctest
  bl)

def_program(
  rand_test
  SOURCES
  rand_test.cpp
  DEPEND
  sys
  ctest
  bl)

def_program(
  any_test
  SOURCES
  any_test.cpp
  DEPEND
  ctest
  bl
  sys)

def_program(optional_test SOURCES optional_test.cpp DEPEND ctest bl sys)
def_program(core_test SOURCES core_test.cpp DEPEND ctest bl)

if(BUILD_TESTING)
  foreach(tgt vec_test rand_test any_test optional_test core_test enum_to_string)
    add_test(NAME ${tgt} COMMAND ${tgt})
  endforeach()
endif()
