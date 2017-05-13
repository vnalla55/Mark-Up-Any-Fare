//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/Utils/SharingSet.h"

namespace tse
{

namespace tools
{

template <class T>
class GetStash
{
public:
  SharingSet<T>& operator()() const
  {
    return _stash;
  }
private:
  static SharingSet<T> _stash;
};

} // namespace tools

} // namespace tse
