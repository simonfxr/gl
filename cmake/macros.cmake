macro(def_program target)
  cmu_add_executable(${ARGV})
  target_compile_definitions(${target} PRIVATE "-DCMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
endmacro()
