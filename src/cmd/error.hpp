#ifndef __CMD_ERROR_HPP
#define __CMD_ERROR_HPP

#include "util/error.hpp"

namespace cmd
{

struct SyntaxError : public util::RuntimeError
{
  SyntaxError() : std::runtime_error("Syntax error.") { }
};

struct PermissionError : public util::RuntimeError
{
  PermissionError() : std::runtime_error("Permission error.") { }
};

} /* cmd namespace */

#endif
