
CXX=clang++

CFLAGS := -Wall -Wextra -I../.. -I.. $(EXTRA_CFLAGS)

MATH_SOURCES=../vec3/vec3.cpp  ../vec4/vec4.cpp ../mat3/impl.cpp ../mat4/impl.cpp ../math/impl.cpp

mat_test: mat_test.cpp $(MATH_SOURCES)
	$(CXX) -o $@ $(CFLAGS) $(LDFLAGS) $^

mat_mul: mat_mul.cpp $(MATH_SOURCES)
	$(CXX) -o $@ $(CFLAGS) -lrt $(LDFLAGS) $^
