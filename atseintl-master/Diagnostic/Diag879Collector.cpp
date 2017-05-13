//----------------------------------------------------------------------------
//  File:        Diag879Collector.cpp
//  Created:     2010-01-26
//
//  Description: Diagnostic 879 formatter
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

#include "Diagnostic/Diag879Collector.h"

#include "Common/ServiceFeeUtil.h"
#include "DBAccess/SubCodeInfo.h"

#include <iomanip>
#include <vector>

namespace tse
{
Diag879Collector&
Diag879Collector::operator<<(const PricingTrx& pricingTrx)
{
  DiagCollector& dc(*this);

  dc << std::endl;
  dc << "**********************************************************\n"
     << "879 : SERVICE FEES - OC FEES CURRENCY CONVERSION\n"
     << "**********************************************************\n";
  dc << std::endl;

  std::vector<Itin*>::const_iterator itinIter = pricingTrx.itin().begin();
  std::vector<Itin*>::const_iterator itinIterEnd = pricingTrx.itin().end();

  for (; itinIter != itinIterEnd; ++itinIter)
  {
    Itin* itin = (*itinIter);

    (*this) << (*itin);
  }

  return (*this);
}

Diag879Collector&
Diag879Collector::operator<<(const Itin& itin)
{
  DiagCollector& dc(*this);

  const PricingTrx* pTrx = static_cast<PricingTrx*>(_trx);
  if (pTrx && pTrx->getRequest() && pTrx->getRequest()->multiTicketActive())
  {
    if (itin.getMultiTktItinOrderNum() == 1)
      dc << "TKT1\n";
    else if (itin.getMultiTktItinOrderNum() == 2)
      dc << "TKT2\n";
  }

  // Display itinerary travel segments
  dc << std::endl << "ITINERARY: ";

  std::vector<TravelSeg*>::const_iterator segIter = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator segIterEnd = itin.travelSeg().end();

  for (; segIter != segIterEnd; ++segIter)
  {
    const TravelSeg* travelSeg = (*segIter);
    const AirSeg* airSegment = dynamic_cast<const AirSeg*>(travelSeg);

    dc << travelSeg->origin()->loc() << "-" << travelSeg->destination()->loc();

    if (airSegment != nullptr)
    {
      dc << " (" << airSegment->carrier() << ") ";
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

    dc << "GROUP CODE: " << serviceFeesGroup->groupCode() << std::endl << std::endl;
    ServiceFeeUtil::createOCFeesUsages(*serviceFeesGroup, *(static_cast<PricingTrx*>(_trx)));

    printOCFees(serviceFeesGroup);
  }

  return (*this);
}

void
Diag879Collector::printOCFees(const ServiceFeesGroup* serviceFeesGroup)
{
  DiagCollector& dc(*this);
  dc << "PSG CX MARKET    AMT   CUR   CAMT   CUR     DESCRIPTION\n";
  dc << "--- -- ------ -------- --- -------- --- ------------------\n";
  (*this) << serviceFeesGroup->ocFeesMap();
  dc << std::endl;
}

Diag879Collector&
Diag879Collector::operator<<(const std::map<const FarePath*, std::vector<OCFees*> >& ocFeesMap)
{
  std::map<const FarePath*, std::vector<OCFees*> >::const_iterator ocFeesMapIter =
      ocFeesMap.begin();

  for (; ocFeesMapIter != ocFeesMap.end(); ++ocFeesMapIter)
  {
    std::vector<OCFees*>::const_iterator ocFeesIter = ocFeesMapIter->second.begin();

    for (; ocFeesIter != ocFeesMapIter->second.end(); ++ocFeesIter)
    {
      if ((*ocFeesIter)->ocfeeUsage().empty())
        continue;

      std::vector<OCFeesUsage*>& ocFeesUsages = (*ocFeesIter)->ocfeeUsage();

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

Diag879Collector&
Diag879Collector::operator<<(const std::vector<PaxOCFees>& paxFees)
{
  std::vector<PaxOCFees>::const_iterator paxOcFeesIter = paxFees.begin();

  for (; paxOcFeesIter != paxFees.end(); ++paxOcFeesIter)
  {
    (*this) << (*paxOcFeesIter);
  }

  return (*this);
}

Diag879Collector&
Diag879Collector::operator<<(const std::vector<PaxOCFeesUsages>& paxFees)
{
  std::vector<PaxOCFeesUsages>::const_iterator paxOcFeesIter = paxFees.begin();

  for (; paxOcFeesIter != paxFees.end(); ++paxOcFeesIter)
  {
    (*this) << (*paxOcFeesIter);
  }

  return (*this);
}

Diag879Collector&
Diag879Collector::operator<<(const PaxOCFees& paxFees)
{
  DiagCollector& dc(*this);

  ServiceFeeUtil util(*(static_cast<PricingTrx*>(_trx)));
  Money targetMoney = util.convertOCFeeCurrency(paxFees.fees());

  dc << std::setw(3) << paxFees.paxType() << " " << std::setw(2) << paxFees.fees()->carrierCode()
     << " " << std::setw(3) << paxFees.fees()->travelStart()->origin()->loc() << std::setw(3)
     << paxFees.fees()->travelEnd()->destination()->loc() << " " << std::setw(8)
     << paxFees.fees()->feeAmount() << " " << std::setw(3) << paxFees.fees()->feeCurrency() << " "
     << std::setw(8) << targetMoney.value() << " " << std::setw(3) << targetMoney.code() << " "
     << paxFees.fees()->subCodeInfo()->commercialName() << std::endl;

  return (*this);
}

Diag879Collector&
Diag879Collector::operator<<(const PaxOCFeesUsages& paxFees)
{
  DiagCollector& dc(*this);

  ServiceFeeUtil util(*(static_cast<PricingTrx*>(_trx)));
  Money targetMoney = util.convertOCFeeCurrency(*(paxFees.fees()));

  dc << std::setw(3) << paxFees.paxType() << " " << std::setw(2) << paxFees.fees()->carrierCode()
     << " " << std::setw(3) << paxFees.fees()->travelStart()->origin()->loc() << std::setw(3)
     << paxFees.fees()->travelEnd()->destination()->loc() << " " << std::setw(8)
     << paxFees.fees()->feeAmount() << " " << std::setw(3) << paxFees.fees()->feeCurrency() << " "
     << std::setw(8) << targetMoney.value() << " " << std::setw(3) << targetMoney.code() << " ";
  dc << getCommercialName(paxFees.fees()) << std::endl;

  return (*this);
}

std::string
Diag879Collector::getCommercialName(const OCFeesUsage* ocFeesUsage)
{
  std::string result = ocFeesUsage->oCFees()->subCodeInfo()->commercialName();

  if (ocFeesUsage && ocFeesUsage->upgradeT198CommercialName() != EMPTY_STRING())
    result = ocFeesUsage->upgradeT198CommercialName();

  return result;
}
}
