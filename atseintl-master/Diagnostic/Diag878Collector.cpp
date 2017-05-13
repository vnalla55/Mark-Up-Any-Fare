//----------------------------------------------------------------------------
//  File:        Diag878Collector.cpp
//  Created:     2010-01-15
//
//  Description: Diagnostic 878 formatter
//
//  Updates:
//
//  Copyright Sabre 2010
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

#include "Diagnostic/Diag878Collector.h"

#include "Common/ServiceFeeUtil.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DBAccess/SubCodeInfo.h"

#include <vector>

namespace tse
{

Diag878Collector&
Diag878Collector::operator<<(const PricingTrx& pricingTrx)
{
  DiagCollector& dc(*this);

  printDiagHeader();
  const AncRequest* req = dynamic_cast<const AncRequest*>(pricingTrx.getRequest());

  std::vector<Itin*>::const_iterator itinIter = pricingTrx.itin().begin();
  std::vector<Itin*>::const_iterator itinIterEnd = pricingTrx.itin().end();

  for (; itinIter != itinIterEnd; ++itinIter)
  {
    dc << std::endl;
    dc << "**********************************************************\n";
    Itin* itin = (*itinIter);

    (*this) << (*itin);

    if (pricingTrx.getOptions()->isSummaryRequest())
    {
      dc << "OC FEES SUMMARY FOR EACH PAX TYPE:" << std::endl;

      MoneyAmount totalAmount = 0.0;

      ServiceFeeUtil util((PricingTrx&)pricingTrx);
      totalAmount = ((util.getOCFeesSummary(itin)).value());

      dc << "  TOTAL: " << totalAmount << std::endl << std::endl;

      dc << "FILTERED OC FEES:" << std::endl << std::endl;

      // Display service fees groups
      std::vector<ServiceFeesGroup*>::const_iterator sfgIter = itin->ocFeesGroup().begin();
      std::vector<ServiceFeesGroup*>::const_iterator sfgIterEnd = itin->ocFeesGroup().end();

      for (; sfgIter != sfgIterEnd; ++sfgIter)
      {
        const ServiceFeesGroup* serviceFeesGroup = (*sfgIter);
        ServiceFeeUtil::createOCFeesUsages(*serviceFeesGroup,
                                           const_cast<PricingTrx&>(pricingTrx));

        bool feesFound = false;

        if (!serviceFeesGroup->ocFeesMap().empty())
        {
          std::map<const FarePath*, std::vector<OCFees*> >::const_iterator mapIter =
              serviceFeesGroup->ocFeesMap().begin();
          std::map<const FarePath*, std::vector<OCFees*> >::const_iterator mapIterEnd =
              serviceFeesGroup->ocFeesMap().end();

          for (; mapIter != mapIterEnd; ++mapIter)
          {
            if (!(mapIter->second).empty())
            {
              feesFound = true;
              break;
            }
          }
        }

        if (!feesFound)
        {
          continue;
        }

        dc << "GROUP CODE: " << serviceFeesGroup->groupCode() << std::endl;

        dc << "GROUP DESCRIPTION: " << serviceFeesGroup->groupDescription() << std::endl
           << std::endl;

        printOCFees(serviceFeesGroup);
        if (req)
          printSortedOCFees(serviceFeesGroup, req->paxType(*itinIter));
        else
          printSortedOCFees(serviceFeesGroup, ((const PricingTrx*)trx())->paxType());
      }
    }

    dc << "**********************************************************\n";
  }

  return (*this);
}

Diag878Collector&
Diag878Collector::operator<<(const Itin& itin)
{
  DiagCollector& dc(*this);

  const PricingTrx* pTrx = dynamic_cast<const PricingTrx*>(trx());
  if (pTrx && pTrx->getRequest() && pTrx->getRequest()->multiTicketActive())
  {
    if (itin.getMultiTktItinOrderNum() == 1)
      dc << "TKT1\n";
    else if (itin.getMultiTktItinOrderNum() == 2)
      dc << "TKT2\n";
  }

  // Display itinerary travel segments
  dc << "ITINERARY:";

  std::vector<std::pair<int, int> >::const_iterator idIter;

  const AncRequest* req = nullptr;
  const AncillaryPricingTrx* aTrx = dynamic_cast<const AncillaryPricingTrx*>(trx());
  if (aTrx)
    req = static_cast<const AncRequest*>(aTrx->getRequest());

  for (idIter = itin.legID().begin(); idIter != itin.legID().end(); ++idIter)
  {
    uint32_t sopId =
        ShoppingUtil::findSopId(*((const PricingTrx*)trx()), idIter->first, idIter->second);
    dc << " (LEG: " << idIter->first + 1 << ", SOP: " << sopId << ")";
  }

  dc << std::endl;

  std::vector<TravelSeg*>::const_iterator segIter = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator segIterEnd = itin.travelSeg().end();

  for (; segIter != segIterEnd; ++segIter)
  {
    const TravelSeg* travelSeg = (*segIter);
    const AirSeg* airSegment = dynamic_cast<const AirSeg*>(travelSeg);

    dc << travelSeg->origin()->loc() << "-" << travelSeg->destination()->loc();

    if (airSegment != nullptr)
    {
      dc << " (" << airSegment->carrier() << "/" << airSegment->operatingCarrierCode() << ") ";
    }
    else
    {
      dc << " ";
    }
  }

  dc << std::endl << std::endl;

  // Display service fees groups
  std::vector<ServiceFeesGroup*>::const_iterator sfgIter = itin.ocFeesGroup().begin();
  std::vector<ServiceFeesGroup*>::const_iterator sfgIterEnd = itin.ocFeesGroup().end();

  for (; sfgIter != sfgIterEnd; ++sfgIter)
  {
    const ServiceFeesGroup* serviceFeesGroup = (*sfgIter);
    ServiceFeeUtil::createOCFeesUsages(*serviceFeesGroup, const_cast<PricingTrx&>(*pTrx));

    dc << "GROUP CODE: " << serviceFeesGroup->groupCode() << std::endl;

    dc << "GROUP DESCRIPTION: " << serviceFeesGroup->groupDescription() << std::endl << std::endl;

    printOCFees(serviceFeesGroup);
    if (req)
      printSortedOCFees(serviceFeesGroup, req->paxType(&itin));
    else
      printSortedOCFees(serviceFeesGroup, ((const PricingTrx*)trx())->paxType());
  }

  return (*this);
}

void
Diag878Collector::printOCFees(const ServiceFeesGroup* serviceFeesGroup)
{
  DiagCollector& dc(*this);
  dc << "OC FEES:" << std::endl;

  (*this) << serviceFeesGroup->ocFeesMap();
  dc << std::endl;
}

void
Diag878Collector::printSortedOCFees(const ServiceFeesGroup* const serviceFeesGroup,
                                    const std::vector<PaxType*>& reqPaxTypes)
{
  DiagCollector& dc(*this);
  dc << "SORTED OC FEES:" << std::endl;
  const PricingTrx* pTrx = dynamic_cast<const PricingTrx*>(trx());
  (*this) << ServiceFeeUtil::getSortedFeesUsages(
                 *serviceFeesGroup, reqPaxTypes, (!(pTrx->getOptions()->isSummaryRequest())));
  dc << std::endl;
}

Diag878Collector&
Diag878Collector::operator<<(const std::map<const FarePath*, std::vector<OCFees*> >& ocFeesMap)
{
  std::map<const FarePath*, std::vector<OCFees*> >::const_iterator ocFeesMapIter =
      ocFeesMap.begin();
  std::map<const FarePath*, std::vector<OCFees*> >::const_iterator ocFeesMapIterEnd =
      ocFeesMap.end();

  for (; ocFeesMapIter != ocFeesMapIterEnd; ++ocFeesMapIter)
  {
    const std::vector<OCFees*>& ocFeesVec = ocFeesMapIter->second;

    std::vector<OCFees*>::const_iterator ocFeesIter = ocFeesVec.begin();
    std::vector<OCFees*>::const_iterator ocFeesIterEnd = ocFeesVec.end();

    for (; ocFeesIter != ocFeesIterEnd; ++ocFeesIter)
    {
      OCFees* ocFees = (*ocFeesIter);
      if ((*ocFeesIter)->ocfeeUsage().empty())
        continue;
      std::vector<OCFeesUsage*>& ocFeesUsages = ocFees->ocfeeUsage();

      std::vector<OCFeesUsage*>::const_iterator ocFeesUIter = ocFeesUsages.begin();
      std::vector<OCFeesUsage*>::const_iterator ocFeesUIterEnd = ocFeesUsages.end();
      for (; ocFeesUIter != ocFeesUIterEnd; ++ocFeesUIter)
      {
        OCFeesUsage* ocFeesU = (*ocFeesUIter);
        FPOCFeesUsages fpOcFeesU(ocFeesMapIter->first, ocFeesU);
        (*this) << fpOcFeesU;
      }
    }
  }

  return (*this);
}

Diag878Collector&
Diag878Collector::operator<<(const std::vector<PaxOCFees>& paxFees)
{
  std::vector<PaxOCFees>::const_iterator paxOcFeesIter = paxFees.begin();
  std::vector<PaxOCFees>::const_iterator paxOcFeesIterEnd = paxFees.end();

  for (; paxOcFeesIter != paxOcFeesIterEnd; ++paxOcFeesIter)
  {
    (*this) << (*paxOcFeesIter);
  }

  return (*this);
}

Diag878Collector&
Diag878Collector::operator<<(const std::vector<PaxOCFeesUsages>& paxFees)
{
  std::vector<PaxOCFeesUsages>::const_iterator paxOcFeesIter = paxFees.begin();
  std::vector<PaxOCFeesUsages>::const_iterator paxOcFeesIterEnd = paxFees.end();

  for (; paxOcFeesIter != paxOcFeesIterEnd; ++paxOcFeesIter)
  {
    (*this) << (*paxOcFeesIter);
  }

  return (*this);
}

Diag878Collector&
Diag878Collector::operator<<(const PaxOCFees& paxFees)
{
  DiagCollector& dc(*this);

  dc << paxFees.paxType() << ", " << paxFees.fees()->subCodeInfo()->commercialName() << ", "
     << paxFees.fees()->carrierCode() << ", " << paxFees.fees()->subCodeInfo()->serviceSubTypeCode()
     << ", " << paxFees.fees()->travelStart()->origin()->loc() << ", "
     << paxFees.fees()->travelEnd()->destination()->loc() << ", " << paxFees.fees()->feeAmount()
     << std::endl;

  return (*this);
}

Diag878Collector&
Diag878Collector::operator<<(const PaxOCFeesUsages& paxFees)
{
  DiagCollector& dc(*this);

  dc << paxFees.paxType() << ", ";

  dc << getCommercialName(paxFees.fees()) << ", ";

  dc << paxFees.fees()->carrierCode() << ", " << paxFees.fees()->subCodeInfo()->serviceSubTypeCode()
     << ", " << paxFees.fees()->travelStart()->origin()->loc() << ", "
     << paxFees.fees()->travelEnd()->destination()->loc() << ", " << paxFees.fees()->feeAmount()
     << std::endl;

  return (*this);
}

std::string
Diag878Collector::getCommercialName(const OCFeesUsage* ocFeesUsage)
{
  std::string result = ocFeesUsage->oCFees()->subCodeInfo()->commercialName();

  if (ocFeesUsage && ocFeesUsage->upgradeT198CommercialName() != EMPTY_STRING())
    result = ocFeesUsage->upgradeT198CommercialName();

  return result;
}

void
Diag878Collector::printDiagHeader()
{
  DiagCollector& dc(*this);

  dc << std::endl;
  dc << "**********************************************************\n"
     << "878 : SERVICE FEES - OC FEES SORTING\n"
     << "**********************************************************\n";
}

void
Diag878Collector::printSkipOcCollectionReason(std::string& reason)
{
  DiagCollector& dc(*this);

  printDiagHeader();

  dc << std::endl << "OC FEES COLLECTOR SKIPPED, REASON: " << std::endl << reason << std::endl
     << std::endl;
}
} // namespace
