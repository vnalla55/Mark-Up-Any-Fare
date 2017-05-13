//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Diagnostic/Diag600Collector.h"

#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/FareMarketPathMatrix.h"
#include "Pricing/PUPath.h"
#include "Pricing/PUPathMatrix.h"

namespace tse
{

namespace
{
void
displayCheapestFare(Diag600Collector& dc,
                    const FareMarketPath* fmPath,
                    const std::vector<PaxType*>& paxTypes)
{
  for (PaxType* paxType : paxTypes)
    if (fmPath->firstFareAmt(paxType) != std::numeric_limits<MoneyAmount>::max())
      dc << paxType->paxType() << "  "
         << "CHEAPEST AMT:  " << fmPath->firstFareAmt(paxType) << "  NUC"
         << "\n";
}

void
displayFareMarketPath(Diag600Collector& dc,
                      const PricingTrx* trx,
                      const FareMarketPath* fmPath,
                      const Itin* itin,
                      const std::vector<FareMarketPath*>* fmps)
{
  uint16_t count = 0;
  uint16_t mod = 4;

  std::vector<MergedFareMarket*>::const_iterator mktIt = fmPath->fareMarketPath().begin();
  for (; mktIt != fmPath->fareMarketPath().end(); ++mktIt)
  {
    ++count;
    MergedFareMarket* fm = *mktIt;
    dc << " " << itin->segmentOrder(fm->travelSeg().front()) << "--"
       << itin->segmentOrder(fm->travelSeg().back()) << ":" << fm->boardMultiCity() << " "
       << fm->offMultiCity() << " ";

    if ((count % mod == 0) && (mktIt != fmPath->fareMarketPath().end() - 1))
    {
      dc << std::endl;
      dc << "              ";
      count = 0;
      mod = 3;
    }
  }

  dc << std::endl;

  // Display SideTrips
  if (!fmPath->sideTrips().empty())
  {
    std::map<MergedFareMarket*, std::vector<FareMarketPath*> >::const_iterator stIt =
        fmPath->sideTrips().begin();
    for (; stIt != fmPath->sideTrips().end(); ++stIt)
    {
      dc << "      " << stIt->first->boardMultiCity() << " " << stIt->first->offMultiCity()
         << " SIDE TRIP: ";
      std::vector<FareMarketPath*>::const_iterator pathIter = stIt->second.begin();
      std::vector<FareMarketPath*>::const_iterator pathIterEnd = stIt->second.end();
      uint8_t count = 1;
      for (; pathIter != pathIterEnd; ++pathIter)
      {
        mktIt = (*pathIter)->fareMarketPath().begin();
        for (; mktIt != (*pathIter)->fareMarketPath().end(); ++mktIt)
        {
          MergedFareMarket* fm = *mktIt;
          dc << itin->segmentOrder(fm->travelSeg().front()) << "--"
             << itin->segmentOrder(fm->travelSeg().back()) << ":" << fm->boardMultiCity() << " "
             << fm->offMultiCity() << "  ";

          if (count % 2 == 0 && (pathIter != pathIterEnd - 1))
            dc << "\n                         ";

          ++count;
        }
      }

      dc << std::endl;
    }
  }

  // Display cheap fare
  if (trx && (trx->getTrxType() == PricingTrx::MIP_TRX) && trx->delayXpn())
    displayCheapestFare(dc, fmPath, trx->paxType());

  // Display message for deleted faremarket path
  if (trx && trx->getRequest()->isBrandedFaresRequest() &&
      (trx->getTrxType() == PricingTrx::MIP_TRX))
  {
    if (!fmPath->isValidForIbf())
      dc << "-------- NO HARD PASS EXISTS, DELETED FAREMARKET PATH  --------\n";
  }
  dc << "\n";
}
} // anonymous namespace

void
Diag600Collector::displayPUPathMatrix(const PUPathMatrix& puMatrix,
                                      const Itin* itin,
                                      const FareMarketPath* fmp,
                                      const std::vector<FareMarketPath*>* fmps)
{
  *this << "\n************************ PUPATH MATRIX ************************\n";

  PricingTrx* ptrx = dynamic_cast<PricingTrx*>(_trx);

  if (fmp)
    displayFareMarketPath(*this, ptrx, fmp, itin, fmps);

  if (ptrx && ptrx->isFlexFare())
    *this << "\n FLEX FARES GROUP ID: " << puMatrix.getFlexFaresGroupId() << std::endl;

  for (auto* const puPath : puMatrix.puPathMatrix())
  {
    *this << "\n************************** PUPATH  ****************************\n";
    ItinPUPath tmp;
    tmp.itin = puMatrix.itin();
    tmp.puPath = puPath;
    *this << tmp;
  }

  *this << " \n \n TOTAL PUPATH COUNT: " << puMatrix.puPathMatrix().size() << "\n";
}

void
Diag600Collector::displayFareMarkets(const Itin& itin,
                                     const std::vector<FareMarketPath*>& fareMarketPathMatrix,
                                     const BrandCode& brandCode)
{
  Diag600Collector& dc = *this;
  const PricingTrx& pTrx = *static_cast<PricingTrx*>(_trx);

  if ((pTrx.getRequest()->isBrandedFaresRequest()) && (pTrx.getTrxType() == PricingTrx::MIP_TRX) &&
      (!fareMarketPathMatrix.empty()))
  {
    dc << "****************** ITINERARY: " << itin.itinNum() << " - BRAND: ";
    dc << brandCode;
    dc << " ********************\n";
  }
  else if (pTrx.getTrxType() == PricingTrx::MIP_TRX)
  {
    dc << "*********************** ITINERARY: " << itin.itinNum()
       << " *************************\n";
  }
  else
  {
    dc << "************************* ITINERARY **************************\n";
  }

  dc << itin;

  dc << "************************ FARE MARKET **************************\n";

  for (const auto fareMarket : itin.fareMarket())
  {
    FareMarket& fmkt = *(fareMarket);

    dc << fmkt << ": ";
    for (const auto fmTravelSegment : fmkt.travelSeg())
      dc << itin.segmentOrder(fmTravelSegment) << " ";

    // side Trips
    for (const auto& fmSideTripTravelSegments : fmkt.sideTripTravelSeg())
    {
      dc << " * ";
      for (const auto sideTripTravelSegment : fmSideTripTravelSegments)
        dc << itin.segmentOrder(sideTripTravelSegment) << " ";
      dc << "* ";
    }

    dc << fmkt.governingCarrier();

    if (pTrx.excTrxType() == PricingTrx::AR_EXC_TRX)
      dc << " CAT31FA: " << fmkt.fareRetrievalFlagToStr(fmkt.retrievalFlag());

    if (fmkt.breakIndicator())
      dc << " - MKT NOT FOR COMB";

    dc << "\n";
  }

  if (pTrx.delayXpn())
    for (FareMarketPath* fmp : fareMarketPathMatrix)
      fmp->calculateFirstFareAmt(pTrx.paxType());
}

void
Diag600Collector::displayFareMarkestWithoutTag2Fare(const Itin& itin,
                                                    const std::vector<MergedFareMarket*>& mfms)
{
  std::ostringstream ostr;
  bool tag2FareInd = false;
  ostr << "************* US-CA FARE MARKET HAVING NO TAG2 FARE ***********\n";

  for (const auto mergedFareMarket : mfms)
  {
    if (mergedFareMarket->tag2FareIndicator() == MergedFareMarket::Tag2FareIndicator::Absent)
    {
      ostr << " " << mergedFareMarket->boardMultiCity() << " " << mergedFareMarket->offMultiCity()
           << ": ";
      for (const auto travelSegment : mergedFareMarket->travelSeg())
        ostr << itin.segmentOrder(travelSegment) << " ";
      ostr << "\n";
      tag2FareInd = true;
    }
  }

  if (tag2FareInd)
    *this << ostr.str();
}

void
Diag600Collector::displayFMPMatrix(const FareMarketPathMatrix& fMatrix,
                                   const Itin* itin,
                                   const std::vector<FareMarketPath*>* fmps)
{
  *this << "******************** FARE MARKET PATH MATRIX ******************" << std::endl;

  if (fMatrix.fareMarketPathMatrix().empty())
  {
    *this << "\nMISSING FAREMARKET- NO FAREMARKET PATH - ATSEI\n\n";
    printLine();
    return;
  }

  const PricingTrx* pTrx = dynamic_cast<PricingTrx*>(_trx);

  for (const auto* fareMarketPath : fMatrix.fareMarketPathMatrix())
    displayFareMarketPath(*this, pTrx, fareMarketPath, itin, fmps);
}

} // namespace tse
