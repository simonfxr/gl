#include "math/mat3.hpp"
#include "math/vec3.hpp"
#include "sys/io.hpp"
#include "sys/sys.hpp"

int
main()
{
    sys::moduleInit();
    auto &out = sys::io::stdout();
    out << "Starting"
        << "\n";
    out << sys::io::HandleError::EOF << "\n";
    out << math::vec3(1, 2, 3) << "\n";
    out << math::mat3() << "\n";
    return 0;
}
