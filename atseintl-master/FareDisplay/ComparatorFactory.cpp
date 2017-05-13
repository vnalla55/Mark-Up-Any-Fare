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

#include "FareDisplay/ComparatorFactory.h"

#include "Common/Global.h"
#include "DBAccess/DataHandle.h"
#include "FareDisplay/BrandComparator.h"
#include "FareDisplay/CabinGroupComparator.h"
#include "FareDisplay/CharCombinationsComparator.h"
#include "FareDisplay/Comparator.h"
#include "FareDisplay/FareAmountComparator.h"
#include "FareDisplay/FareBasisComparator.h"
#include "FareDisplay/GlobalDirectionComparator.h"
#include "FareDisplay/MultiTransportComparator.h"
#include "FareDisplay/NormalSpecialComparator.h"
#include "FareDisplay/NullComparator.h"
#include "FareDisplay/OneWayRoundTripComparator.h"
#include "FareDisplay/PsgTypeComparator.h"
#include "FareDisplay/PublicPrivateComparator.h"
#include "FareDisplay/RoutingNumberComparator.h"
#include "FareDisplay/S8BrandComparator.h"
#include "FareDisplay/ScheduleCountComparator.h"
#include "FareDisplay/TravelDiscontinueDateComparator.h"

namespace tse
{
ComparatorFactory::~ComparatorFactory() {}

Comparator*
ComparatorFactory::getComparator(Group::GroupType& type)
{

  switch (type)
  {
  case Group::GROUP_BY_SCHEDULE_COUNT:
  {
    ScheduleCountComparator* schedCompartor = nullptr;
    _trx.dataHandle().get(schedCompartor);
    return schedCompartor;
  }

  case Group::GROUP_BY_ROUTING:
  {
    RoutingNumberComparator* rtgComparator = nullptr;
    _trx.dataHandle().get(rtgComparator);
    return rtgComparator;
  }

  case Group::GROUP_BY_GLOBAL_DIR:
  {
    GlobalDirectionComparator* gdComparator = nullptr;
    _trx.dataHandle().get(gdComparator);
    return gdComparator;
  }
  case Group::GROUP_BY_PSG_TYPE:
  {
    PsgTypeComparator* paxComparator = nullptr;
    _trx.dataHandle().get(paxComparator);
    return paxComparator;
  }
  case Group::GROUP_BY_NORMAL_SPECIAL:
  {
    NormalSpecialComparator* nsComparator = nullptr;
    _trx.dataHandle().get(nsComparator);
    return nsComparator;
  }
  case Group::GROUP_BY_PUBLIC_PRIVATE:
  {
    PublicPrivateComparator* prComparator = nullptr;
    _trx.dataHandle().get(prComparator);
    return prComparator;
  }
  case Group::GROUP_BY_FARE_AMOUNT:
  {
    FareAmountComparator* amtComparator = nullptr;
    _trx.dataHandle().get(amtComparator);
    return amtComparator;
  }
  case Group::GROUP_BY_OW_RT:
  {
    OneWayRoundTripComparator* orComparator = nullptr;
    _trx.dataHandle().get(orComparator);
    return orComparator;
  }

  case Group::GROUP_BY_FARE_BASIS_CODE:
  {
    FareBasisComparator* fbComparator = nullptr;
    _trx.dataHandle().get(fbComparator);
    return fbComparator;
  }

  case Group::GROUP_BY_FARE_BASIS_CHAR_COMB:
  {
    CharCombinationsComparator* charComparator = nullptr;
    _trx.dataHandle().get(charComparator);
    return charComparator;
  }

  case Group::GROUP_BY_TRAVEL_DISCONTINUE_DATE:
  {
    TravelDiscontinueDateComparator* dtComparator = nullptr;
    _trx.dataHandle().get(dtComparator);
    return dtComparator;
  }
  case Group::GROUP_BY_MULTITRANSPORT:
  {
    MultiTransportComparator* mtComparator = nullptr;
    _trx.dataHandle().get(mtComparator);
    return mtComparator;
  }
  case Group::GROUP_BY_BRAND:
  {
    BrandComparator* brandComparator = nullptr;
    _trx.dataHandle().get(brandComparator);
    return brandComparator;
  }
  case Group::GROUP_BY_S8BRAND:
  {
    S8BrandComparator* brandComparator = nullptr;
    _trx.dataHandle().get(brandComparator);
    return brandComparator;
  }
  case Group::GROUP_BY_CABIN:
  {
    CabinGroupComparator* cabinComparator = nullptr;
    _trx.dataHandle().get(cabinComparator);
    return cabinComparator;
  }
  case Group::GROUP_NOT_REQUIRED:
  {
    NullComparator* nullComparator = nullptr;
    _trx.dataHandle().get(nullComparator);
    return nullComparator;
  }
  default:
    return nullptr;
  }
}
}
