#pragma once

#include <boost/range/adaptors.hpp>

#define REVERSE_FOREACH(var_decl, range) for(var_decl : boost::adaptors::reverse(range))
