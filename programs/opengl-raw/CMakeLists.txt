
include(${PROJECT_SOURCE_DIR}/cmake/program.cmake)

add_executable(opengl-raw opengl-raw.cpp)
target_link_libraries(opengl-raw ${GLEW_LIBRARY} ${OPENGL_gl_LIBRARY} ${GLFW_LIBRARY} ge)
