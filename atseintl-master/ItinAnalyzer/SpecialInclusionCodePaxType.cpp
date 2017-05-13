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

#include "ItinAnalyzer/SpecialInclusionCodePaxType.h"

#include "Common/NonFatalErrorResponseException.h"
#include "Common/PaxTypeFilter.h"
#include "Common/PaxTypeUtil.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxType.h"
#include "DBAccess/PaxTypeInfo.h"
#include "FareDisplay/InclusionCodeConsts.h"

namespace tse
{

static Logger
logger("atseintl.ItinAnalyzerService.SpecialInclusionCodePaxType");

void
SpecialInclusionCodePaxType::getPaxType(FareDisplayTrx& trx)
{
  LOG4CXX_DEBUG(logger,
                " Inserting only Pax Types from FareDisplayWebSeg table for specified carrier");
  if (trx.getRequest()->inclusionCode() == TRAVELOCITY_INCL)
  {
    if (trx.preferredCarriers().empty())
    {
      throw NonFatalErrorResponseException(ErrorResponseException::NEED_PREFERRED_CARRIER);
    }
    CarrierCode carrier = *trx.preferredCarriers().begin();
    const std::set<std::pair<PaxTypeCode, VendorCode> >& paxCodeTypes =
        getFareDisplayWebPaxForCxr(trx, carrier);
    if (paxCodeTypes.empty())
    {
      trx.response() << "$CARRIER " << carrier << " HAS NOT SPECIFIED FARES FOR WEB INCLUSION CODE$"
                     << std::endl;
      throw NonFatalErrorResponseException(
          ErrorResponseException::CXR_NOT_SPECIFIED_FARES_FOR_INCL_CODE);
    }
    bool isChild = trx.getOptions()->isChildFares();
    bool isInfant = trx.getOptions()->isInfantFares();

    std::set<std::pair<PaxTypeCode, VendorCode> >::iterator iter = paxCodeTypes.begin();
    std::set<std::pair<PaxTypeCode, VendorCode> >::iterator iterEnd = paxCodeTypes.end();

    for (; iter != iterEnd; iter++)
    {
      const PaxTypeInfo* ptInfo = getPaxType(trx, (*iter).first, (*iter).second);
      if (ptInfo)
      {
        if ((ptInfo->isChild() && isChild) || (ptInfo->isInfant() && isInfant) || ptInfo->isAdult())
        {
          trx.getRequest()->passengerTypes().insert((*iter).first);
          trx.getRequest()->rec8PassengerTypes().insert((*iter).first);
        }
      }
    }
    addRequestedPaxTypes(trx);
  }
  else
    trx.getRequest()->passengerTypes().insert(ADULT);
}
const PaxTypeInfo*
SpecialInclusionCodePaxType::getPaxType(FareDisplayTrx& trx,
                                        const PaxTypeCode& paxTypeCode,
                                        const VendorCode& vendor)
{
  return trx.dataHandle().getPaxType(paxTypeCode, vendor);
}
const std::set<std::pair<PaxTypeCode, VendorCode> >&
SpecialInclusionCodePaxType::getFareDisplayWebPaxForCxr(FareDisplayTrx& trx,
                                                        const CarrierCode& carrier)
{
  return trx.dataHandle().getFareDisplayWebPaxForCxr(carrier);
}

}
