//----------------------------------------------------------------------------
//
//  Copyright Sabre 2003
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/Singleton.h"
#include "DBAccess/DataHandle.h"

#include <map>

namespace tse
{

class RestrictionValidator;
class PricingTrx;

class RestrictionValidatorFactory
{
  friend class tse::Singleton<RestrictionValidatorFactory>;

public:
  RestrictionValidator* getRestrictionValidator(const RestrictionNumber&, const PricingTrx&);
  ~RestrictionValidatorFactory();

private:
  enum RestNum
  {
    RESTNUM_1 = 1,
    RESTNUM_2 = 2,
    RESTNUM_3 = 3,
    RESTNUM_4 = 4,
    RESTNUM_5 = 5,
    RESTNUM_6 = 6,
    RESTNUM_7 = 7,
    RESTNUM_8 = 8,
    RESTNUM_9 = 9,
    RESTNUM_10 = 10,
    RESTNUM_11 = 11,
    RESTNUM_12 = 12,
    RESTNUM_13 = 13,
    RESTNUM_14 = 14,
    RESTNUM_15 = 15,
    RESTNUM_17 = 17,
    RESTNUM_18 = 18,
    RESTNUM_19 = 19,
    RESTNUM_21 = 21
  };

  RestrictionValidatorFactory();
  RestrictionValidatorFactory(const RestrictionValidatorFactory&);

  DataHandle dataHandle;
  std::map<RestNum, RestrictionValidator*> validators;
};
}

