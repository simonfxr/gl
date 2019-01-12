#include "sys/io.hpp"
#include "sys/sys.hpp"

int
main()
{
    sys::moduleInit();
    auto &out = sys::io::stdout();
    out << "Starting" << sys::io::endl;
    out << sys::io::HandleError::EOF << sys::io::endl;
    return 0;
}