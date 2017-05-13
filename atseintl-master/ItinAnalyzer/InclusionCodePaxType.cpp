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

#include "ItinAnalyzer/InclusionCodePaxType.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/PaxTypeFilter.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareDisplayInclCd.h"
#include "Diagnostic/DCFactoryFareDisplay.h"
#include "Diagnostic/Diag207CollectorFD.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "ItinAnalyzer/ALLInclusionCodePaxType.h"
#include "ItinAnalyzer/GenericInclusionCodePaxType.h"
#include "ItinAnalyzer/InclusionCodeRetriever.h"
#include "ItinAnalyzer/NullInclusionCodePaxType.h"
#include "ItinAnalyzer/SpecialInclusionCodePaxType.h"

namespace tse
{
FALLBACK_DECL(fallbackFareDisplayByCabinActivation);

static Logger
logger("atseintl.ItinAnalyzerService.InclusionCodePaxType");

InclusionCodePaxType*
InclusionCodePaxType::getInclusionCodePaxType(FareDisplayTrx& trx)
{
  InclusionCodePaxType* inclusionCodePaxType(nullptr);
  Diag207CollectorFD* diag(nullptr);
  if (trx.diagnostic().isActive() && (trx.diagnostic().diagnosticType() == Diagnostic207 ||
                                      trx.diagnostic().diagnosticType() == Diagnostic209))
  {
    diag = dynamic_cast<Diag207CollectorFD*>(DCFactoryFareDisplay::instance()->create(trx));
    if (diag)
    {
      diag->enable(Diagnostic207, Diagnostic209);
    }
  }
  if (trx.getRequest()->inclusionCode() == ADDON_FARES ||
      trx.getRequest()->inclusionCode() == TRAVELOCITY_INCL)
  {
    LOG4CXX_DEBUG(logger, " Getting SpecialInclusionCodePaxType ");
    SpecialInclusionCodePaxType* specialInclCode(nullptr);
    trx.dataHandle().get(specialInclCode);
    if (diag)
      diag->displayWebInclusionCode(trx);
    inclusionCodePaxType = specialInclCode;
  }
  else if (trx.getRequest()->inclusionCode() == ALL_FARES)
  {
    LOG4CXX_DEBUG(logger, " Getting ALLInclusionCodePaxType ");
    ALLInclusionCodePaxType* allInclCode(nullptr);
    trx.dataHandle().get(allInclCode);
    if (diag)
      diag->displayInclusionCode(trx, nullptr);
    inclusionCodePaxType = allInclCode;
  }
  else if (!trx.getRequest()->inclusionCode().empty())
  {
    LOG4CXX_DEBUG(logger, " Getting InclusionCode Record ");
    InclusionCodeRetriever inclCdRetriever(trx);
    FareDisplayInclCd* inclCode = inclCdRetriever.fetch();
    if (inclCode == nullptr)
    {
      LOG4CXX_DEBUG(logger, " Getting NullInclusionCodePaxType ");
      NullInclusionCodePaxType* nullInclCode(nullptr);
      trx.dataHandle().get(nullInclCode);
      if (nullInclCode != nullptr)
      {
        inclusionCodePaxType = nullInclCode;
      }
    }
    else
    {
      if(!fallback::fallbackFareDisplayByCabinActivation(&trx) &&
         trx.getRequest()->multiInclusionCodes())
         trx.fdResponse()->fdInclCdPerInclCode().insert(
                           std::make_pair(
                           trx.getRequest()->inclusionNumber(trx.getRequest()->inclusionCode()), inclCode));
      else
        trx.fdResponse()->fareDisplayInclCd() = inclCode;

      GenericInclusionCodePaxType* genericInclCode(nullptr);
      trx.dataHandle().get(genericInclCode);
      if (genericInclCode != nullptr)
      {
        genericInclCode->inclusionCode(inclCode);
        inclusionCodePaxType = genericInclCode;
      }
    }
    if (diag)
      diag->displayInclusionCode(trx, inclCode);
  }
  else
  {
    LOG4CXX_ERROR(logger, " Getting NullInclusionCodePaxType ");
    NullInclusionCodePaxType* nullInclCode(nullptr);
    trx.dataHandle().get(nullInclCode);
    if (nullInclCode != nullptr)
    {
      inclusionCodePaxType = nullInclCode;
    }
    if (diag)
      diag->displayInclusionCode(trx, nullptr);
  }

  if (diag)
  {
    diag->flushMsg();
  }

  return inclusionCodePaxType;
}

void
InclusionCodePaxType::getDiscountPaxTypes(FareDisplayTrx& trx) const
{
  if (trx.getOptions()->isChildFares())
  {
    LOG4CXX_DEBUG(logger, " Getting All Child PaxTypes ");
    getChildPaxTypes(trx);
  }

  if (trx.getOptions()->isInfantFares())
  {
    LOG4CXX_DEBUG(logger, " Getting All Infant PaxTypes ");
    getInfantPaxTypes(trx);
  }
}

void
InclusionCodePaxType::getChildPaxTypes(FareDisplayTrx& trx) const
{
  PaxTypeFilter::getAllChildPaxType(trx, trx.getRequest()->passengerTypes());
  PaxTypeFilter::getAllChildPaxType(trx, trx.getRequest()->rec8PassengerTypes());
}

void
InclusionCodePaxType::getInfantPaxTypes(FareDisplayTrx& trx) const
{
  PaxTypeFilter::getAllInfantPaxType(trx, trx.getRequest()->passengerTypes());
  PaxTypeFilter::getAllInfantPaxType(trx, trx.getRequest()->rec8PassengerTypes());
}

void
InclusionCodePaxType::getDefaultPaxCode(FareDisplayTrx& trx) const
{
  trx.getRequest()->passengerTypes().insert(ADULT);
}

void
InclusionCodePaxType::addRequestedPaxTypes(FareDisplayTrx& trx) const
{
  if (!trx.getRequest()->displayPassengerTypes().empty())
  {
    std::copy(trx.getRequest()->displayPassengerTypes().begin(),
              trx.getRequest()->displayPassengerTypes().end(),
              std::inserter(trx.getRequest()->passengerTypes(),
                            trx.getRequest()->passengerTypes().begin()));
  }
  else
  {
  }
}

void
InclusionCodePaxType::getAdditionalPaxType(FareDisplayTrx& trx) const
{
  LOG4CXX_DEBUG(logger, "Now Adding Additional Pax Type: ");
  if (trx.getOptions() && trx.getOptions()->isRtw())
  {
    FareDisplayRequest& req = *trx.getRequest();
    std::set<PaxTypeCode>& paxs = req.passengerTypes();

    if (!req.displayPassengerTypes().empty())
      paxs.insert(req.displayPassengerTypes().begin(), req.displayPassengerTypes().end());
    else
    {
      paxs.insert(req.inputPassengerTypes().begin(), req.inputPassengerTypes().end());
      getDiscountPaxTypes(trx);
    }
  }
  else
  {
    if (trx.getRequest()->displayPassengerTypes().empty())
    {
      getDiscountPaxTypes(trx);
    }
    else
      addRequestedPaxTypes(trx);
  }
}

}
