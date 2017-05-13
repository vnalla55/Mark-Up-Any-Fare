// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#ifndef UTIL_VECTOR_FWD_H
#define UTIL_VECTOR_FWD_H

#ifdef _GLIBCXX_DEBUG

#include <memory>

namespace tse
{
template <typename Type, typename Allocator = std::allocator<Type> >
class Vector;
}

#else

#include "Util/Allocator.h"

namespace tse
{
template <typename Type, typename Allocator = typename GetAllocator<Type>::type>
class Vector;
}

#endif

#endif
