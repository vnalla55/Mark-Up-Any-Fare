//-------------------------------------------------------------------
//
//  Copyright Sabre 2005
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/WebICValidator.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareDisplayWeb.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Rules/RuleUtil.h"

namespace tse
{

static Logger
logger("atseintl.FareDisplay.WebICValidator");

using namespace std;

bool
WebICValidator::validate(const PaxTypeFare& paxTypeFare)
{
  PaxTypeCode farePaxType("");
  farePaxType = paxTypeFare.fcasPaxType();

  bool isChild = _trx->getOptions()->isChildFares();
  bool isInfant = _trx->getOptions()->isInfantFares();
  bool isAdult = _trx->getOptions()->isAdultFares() || (!isChild && !isInfant);

  if (!((paxTypeFare.actualPaxType()->paxTypeInfo().isInfant() && isInfant) ||
        (paxTypeFare.actualPaxType()->paxTypeInfo().isChild() && isChild) ||
        (paxTypeFare.actualPaxType()->paxTypeInfo().isAdult() && isAdult)))

    return false;

  if (farePaxType.empty())
    farePaxType = ADULT;

  const std::vector<FareDisplayWeb*> webRecs = getFareDisplayWeb(*_trx,
                                                                 paxTypeFare.fcaDisplayCatType(),
                                                                 paxTypeFare.vendor(),
                                                                 paxTypeFare.carrier(),
                                                                 paxTypeFare.fareTariff(),
                                                                 paxTypeFare.ruleNumber(),
                                                                 farePaxType);
  if (!webRecs.empty())
  {
    std::vector<FareDisplayWeb*>::const_iterator iter = webRecs.begin();
    std::vector<FareDisplayWeb*>::const_iterator iterEnd = webRecs.end();

    for (int i = 0; iter != iterEnd; iter++, i++)
    {
      // Match fare class
      if (!((*iter)->fareClass().empty()))
      {
        if (!RuleUtil::matchFareClass((*iter)->fareClass().c_str(),
                                      paxTypeFare.fareClass().c_str()))
        {
          LOG4CXX_DEBUG(logger,
                        "Not a match on fare class: " << paxTypeFare.fareClass()
                                                      << ", rule: " << (*iter)->fareClass());
          continue;
        }
      }
      // Match ticket designator
      if (!((*iter)->tktDesignator().empty()))
      {
        if (!RuleUtil::matchFareClass((*iter)->tktDesignator().c_str(),
                                      paxTypeFare.fcasTktDesignator().c_str()))
        {
          LOG4CXX_DEBUG(logger,
                        "Not a match on tkt designator: " << paxTypeFare.fcasTktDesignator()
                                                          << ", rule: "
                                                          << (*iter)->tktDesignator());
          continue;
        }
      }

      LOG4CXX_DEBUG(logger, "Match on fare class: " << paxTypeFare.fareClass());

      LOG4CXX_INFO(logger, "Found a WEB match");
      return true;
    }
    LOG4CXX_DEBUG(logger, "Fare will be Failed due to No-Match Against Web-Incl-Records");
    return false;
  }
  LOG4CXX_INFO(logger,
               "No WEB records for carrier: " << paxTypeFare.carrier()
                                              << ", vendor: " << paxTypeFare.vendor()
                                              << " -- Fare will FAIL WEB Incl Code ");
  // paxTypeFare.invalidateFare(PaxTypeFare::FD_Inclusion_Code);
  return false;
}

bool
WebICValidator::initialize(const FareDisplayTrx& trx)
{
  _trx = &trx;
  return true;
}
const std::vector<FareDisplayWeb*>&
WebICValidator::getFareDisplayWeb(const FareDisplayTrx& trx,
                                  const Indicator& dispInd,
                                  const VendorCode& vendor,
                                  const CarrierCode& carrier,
                                  const TariffNumber& ruleTariff,
                                  const RuleNumber& rule,
                                  const PaxTypeCode& paxTypeCode)
{
  return trx.dataHandle().getFareDisplayWeb(
      dispInd, vendor, carrier, ruleTariff, rule, paxTypeCode);
}
}
