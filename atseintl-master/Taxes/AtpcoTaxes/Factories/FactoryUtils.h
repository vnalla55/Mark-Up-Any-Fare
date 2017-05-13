// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include "DataModel/Common/Types.h"

#include <boost/ptr_container/ptr_vector.hpp>

#include <cassert>
#include <sstream>
#include <stdexcept>

namespace tax
{

template <typename T>
const T&
get(const type::Index& pathRefId, const std::string pathName, const boost::ptr_vector<T>& paths)
{
  if (pathRefId == std::numeric_limits<type::Index>::max())
  {
    throw std::logic_error(pathName + " ref id not specified in itin information.");
  }
  else if (pathRefId >= paths.size())
  {
    std::ostringstream msg;
    msg << pathName << " ref id " << pathRefId << " could not be matched.";
    throw std::logic_error(msg.str());
  }
  else
  {
    return paths[pathRefId];
  }
}

template <typename Factory, typename From, typename To>
void create(const boost::ptr_vector<From>& from, std::vector<To>& to)
{
  assert(to.empty());
  to.reserve(from.size());
  for (const From& fromEl : from)
  {
    to.push_back(Factory::createFromInput(fromEl));
  }
}

} // namespace tax
