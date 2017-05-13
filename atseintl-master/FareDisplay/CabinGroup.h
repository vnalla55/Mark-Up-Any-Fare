//-------------------------------------------------------------------
//
//  File:        CabinGroup.h
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

#include <boost/noncopyable.hpp>
#include <log4cxx/helpers/objectptr.h>

#include <vector>

namespace tse
{
class FareDisplayTrx;
class Group;

class CabinGroup : boost::noncopyable
{
  friend class CabinGroupTest;

public:
  CabinGroup();
  void initializeCabinGroup(FareDisplayTrx& trx, std::vector<Group*>& groups);

};
} // tse
