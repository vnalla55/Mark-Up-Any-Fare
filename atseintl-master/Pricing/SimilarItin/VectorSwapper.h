/*---------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
#pragma once

#include <vector>

namespace tse
{
namespace similaritin
{
template <typename T>
class VectorSwapper
{
public:
  VectorSwapper(const std::vector<T>& origin, std::vector<T>& destination)
    : _origin(origin), _destination(destination)
  {
    _destination.swap(_origin);
  }

  ~VectorSwapper() { _destination.swap(_origin); }

private:
  std::vector<T> _origin;
  std::vector<T>& _destination;
};
}
}
