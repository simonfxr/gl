#ifndef GE_TOKENIZER_HPP
#define GE_TOKENIZER_HPP

#include "ge/CommandArgs.hpp"

#include <istream>
#include <vector>

namespace ge {

bool tokenize(std::istream& in, std::vector<CommandArg>& args);

} // namespace ge

#endif
