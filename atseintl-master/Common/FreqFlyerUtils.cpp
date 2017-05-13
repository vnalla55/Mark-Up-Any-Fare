//----------------------------------------------------------------------------
//
// Copyright Sabre 2015
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/FreqFlyerUtils.h"

#include "DataModel/PaxType.h"
#include "Diagnostic/Diag852Collector.h"
#include "DBAccess/FreqFlyerStatus.h"
#include "DBAccess/FreqFlyerStatusSeg.h"
#include "Util/BranchPrediction.h"

#include <algorithm>

namespace tse
{
namespace
{
uint16_t
getFrequentFlyerTier(const CarrierCode partnerCarrier,
                     const uint16_t partnerLevel,
                     const std::vector<const FreqFlyerStatusSeg*>& freqFlyerTierStatuses,
                     const CarrierCode carrier,
                     Diag852Collector* dc)
{
  uint16_t determinedLevel = FF_LEVEL_NOT_DETERMINED;
  if (partnerCarrier == carrier)
    determinedLevel = partnerLevel;
  else
  {
    for (const FreqFlyerStatusSeg* freqFlyerTierStatus : freqFlyerTierStatuses)
    {
      const FreqFlyerStatusSeg::PartnerStatusMap& partnerStatusMap =
          freqFlyerTierStatus->partnerStatusMap();
      FreqFlyerStatusSeg::PartnerStatusMapping searchedMapping(partnerCarrier, partnerLevel);
      auto levelIt =
          std::lower_bound(partnerStatusMap.begin(), partnerStatusMap.end(), searchedMapping);
      if (*levelIt == searchedMapping) // compares only partnerCarrier and partnerLevel
      {
        determinedLevel = levelIt->_level;
        break; //first found is good
      }
    }
  }
  if (UNLIKELY(dc))
    dc->printFFStatuses(partnerCarrier, partnerLevel, determinedLevel);

  return determinedLevel;
}
void
setDiagCarrierAndA03Status(Diag852Collector* dc,
                           CarrierCode carrier,
                           DataHandle& dataHandle,
                           PricingTrx* trx)
{
  if (LIKELY(!dc))
    return;

  const std::vector<const FreqFlyerStatus*> statuses =
      dataHandle.getFreqFlyerStatuses(carrier, trx->ticketingDate(), true);
  dc->setCarrierWithA03Status(carrier, !statuses.empty());
}
}

namespace freqflyerutils
{
uint16_t
determineFreqFlyerTierLevel(Diag852Collector* dc,
                            std::vector<PaxType::FreqFlyerTierWithCarrier*>& freqFlyerData,
                            CarrierCode carrier,
                            PricingTrx* trx)
{
  uint16_t mostSignificantLevel = FF_LEVEL_NOT_DETERMINED;

  if (freqFlyerData.empty())
    return mostSignificantLevel;

  setDiagCarrierAndA03Status(dc, carrier, trx->dataHandle(), trx);

  const bool readDB = checkReadingTableA03(freqFlyerData, carrier, dc);
  std::vector<const FreqFlyerStatusSeg*> freqFlyerTierStatuses;
  if (readDB)
    freqFlyerTierStatuses =
        std::move(trx->dataHandle().getFreqFlyerStatusSegs(carrier, trx->ticketingDate()));

  for (const auto data : freqFlyerData)
  {
    mostSignificantLevel =
        std::min(mostSignificantLevel,
                 getFrequentFlyerTier(
                     data->cxr(), data->freqFlyerTierLevel(), freqFlyerTierStatuses, carrier, dc));
  }

  printDeterminedFFStatus(dc, mostSignificantLevel);
  return mostSignificantLevel;
}

bool
checkReadingTableA03(const std::vector<PaxType::FreqFlyerTierWithCarrier*>& freqFlyerData,
                     const CarrierCode carrier,
                     Diag852Collector* dc)
{
  const bool result = std::any_of(freqFlyerData.begin(),
                                  freqFlyerData.end(),
                                  [=](PaxType::FreqFlyerTierWithCarrier* ffData)
                                  { return ffData->cxr() != carrier; });
  if (UNLIKELY(dc))
  {
    if (result)
      dc->printReadTable(freqFlyerData);
    else
      dc->printDoNotReadA03Table();
  }
  return result;
}

void
printDeterminedFFStatus(Diag852Collector* dc, const uint16_t frequentFlyerTierLevel)
{
  if (LIKELY(!dc))
    return;

  dc->printDeterminedFFStatus(frequentFlyerTierLevel);
}

bool
checkCarrierPresenceInTableA03(const std::vector<FreqFlyerStatusData>& carrierData)
{
  return !carrierData.empty();
}
}
}
