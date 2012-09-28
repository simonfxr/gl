
include(${PROJECT_SOURCE_DIR}/cmake/program.cmake)

def_program(chan SOURCES chan.cpp DEPEND ge LIB_DEPEND pthread)
def_program(ref_test SOURCES ref_test.cpp DEPEND ge)
#def_program(tokenizer SOURCES tokenizer.cpp DEPEND ge)
def_program(nb-server SOURCES nb-server.cpp DEPEND ge)
def_program(ref_code SOURCES ref_code.cpp DEPEND ge)
