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

#include "Routing/RestrictionValidatorFactory.h"

#include "Common/Global.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/RoutingRestriction.h"
#include "Routing/AirSurfaceRestrictionValidator.h"
#include "Routing/CityCarrierRestrictionValidator.h"
#include "Routing/NonPricingRestrictionValidator.h"
#include "Routing/RestrictionValidator.h"
#include "Routing/SpecifiedMpmValidator.h"
#include "Routing/StopOverRestrictionValidator.h"
#include "Routing/StopTypeRestrictionValidator.h"
#include "Routing/TravelRestrictionValidator.h"

namespace tse
{

RestrictionValidatorFactory::RestrictionValidatorFactory()
{
  CityCarrierRestrictionValidator* ccValidator = nullptr;
  dataHandle.get(ccValidator);
  validators.insert(std::make_pair(RESTNUM_1, ccValidator));
  validators.insert(std::make_pair(RESTNUM_2, ccValidator));
  validators.insert(std::make_pair(RESTNUM_5, ccValidator));
  validators.insert(std::make_pair(RESTNUM_18, ccValidator));
  validators.insert(std::make_pair(RESTNUM_19, ccValidator));
  validators.insert(std::make_pair(RESTNUM_21, ccValidator));
  StopTypeRestrictionValidator* stValidator = nullptr;
  dataHandle.get(stValidator);
  validators.insert(std::make_pair(RESTNUM_3, stValidator));
  validators.insert(std::make_pair(RESTNUM_4, stValidator));
  validators.insert(std::make_pair(RESTNUM_6, stValidator));
  validators.insert(std::make_pair(RESTNUM_12, stValidator));
  StopOverRestrictionValidator* soValidator = nullptr;
  dataHandle.get(soValidator);
  validators.insert(std::make_pair(RESTNUM_7, soValidator));
  SpecifiedMpmValidator* mpmValidator = nullptr;
  dataHandle.get(mpmValidator);
  validators.insert(std::make_pair(RESTNUM_8, mpmValidator));
  TravelRestrictionValidator* tvlValidator = nullptr;
  dataHandle.get(tvlValidator);
  validators.insert(std::make_pair(RESTNUM_9, tvlValidator));
  validators.insert(std::make_pair(RESTNUM_10, tvlValidator));
  AirSurfaceRestrictionValidator* asValidator = nullptr;
  dataHandle.get(asValidator);
  validators.insert(std::make_pair(RESTNUM_11, asValidator));
  NonPricingRestrictionValidator* npValidator = nullptr;
  dataHandle.get(npValidator);
  validators.insert(std::make_pair(RESTNUM_13, npValidator));
  validators.insert(std::make_pair(RESTNUM_14, npValidator));
  validators.insert(std::make_pair(RESTNUM_15, npValidator));
  validators.insert(std::make_pair(RESTNUM_17, npValidator));
}

RestrictionValidatorFactory::~RestrictionValidatorFactory() {}

RestrictionValidator*
RestrictionValidatorFactory::getRestrictionValidator(const RestrictionNumber& resNum,
                                                     const PricingTrx& trx)
{
  RestNum restNum = static_cast<RestNum>(atoi(resNum.c_str()));
  if ((restNum != RESTNUM_18) && (restNum != RESTNUM_19))
  {
    std::map<RestNum, RestrictionValidator*>::const_iterator itr = validators.find(restNum);
    if (LIKELY(itr != validators.end()))
      return itr->second;
  }

  return nullptr;
}
}
