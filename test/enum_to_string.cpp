#include "math/mat3.hpp"
#include "math/vec3.hpp"
#include "sys/io.hpp"

int
main()
{
    auto &out = sys::io::stdout();
    out << "Starting" << sys::io::endl;
    out << sys::io::HandleError::EOF << sys::io::endl;
    out << math::vec3(1, 2, 3) << sys::io::endl;
    out << math::mat3() << sys::io::endl;
    return 0;
}
