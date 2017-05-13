//----------------------------------------------------------------------------
//  Copyright Sabre 2004
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

#include "ItinAnalyzer/ALLInclusionCodePaxType.h"

#include "Common/Logger.h"
#include "Common/PaxTypeFilter.h"
#include "DataModel/FareDisplayTrx.h"

namespace tse
{
static Logger
logger ("atseintl.ItinAnalyzerService.InclusionCodePaxType");

void
ALLInclusionCodePaxType::getPaxType(FareDisplayTrx& trx)
{
  if (trx.getRequest()->displayPassengerTypes().empty())
  {
    // if not requested passenger type, check ACI option
    if (!trx.getOptions()->isChildFares() && !trx.getOptions()->isInfantFares() &&
        !trx.getOptions()->isAdultFares())
    {
      LOG4CXX_DEBUG(logger, " Retrieving ALL Pax Type for ALL Inclusion Code");
      PaxTypeFilter::getAllPaxType(trx, trx.getRequest()->passengerTypes());
      PaxTypeFilter::getAllPaxType(trx, trx.getRequest()->rec8PassengerTypes());
      return;
    }
    LOG4CXX_DEBUG(logger, " Retrieving ACI option required Pax Type for ALL Inclusion Code");
    PaxTypeFilter::getAllAdultPaxType(trx, trx.getRequest()->passengerTypes());
    PaxTypeFilter::getAllAdultPaxType(trx, trx.getRequest()->rec8PassengerTypes());

    if (trx.getOptions()->isChildFares())
      getChildPaxTypes(trx);
    if (trx.getOptions()->isInfantFares())
      getInfantPaxTypes(trx);
  }
  else
  {
    LOG4CXX_DEBUG(logger, " Retrieving requested Pax Type for ALL Inclusion Code");
    PaxTypeFilter::getAllAdultPaxType(trx, trx.getRequest()->passengerTypes());
    PaxTypeFilter::getAllAdultPaxType(trx, trx.getRequest()->rec8PassengerTypes());
    addRequestedPaxTypes(trx);
  }
}
}
