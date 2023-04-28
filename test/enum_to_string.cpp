#include "math/fvec.hpp"
#include "math/mat3.hpp"
#include "sys/io.hpp"
#include "sys/sys.hpp"

using namespace bl::literals;

int
main()
{
    sys::moduleInit();
    auto &out = sys::io::stdout();
    out << "Starting"_k
        << "\n"_k;
    out << sys::io::HandleError::EOF << "\n"_k;
    out << math::vec3(1, 2, 3) << "\n"_k;
    out << math::mat3() << "\n"_k;
    return 0;
}
