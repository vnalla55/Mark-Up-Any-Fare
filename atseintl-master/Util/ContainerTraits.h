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

#pragma once

#include <vector>

#include <boost/unordered/unordered_map_fwd.hpp>
#include <boost/unordered/unordered_set_fwd.hpp>

#include "Util/FlatFwd.h"
#include "Util/VectorFwd.h"

namespace tse
{
template <typename Container>
struct ContainerTraits
{
  static const bool is_container = false;
};

#define DEFINE_CONTAINER_(container, template_args, args) \
  template <template_args> \
  struct ContainerTraits< container<args> > \
  { \
    static const bool is_container = true; \
  };

#define TEMPLATE_ARGS1 typename A1
#define TEMPLATE_ARGS2 TEMPLATE_ARGS1, typename A2
#define TEMPLATE_ARGS3 TEMPLATE_ARGS2, typename A3
#define TEMPLATE_ARGS4 TEMPLATE_ARGS3, typename A4
#define TEMPLATE_ARGS5 TEMPLATE_ARGS4, typename A5

#define ARGS1 A1
#define ARGS2 ARGS1, A2
#define ARGS3 ARGS2, A3
#define ARGS4 ARGS3, A4
#define ARGS5 ARGS4, A5

#define DEFINE_CONTAINER(container, args) \
  DEFINE_CONTAINER_(container, TEMPLATE_ARGS ## args, ARGS ## args)

DEFINE_CONTAINER(std::vector, 2)
DEFINE_CONTAINER(Vector, 2)

DEFINE_CONTAINER(FlatSet, 3)
DEFINE_CONTAINER(FlatMultiSet, 3)
DEFINE_CONTAINER(FlatMap, 4)
DEFINE_CONTAINER(FlatMultiMap, 4)

DEFINE_CONTAINER(boost::unordered_set, 4)
DEFINE_CONTAINER(boost::unordered_multiset, 4)
DEFINE_CONTAINER(boost::unordered_map, 5)
DEFINE_CONTAINER(boost::unordered_multimap, 5)

//TODO Add more containers here, preferably these with fwd-like headers.

#undef DEFINE_CONTAINER
#undef ARGS1
#undef ARGS2
#undef ARGS3
#undef ARGS4
#undef ARGS5
#undef TEMPLATE_ARGS1
#undef TEMPLATE_ARGS2
#undef TEMPLATE_ARGS3
#undef TEMPLATE_ARGS4
#undef TEMPLATE_ARGS5
#undef DEFINE_CONTAINER_

}

