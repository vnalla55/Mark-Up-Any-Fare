//-----------------------------------------------------------------------------
//
//  File:     Diag980Collector.cpp
//
//  Author :  Kul Shekhar
//
//  Copyright Sabre 2005
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------

#include "Diagnostic/Diag980Collector.h"

#include "Common/ClassOfService.h"
#include "Common/FallbackUtil.h"
#include "Common/ItinUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/OAndDMarket.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MarriedCabin.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcCollector.h"

using namespace std;
namespace tse
{
Diag980Collector&
Diag980Collector::operator << (const PricingTrx& trx)
{
  if (!_active)
    return *this;
  _showFareDetails = trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "FARES";

  DiagCollector& dc = (DiagCollector&)*this;
  activation(trx);

  if (trx.itin().size() < 1)
  {
    dc << "ATSEI - NO OPTIONS FOUND \n";
    return *this;
  }

  if (trx.getRequest()->owPricingRTTaxProcess() || trx.getRequest()->getBrandedFareSize() > 1 ||
      trx.getRequest()->isBrandedFaresRequest())
    displayOptions(trx);
  else
    showOptions(trx);

  return *this;
}

void
Diag980Collector::activation(const PricingTrx& trx)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;

  if(trx.getRequest()->isBrandedFaresRequest() && trx.getRequest()->isChangeSoldoutBrand())
  {
    dc << "\n";
    dc << "CHANGE BRAND FOR SOLDOUT FARES LOGIC ACTIVE\n";
    dc << "FARES MAY HAVE DIFFERENT BRAND THAN THE FAREPATH";
    dc << "THEY ARE PART OF";
    dc << "\n";
  }

  dc << " \n";
  dc << "------------------------------------------------------------- \n";
  dc << "JOURNEY ACTIVE: ";
  if (trx.getTrxType() == PricingTrx::PRICING_TRX)
  {
    if (trx.getOptions()->journeyActivatedForPricing())
      dc << "Y";
    else
      dc << "N";
  }
  else if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    if (trx.getOptions()->journeyActivatedForShopping())
      dc << "Y";
    else
      dc << "N";
  }

  dc << "   JOURNEY APPLIED: ";
  if (trx.getOptions()->applyJourneyLogic())
    dc << "Y";
  else
    dc << "N";

  dc << "   JOURNEY CUSTOMER: ";
  if (trx.getRequest()->ticketingAgent() == 0)
  {
    dc << "NA";
  }
  else
  {
    if (trx.getRequest()->ticketingAgent()->agentTJR() == 0)
    {
      dc << "NA";
    }
    else
    {
      if (trx.getTrxType() == PricingTrx::PRICING_TRX)
      {
        if (trx.getRequest()->ticketingAgent()->agentTJR()->activateJourneyPricing() == YES)
          dc << "Y";
        else
          dc << "N";
      }
      else if (trx.getTrxType() == PricingTrx::MIP_TRX)
      {
        if (trx.getRequest()->ticketingAgent()->agentTJR()->activateJourneyShopping() == YES)
          dc << "Y";
        else
          dc << "N";
      }
    }
  }
  dc << " \n";
  dc << "------------------------------------------------------------- \n";
  return;
}

namespace
{

struct PriceCollector
{
  const PricingTrx& _trx;
  multimap<MoneyAmount, vector<FarePath*> >& _solutionMap;
  bool _groupByItin;
  PriceCollector(const PricingTrx& trx,
                 multimap<MoneyAmount, vector<FarePath*> >& solutionMap,
                 bool groupByItin = false)
    : _trx(trx), _solutionMap(solutionMap), _groupByItin(groupByItin)
  {
  }

  void operator()(const Itin* itin)
  {
    if (!itinValidOption(*itin))
      return;

    getAmounts(itin);
  }

  void getAmounts(const Itin* itin)
  {
    MoneyAmount totalAmount = 0.0;

    map<const Itin*, FareCalcCollector*>::const_iterator iter =
        _trx.fareCalcCollectorMap().find(itin);

    if (iter == _trx.fareCalcCollectorMap().end())
      return;

    const FareCalcCollector& calc = *iter->second;

    const FareCalcCollector::CalcTotalsMap& ctMap = calc.calcTotalsMap();

    ostringstream ostr;
    if (!_groupByItin)
    {
      // this probably keeps info for all pax - need differentiate
      for (const auto& elem : ctMap)
      {
        const CalcTotals& ct = *elem.second;
        totalAmount = ct.convertedBaseFare + ct.taxAmount();
        vector<FarePath*> fpv;
        fpv.push_back(const_cast<FarePath*>(elem.first));
        _solutionMap.insert(pair<MoneyAmount, vector<FarePath*> >(totalAmount, fpv));

        ostr << "---Adding solution FP#(" << ((long)elem.first) << ")"
             << " ITIN#(" << ((long)itin) << ") Amount-" << totalAmount << endl;
      }
    }
    else
    {
      ostr << "---Adding solution group:"
           << " ITIN#(" << ((long)itin) << ")";

      vector<FarePath*> fpv;
      for (const auto& elem : ctMap)
      {
        const CalcTotals& ct = *elem.second;
        totalAmount += ct.convertedBaseFare + ct.taxAmount();
        fpv.push_back(const_cast<FarePath*>(elem.first));
        ostr << "FP#(" << ((long)elem.first) << ")::";
      }

      ostr << " Amount-" << totalAmount << endl;
      _solutionMap.insert(pair<MoneyAmount, vector<FarePath*> >(totalAmount, fpv));
    }
  }

  bool itinValidOption(const Itin& itin)
  {
    if (itin.farePath().empty() || itin.errResponseCode() != ErrorResponseException::NO_ERROR)
      return false;

    return true;
  }
};
} // end namespace

void
Diag980Collector::displayOptions(const PricingTrx& trx)
{

  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  bool groupByDate =
      trx.isAltDates() && trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "BYDATE";
  bool groupByItin = trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "BYITIN";

  multimap<MoneyAmount, vector<FarePath*> > solutionMap;
  PriceCollector priceCollector(trx, solutionMap, groupByItin || groupByDate);
  for_each(trx.itin().begin(), trx.itin().end(), priceCollector);
  dc << "TOTAL SOLUTIONS: " << solutionMap.size() << " \n";
  if (trx.isAltDates() && groupByDate)
  {
    displayAltDateOptions(trx, solutionMap);
    return;
  }

  // now display the options
  multimap<MoneyAmount, vector<FarePath*> >::const_iterator solutionIt = solutionMap.begin();
  for (uint16_t option = 1; solutionIt != solutionMap.end(); ++solutionIt, option++)
  {
    const vector<FarePath*>& fpaths = solutionIt->second;
    const Itin& itin = *fpaths.front()->itin();

    if (!groupByItin)
    {

      const FarePath* fpath = fpaths.front();

      dc << "OPTION " << option << ": ";
      printItinNum(itin);

      if (trx.getRequest()->getBrandedFareSize() > 1)
        dc << ", BRAND ID: " << trx.getRequest()->brandId(fpath->brandIndex());

      if (trx.getRequest()->isBrandedFaresRequest())
      {
        if (fpath->getBrandCode() == NO_BRAND)
          dc << ", NO BRAND ";
        else
        {
          dc << ", BRAND " << fpath->getBrandCode();
          std::string programIds = ShoppingUtil::getBrandPrograms(trx, fpath);
          if (!programIds.empty())
            dc << " (" << programIds << ")";
        }
      }

      dc << " \n";

      printItin(trx, fpath);

      aseAmounts(trx, fpath);

      printFarePath(fpath);

      dc << " \n";

      bool journeyExist = journeys(trx, itin);

      if (journeyExist)
        availability(trx, fpath);

      dc << " \n";
    }
    else
    {

      dc << "------------------------------------------------------------\n";
      dc << "ITIN OPTION GRP " << option << "\n";
      for (const FarePath* fpath : fpaths)
      {
        printItin(trx, fpath);

        aseAmounts(trx, fpath);

        printFarePath(fpath);

        if (trx.getRequest()->getBrandedFareSize() > 1)
          dc << " BRAND ID: " << trx.getRequest()->brandId(fpath->brandIndex()) << " \n";

        bool journeyExist = journeys(trx, itin);

        if (journeyExist)
          availability(trx, fpath);
        dc << " \n";
      }
    }

  } // end option
}

void
Diag980Collector::displayAltDateOptions(const PricingTrx& trx,
                                        multimap<MoneyAmount, std::vector<FarePath*> >& solutionMap)
{
  if (!trx.isAltDates())
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  typedef std::vector<const FarePath*> FarePathVec;
  typedef std::map<DatePair, std::vector<const FarePath*> > AltDateMap;
  typedef std::pair<const DatePair, std::vector<const FarePath*> > AltDatesPair;
  typedef std::pair<const DatePair, PricingTrx::AltDateInfo*> AltDateInfoPair;

  AltDateMap altDateSolutionMap;

  for (const AltDateInfoPair& pair : trx.altDatePairs())
  {
    const DatePair& dp = pair.first;
    altDateSolutionMap[dp] = FarePathVec();
  }

  multimap<MoneyAmount, vector<FarePath*> >::const_iterator solutionIt = solutionMap.begin();
  for (; solutionIt != solutionMap.end(); ++solutionIt)
  {
    const vector<FarePath*>& fpaths = solutionIt->second;
    const Itin& itin = *fpaths.front()->itin();
    const DatePair& dp = *itin.datePair();

    altDateSolutionMap[dp].insert(altDateSolutionMap[dp].end(), fpaths.begin(), fpaths.end());
  }

  for (const AltDatesPair& pair : altDateSolutionMap)
  {
    uint16_t option = 1;
    const DatePair& dp = pair.first;
    const vector<const FarePath*>& fpaths = pair.second;
    const Itin& itin = *fpaths.front()->itin();

    dc << dp.first.dateToString(DDMMM, "") << "\n";
    if (fpaths.empty())
      dc << " NO OPTION FOUND\n";
    for (const FarePath* fpath : fpaths)
    {

      dc << "OPTION " << option++ << " ";
      printItinNum(itin);
      dc << "\n";

      printItin(trx, fpath);

      aseAmounts(trx, fpath);

      printFarePath(fpath);

      if (trx.getRequest()->getBrandedFareSize() > 1)
        dc << " BRAND ID: " << trx.getRequest()->brandId(fpath->brandIndex());

      dc << " \n";

      bool journeyExist = journeys(trx, itin);

      if (journeyExist)
        availability(trx, fpath);
      dc << " \n";
    }
    dc << "------------------------------------------------------------\n";
  }
  return;
}

void
Diag980Collector::printItin(const PricingTrx& trx, const FarePath* fpath)
{
  const Itin& itin = *fpath->itin();
  DiagCollector& dc = (DiagCollector&)*this;

  uint16_t fltSeq = 1;
  const FareUsage* fu = 0;
  const FareUsage* prevFu = 0;
  const AirSeg* airSeg = 0;
  uint16_t tvlSegIndex = 0;
  const DifferentialData* diff = 0;
  BookingCode bc;

  std::vector<TravelSeg*>::const_iterator tI = itin.travelSeg().begin();
  for (; tI != itin.travelSeg().end(); tI++)
  {
    airSeg = dynamic_cast<AirSeg*>(*tI);
    if (airSeg == 0)
    {
      dc << "ARUNK \n";
      continue;
    }

    fu = getFareUsage(fpath, *tI, tvlSegIndex);
    if (prevFu == 0)
    {
      prevFu = fu;
    }
    else
    {
      if (fu != prevFu)
        ++fltSeq;
    }
    prevFu = fu;
    dc << fltSeq;
    if (fltSeq < 10)
      dc << "  ";
    else
      dc << " ";
    dc << airSeg->carrier();

    if (airSeg->localJourneyCarrier())
      dc << " L ";
    else if (airSeg->flowJourneyCarrier())
      dc << " F ";
    else
      dc << " - ";

    dc << setw(4) << airSeg->flightNumber() << " ";
    dc << airSeg->getBookingCode()[0] << " ";
    if (fu == 0)
    {
      dc << setw(2) << "-";
      dc << " ";
      dc << "- ";
    }
    else
    {
      diff = differentialData(fu, *tI);
      const PaxTypeFare::SegmentStatus& segStat = fu->segmentStatus()[tvlSegIndex];

      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
          !(segStat._bkgCodeReBook.empty()))
      {
        bc = segStat._bkgCodeReBook;
      }
      else
      {
        bc = airSeg->getBookingCode();
      }

      dc << setw(2) << bc << " ";

      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
        dc << "T ";
      else
        dc << "F ";
    }
    dc << setw(5) << airSeg->departureDT().dateToString(DDMMM, "") << " ";
    dc << setw(3) << airSeg->origAirport() << " ";
    dc << setw(3) << airSeg->destAirport() << " ";

    string depTime = airSeg->departureDT().timeToString(HHMM_AMPM, "");
    string arrTime = airSeg->arrivalDT().timeToString(HHMM_AMPM, "");

    // Cut off the M from the time
    depTime = depTime.substr(0, depTime.length() - 1);
    arrTime = arrTime.substr(0, arrTime.length() - 1);

    if (diff == 0)
    {
      dc << setw(5) << depTime << " ";
      dc << setw(5) << arrTime << " ";
    }
    else
    {
      const PaxTypeFare::SegmentStatus& diffSegStat = diffSegStatus(diff, *tI);
      if (diffSegStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
          !(diffSegStat._bkgCodeReBook.empty()))
      {
        bc = diffSegStat._bkgCodeReBook;
      }
      else
      {
        bc = airSeg->getBookingCode();
      }
      dc << "DIFF " << bc[0] << "-";

      if (diffSegStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
        dc << "T";
      else
        dc << "F";

      dc << setw(8) << diff->fareClassHigh();
    }

    if (fu != 0)
    {
      printFareInfo(fu->paxTypeFare(), diff == 0);

      if(trx.getRequest()->isChangeSoldoutBrand())
      {
        IbfErrorMessage ibfErrorMessage =
          itin.getIbfAvailabilityTracker().getStatusForBrand(fpath->getBrandCode());

        if(ibfErrorMessage == IbfErrorMessage::IBF_EM_NOT_OFFERED || ibfErrorMessage == IbfErrorMessage::IBF_EM_NOT_AVAILABLE)
        {
          dc << " (" << fu->paxTypeFare()->getFirstValidBrand(trx, fu->getFareUsageDirection()) << ")";
        }
      }
    }
    // else dc << "-FU_NF ";
    dc << " \n";
    marriage(trx, airSeg, bc);
  }
}

void
Diag980Collector::showOptions(const PricingTrx& trx)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  std::string depTime;
  std::string arrTime;
  std::vector<TravelSeg*>::const_iterator tI;
  std::vector<TravelSeg*>::const_iterator tE;
  std::vector<Itin*>::const_iterator iI = trx.itin().begin();
  std::vector<Itin*>::const_iterator iE = trx.itin().end();
  uint16_t option = 1;
  uint16_t fltSeq = 1;
  const AirSeg* airSeg = 0;
  const FareUsage* fu = 0;
  const FareUsage* prevFu = 0;
  uint16_t tvlSegIndex = 0;
  const DifferentialData* diff = 0;
  BookingCode bc;
  bool journeyExist = false;
  std::vector<Itin*> itins;
  std::vector<MoneyAmount> totalAmounts;

  for (; iI != iE; iI++)
  {
    const Itin& itin = *(*iI);
    if (!itinValidOption(itin))
      continue;
    itins.push_back(*iI);
    totalAmounts.push_back(amount(trx, *iI));
  }

  uint16_t numItins = itins.size();
  uint16_t iItin = 0;
  uint16_t jItin = 0;
  Itin* tmpItin = 0;
  MoneyAmount tmpAmount = 0.0;
  if (totalAmounts.size() != numItins)
  {
    dc << "ERROR IN CREATING DIAG 980 \n";
    return;
  }

  // sort the Options
  for (iItin = 0; iItin < numItins; iItin++)
  {
    for (jItin = iItin + 1; jItin < numItins; jItin++)
    {
      if (totalAmounts[jItin] < totalAmounts[iItin])
      {
        tmpAmount = totalAmounts[jItin];
        totalAmounts[jItin] = totalAmounts[iItin];
        totalAmounts[iItin] = tmpAmount;

        tmpItin = itins[jItin];
        itins[jItin] = itins[iItin];
        itins[iItin] = tmpItin;
      }
    }
  }

  // now display the options
  iI = itins.begin();
  iE = itins.end();
  for (; iI != iE; iI++, option++)
  {
    const Itin& itin = *(*iI);
    if (!itinValidOption(itin))
      continue;
    if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.billing()->actionCode() != "WPNI.C")
    {
      dc << "OPTION " << option << " ";
      printItinNum(itin);
      dc << " \n";
    }
    fltSeq = 1;
    prevFu = 0;
    tI = itin.travelSeg().begin();
    tE = itin.travelSeg().end();
    for (; tI != tE; tI++)
    {
      airSeg = dynamic_cast<AirSeg*>(*tI);
      if (airSeg == 0)
      {
        dc << "ARUNK \n";
        continue;
      }
      fu = getFareUsage(itin.farePath()[0], *tI, tvlSegIndex);
      if (prevFu == 0)
      {
        prevFu = fu;
      }
      else
      {
        if (fu != prevFu)
          ++fltSeq;
      }
      prevFu = fu;
      dc << fltSeq;
      if (fltSeq < 10)
        dc << "  ";
      else
        dc << " ";
      dc << airSeg->carrier();

      if (airSeg->localJourneyCarrier())
        dc << " L ";
      else if (airSeg->flowJourneyCarrier())
        dc << " F ";
      else
        dc << " - ";

      dc << setw(4) << airSeg->flightNumber() << " ";
      dc << airSeg->getBookingCode()[0] << " ";
      if (fu == 0)
      {
        dc << setw(2) << "-";
        dc << " ";
        dc << "- ";
      }
      else
      {
        diff = differentialData(fu, *tI);
        const PaxTypeFare::SegmentStatus& segStat = fu->segmentStatus()[tvlSegIndex];

        if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
            !(segStat._bkgCodeReBook.empty()))
        {
          bc = segStat._bkgCodeReBook;
        }
        else
        {
          bc = airSeg->getBookingCode();
        }

        dc << setw(2) << bc << " ";

        if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
          dc << "T ";
        else
          dc << "F ";
      }
      dc << setw(5) << airSeg->departureDT().dateToString(DDMMM, "") << " ";
      dc << setw(3) << airSeg->origAirport() << " ";
      dc << setw(3) << airSeg->destAirport() << " ";
      depTime = airSeg->departureDT().timeToString(HHMM_AMPM, "");
      arrTime = airSeg->arrivalDT().timeToString(HHMM_AMPM, "");
      // Cut off the M from the time
      depTime = depTime.substr(0, depTime.length() - 1);
      arrTime = arrTime.substr(0, arrTime.length() - 1);
      if (diff == 0)
      {
        dc << setw(5) << depTime << " ";
        dc << setw(5) << arrTime << " ";
      }
      else
      {
        const PaxTypeFare::SegmentStatus& diffSegStat = diffSegStatus(diff, *tI);
        if (diffSegStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
            !(diffSegStat._bkgCodeReBook.empty()))
        {
          bc = diffSegStat._bkgCodeReBook;
        }
        else
        {
          bc = airSeg->getBookingCode();
        }
        dc << "DIFF " << bc[0] << "-";
        if (diffSegStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
          dc << "T";
        else
          dc << "F";
        dc << setw(8) << diff->fareClassHigh();
      }
      // dc << setw(2) << airSeg->hiddenStops().size();
      if (fu != 0)
        printFareInfo(fu->paxTypeFare(), diff == 0);

      dc << " \n";
      marriage(trx, airSeg, bc);
    }

    amounts(trx, *iI);
    if (!itin.farePath().empty())
      printFarePath(itin.farePath()[0]);

    journeyExist = journeys(trx, itin);
    if (journeyExist)
      availability(trx, itin.farePath()[0]);
    dc << " \n";
  }
  return;
}

FareUsage*
Diag980Collector::getFareUsage(const FarePath* fpath, TravelSeg* tvlSeg, uint16_t& tvlSegIndex)
{
  std::vector<PricingUnit*>::const_iterator puI = fpath->pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puE = fpath->pricingUnit().end();

  std::vector<FareUsage*>::const_iterator fuI;
  std::vector<FareUsage*>::const_iterator fuE;

  std::vector<TravelSeg*>::const_iterator tvlI;
  std::vector<TravelSeg*>::const_iterator tvlE;

  uint16_t i = 0;
  for (; puI != puE; puI++)
  {
    PricingUnit& pu = **puI;
    fuI = pu.fareUsage().begin();
    fuE = pu.fareUsage().end();
    for (; fuI != fuE; fuI++)
    {
      FareUsage& fu = **fuI;
      tvlI = fu.travelSeg().begin();
      tvlE = fu.travelSeg().end();
      for (i = 0; tvlI != tvlE; tvlI++, i++)
      {
        if (tvlSeg == *tvlI)
        {
          tvlSegIndex = i;
          return *fuI;
        }
      }
    }
  }
  return 0;
}

bool
Diag980Collector::itinValidOption(const Itin& itin)
{
  if (itin.farePath().empty())
    return false;
  if (itin.errResponseCode() != ErrorResponseException::NO_ERROR)
    return false;
  return true;
}

void
Diag980Collector::aseAmounts(const PricingTrx& trx, const FarePath* fpath)
{
  if (!_active)
    return;

  if (trx.getTrxType() == PricingTrx::PRICING_TRX)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  CurrencyCode curr;
  const Itin* itin = fpath->itin();

  map<const Itin*, FareCalcCollector*>::const_iterator iter = trx.fareCalcCollectorMap().find(itin);

  if (iter == trx.fareCalcCollectorMap().end())
    return;

  const FareCalcCollector& calc = *iter->second;

  MoneyAmount totalEquivAmount = 0.0, totalOrigAmount = 0.0, totalTaxes = 0.0;
  MoneyAmount finalTotal = 0.0;

  const FareCalcCollector::CalcTotalsMap& ctMap = calc.calcTotalsMap();

  FareCalcCollector::CalcTotalsMap::const_iterator calcIt = ctMap.find(fpath);
  if (calcIt == ctMap.end())
    return;

  const CalcTotals& ct = *calcIt->second;

  totalOrigAmount = ct.convertedBaseFare;
  totalTaxes = ct.taxAmount();
  totalEquivAmount = ct.equivFareAmount;
  curr = ct.taxCurrencyCode();

  dc << "TOTAL FARE - ";
  dc << setw(3) << curr << "  ";

  if (curr == "USD")
    finalTotal = totalOrigAmount + totalTaxes;
  else
    finalTotal = totalEquivAmount + totalTaxes;

  Money grandTotal(finalTotal, curr);

  string grandTotalString;
  grandTotalString = grandTotal.toString();

  grandTotalString = grandTotalString.substr(3, grandTotalString.length() - 3);
  dc << setw(8) << grandTotalString;

  return;
}

void
Diag980Collector::amounts(const PricingTrx& trx, const Itin* itin)
{
  if (!_active)
    return;

  if (trx.getTrxType() == PricingTrx::PRICING_TRX)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  CurrencyCode curr;
  const FareCalcCollector* calc = 0;
  std::vector<FareCalcCollector*>::const_iterator fccI = trx.fareCalcCollector().begin();
  std::vector<FareCalcCollector*>::const_iterator fccE = trx.fareCalcCollector().end();
  for (; fccI != fccE; ++fccI)
  {
    if ((*fccI)->passengerCalcTotals().empty())
      continue;
    if (itin == (*fccI)->passengerCalcTotals()[0]->farePath->itin())
    {
      calc = *fccI;
      break;
    }
  }
  if (calc == 0)
    return;

  MoneyAmount totalEquivAmount = 0.0, totalOrigAmount = 0.0, totalTaxes = 0.0;
  MoneyAmount finalTotal = 0.0;
  int totalMileage = 0;

  const FareCalcCollector::CalcTotalsMap& ctMap = calc->calcTotalsMap();
  for (const auto& elem : ctMap)
  {
    const FarePath& fp = *elem.first;
    const CalcTotals& ct = *elem.second;

    totalOrigAmount += ct.convertedBaseFare;
    totalTaxes += ct.taxAmount();
    totalEquivAmount += ct.equivFareAmount;
    curr = ct.taxCurrencyCode();

    if (itin->farePath().size() > 1)
    {
      dc << "   " << fp.paxType()->number() << ct.truePaxType << " " << setw(8)
         << ct.equivFareAmount << "  " << setw(8) << ct.equivFareAmount << " \n";
    }

    if (trx.awardRequest())
      totalMileage += fp.paxType()->number() * getMileage(fp);
  }

  dc << "TOTAL FARE - ";

  dc << setw(3) << curr << "  ";
  if (curr == "USD")
    finalTotal = totalOrigAmount + totalTaxes;
  else
    finalTotal = totalEquivAmount + totalTaxes;

  Money grandTotal(finalTotal, curr);

  string grandTotalString;
  grandTotalString = grandTotal.toString();

  grandTotalString = grandTotalString.substr(3, grandTotalString.length() - 3);
  dc << setw(8) << grandTotalString << " \n";

  if (trx.awardRequest())
    dc << setw(8) << "TOTAL MILEAGE - " << totalMileage << " MIL\n";

  return;
}

MoneyAmount
Diag980Collector::amount(const PricingTrx& trx, const Itin* itin)
{
  const FareCalcCollector* calc = 0;
  MoneyAmount totalEquivAmount = 0.0, totalOrigAmount = 0.0, totalTaxes = 0.0;
  MoneyAmount totalAmount = 0.0;
  std::vector<FareCalcCollector*>::const_iterator fccI = trx.fareCalcCollector().begin();
  std::vector<FareCalcCollector*>::const_iterator fccE = trx.fareCalcCollector().end();
  for (; fccI != fccE; ++fccI)
  {
    if ((*fccI)->passengerCalcTotals().empty())
      continue;
    if (itin == (*fccI)->passengerCalcTotals()[0]->farePath->itin())
    {
      calc = *fccI;
      break;
    }
  }
  if (calc == 0)
    return totalAmount;

  const FareCalcCollector::CalcTotalsMap& ctMap = calc->calcTotalsMap();
  for (const auto& elem : ctMap)
  {
    const CalcTotals& ct = *elem.second;

    totalOrigAmount += ct.convertedBaseFare;
    totalTaxes += ct.taxAmount();
    totalEquivAmount += ct.equivFareAmount;
  }

  totalAmount = totalOrigAmount + totalTaxes;
  return totalAmount;
}

bool
Diag980Collector::journeys(const PricingTrx& trx, const Itin& itin)
{
  bool journeyExist = false;
  if (!_active)
    return journeyExist;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << "JOURNEYS: ";
  if (!trx.getOptions()->applyJourneyLogic())
    return journeyExist;

  if (trx.getTrxType() == PricingTrx::PRICING_TRX)
  {
    if (!trx.getOptions()->journeyActivatedForPricing())
      return journeyExist;
  }
  else if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    if (!trx.getOptions()->journeyActivatedForShopping())
      return journeyExist;
  }

  std::vector<FareMarket*> journeysForShoppingItin;
  if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    ItinUtil::journeys(
        const_cast<Itin&>(itin), const_cast<PricingTrx&>(trx), journeysForShoppingItin);
  }
  std::vector<FareMarket*>::const_iterator fmI = itin.flowFareMarket().begin();
  std::vector<FareMarket*>::const_iterator fmE = itin.flowFareMarket().end();

  for (; fmI != fmE; ++fmI)
  {
    FareMarket& fm = *(*fmI);
    if (!isValidJourney(trx, *fmI, journeysForShoppingItin))
      continue;
    dc << fm.boardMultiCity() << "-" << fm.offMultiCity() << " ";
    journeyExist = true;
  }
  dc << " \n";
  return journeyExist;
}

void
Diag980Collector::availability(const PricingTrx& trx, const FarePath* fpath)
{
  if (!_active)
    return;
  const Itin& itin = *fpath->itin();

  std::vector<FareMarket*> journeysForShoppingItin;
  if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    ItinUtil::journeys(
        const_cast<Itin&>(itin), const_cast<PricingTrx&>(trx), journeysForShoppingItin);
  }
  DiagCollector& dc = (DiagCollector&)*this;
  dc << "AVAILABILITY: \n";
  const AirSeg* airSeg = 0;
  uint16_t i = 0;
  uint16_t n = 0;
  const std::vector<ClassOfService*>* cosVec = 0;
  const FareUsage* fu = 0;
  uint16_t tvlSegIndex = 0;
  uint16_t numSeatsRequired = 0;
  std::vector<TravelSeg*>::const_iterator tI = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tE = itin.travelSeg().end();
  for (; tI != tE; tI++)
  {
    if (!partOfJourney(*tI, itin, trx, journeysForShoppingItin))
      continue;
    airSeg = dynamic_cast<AirSeg*>(*tI);
    if (airSeg == 0)
    {
      dc << "  ARUNK \n";
      continue;
    }
    dc << airSeg->carrier() << " ";
    dc << setw(4) << airSeg->flightNumber();
    dc << airSeg->getBookingCode()[0] << " ";
    dc << "LOCAL:";
    n = airSeg->classOfService().size();
    for (i = 0; i < n; i++)
    {
      const ClassOfService& cs = *(airSeg->classOfService()[i]);
      if (cs.numSeats() < 1)
        continue;
      dc << cs.bookingCode()[0];
    }

    if (!(trx.getTrxType() == PricingTrx::PRICING_TRX || trx.getTrxType() == PricingTrx::MIP_TRX))
    {
      dc << " \n";
      continue;
    }
    if (trx.getTrxType() == PricingTrx::PRICING_TRX)
    {
      dc << " \n";
      continue;
    }
    dc << " \n";
    dc << "         FLOW :";
    cosVec = 0;
    if ((airSeg->localJourneyCarrier() || airSeg->flowJourneyCarrier()))
      cosVec = &ShoppingUtil::getMaxThruClassOfServiceForSeg(trx, *tI);

    fu = getFareUsage(fpath, *tI, tvlSegIndex);
    numSeatsRequired = 0;
    if (fu != 0)
    {
      numSeatsRequired =
          PaxTypeUtil::numSeatsForFare(const_cast<PricingTrx&>(trx), *(fu->paxTypeFare()));
    }
    if (cosVec != 0)
    {
      n = cosVec->size();
      for (i = 0; i < n; i++)
      {
        const ClassOfService& cs = *(*cosVec)[i];
        if (cs.numSeats() < 1)
          continue;
        if (cs.numSeats() < numSeatsRequired)
          continue;
        dc << cs.bookingCode()[0];
      }
    }
    dc << " \n";
  }
  thruAvailability(trx, fpath);
  return;
}

void
Diag980Collector::thruAvailability(const PricingTrx& trx, const FarePath* fpath)
{
  if (!_active)
    return;

  const Itin& itin = *fpath->itin();

  std::vector<FareMarket*> journeysForShoppingItin;
  if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    ItinUtil::journeys(
        const_cast<Itin&>(itin), const_cast<PricingTrx&>(trx), journeysForShoppingItin);
  }

  DiagCollector& dc = (DiagCollector&)*this;
  const AirSeg* airSeg = 0;
  uint16_t i = 0;
  uint16_t n = 0;
  const std::vector<ClassOfService*>* cosVec = 0;
  const FareUsage* fu = 0;
  uint16_t tvlSegIndex = 0;
  uint16_t numSeatsRequired = 0;
  uint16_t iCos = 0;
  std::vector<FareMarket*> thruFms;
  std::vector<TravelSeg*>::const_iterator tI = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tE = itin.travelSeg().end();

  JourneyUtil::getThruFMs(&itin, fpath, thruFms);

  if (thruFms.empty())
    return;

  dc << " \n";
  dc << "FLOW AVAILABILITY BY MARKETS: \n";

  std::vector<FareMarket*>::iterator fmI = thruFms.begin();
  std::vector<FareMarket*>::iterator fmE = thruFms.end();
  for (; fmI != fmE; fmI++)
  {
    FareMarket& fm = *(*fmI);
    tI = fm.travelSeg().begin();
    tE = fm.travelSeg().end();
    iCos = 0;

    airSeg = dynamic_cast<AirSeg*>(*tI);
    if (airSeg == 0)
      continue;
    if (!(airSeg->localJourneyCarrier() || airSeg->flowJourneyCarrier()))
      continue;

    dc << "MARKET: " << fm.boardMultiCity() << "-" << fm.offMultiCity();

    if (airSeg->localJourneyCarrier())
    {
      if (trx.getTrxType() == PricingTrx::PRICING_TRX)
      {
        if (!fm.flowMarket())
          dc << " -REDEFINED JOURNEY";
      }
      else if (trx.getTrxType() == PricingTrx::MIP_TRX)
      {
        if (!isValidJourney(trx, &fm, journeysForShoppingItin))
          dc << " -REDEFINED JOURNEY";
      }
    }

    dc << " \n";
    for (; tI != tE; tI++, iCos++)
    {
      airSeg = dynamic_cast<AirSeg*>(*tI);
      if (airSeg == 0)
      {
        dc << "  ARUNK \n";
        continue;
      }
      dc << airSeg->carrier() << " ";
      dc << setw(4) << airSeg->flightNumber();
      dc << airSeg->getBookingCode()[0] << "      :";
      cosVec = fm.classOfServiceVec()[iCos];

      // Do not show out-of-sequence marriage segments
      OAndDMarket* od = JourneyUtil::getOAndDMarketFromFM(itin, &fm);
      if (od && !od->hasTravelSeg(*tI))
        cosVec = 0;

      fu = getFareUsage(fpath, *tI, tvlSegIndex);
      numSeatsRequired = 0;
      if (fu != 0)
      {
        numSeatsRequired =
            PaxTypeUtil::numSeatsForFare(const_cast<PricingTrx&>(trx), *(fu->paxTypeFare()));
      }
      if (cosVec != 0)
      {
        n = cosVec->size();
        for (i = 0; i < n; i++)
        {
          const ClassOfService& cs = *(*cosVec)[i];
          if (cs.numSeats() < 1)
            continue;

          if (cs.numSeats() < numSeatsRequired)
            continue;
          dc << cs.bookingCode()[0];
        }
      }
      dc << " \n";
    }
    dc << " \n";
  }
  return;
}

bool
Diag980Collector::partOfJourney(const TravelSeg* tvlSeg,
                                const Itin& itin,
                                const PricingTrx& trx,
                                std::vector<FareMarket*>& journeysForShoppingItin)
{
  std::vector<FareMarket*>::const_iterator fmI = itin.flowFareMarket().begin();
  std::vector<FareMarket*>::const_iterator fmE = itin.flowFareMarket().end();

  std::vector<TravelSeg*>::const_iterator tvlI;
  std::vector<TravelSeg*>::const_iterator tvlE;

  for (; fmI != fmE; ++fmI)
  {
    const FareMarket& fm = *(*fmI);

    if (!isValidJourney(trx, *fmI, journeysForShoppingItin))
      continue;
    tvlI = fm.travelSeg().begin();
    tvlE = fm.travelSeg().end();
    for (; tvlI != tvlE; tvlI++)
    {
      if (tvlSeg == *tvlI)
        return true;
    }
  }
  return false;
}

bool
Diag980Collector::isValidJourney(const PricingTrx& trx,
                                 const FareMarket* fm,
                                 std::vector<FareMarket*>& journeysForShoppingItin)
{
  if (!fm->flowMarket())
    return false;

  if (trx.getTrxType() == PricingTrx::PRICING_TRX)
    return true;

  if (trx.getTrxType() != PricingTrx::MIP_TRX)
    return false;

  std::vector<FareMarket*>::const_iterator fmIShopping = journeysForShoppingItin.begin();
  const std::vector<FareMarket*>::const_iterator fmEShopping = journeysForShoppingItin.end();

  for (; fmIShopping != fmEShopping; fmIShopping++)
  {
    if ((*fmIShopping) == fm)
      return true;
  }
  return false;
}

void
Diag980Collector::marriage(const PricingTrx& trx, const AirSeg* airSeg, const BookingCode& bc)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;

  std::map<std::string, std::string>::const_iterator endI = trx.diagnostic().diagParamMap().end();
  std::map<std::string, std::string>::const_iterator beginI =
      trx.diagnostic().diagParamMap().find(Diagnostic::DISPLAY_DETAIL);

  if (beginI == endI)
    return;

  if ((*beginI).second != "MARRIAGE")
    return;

  const std::vector<MarriedCabin*>& marriedCabin =
      trx.dataHandle().getMarriedCabins(airSeg->carrier(), bc, airSeg->departureDT());
  dc << "   MARRY: " << marriedCabin.size();

  if (marriedCabin.size() == 0)
  {
    dc << " \n";
    return;
  }

  std::vector<MarriedCabin*>::const_iterator mBeginI = marriedCabin.begin();
  std::vector<MarriedCabin*>::const_iterator mBeginE = marriedCabin.end();
  size_t marrySize = 0;
  for (; mBeginI != mBeginE; mBeginI++, marrySize++)
  {
    const MarriedCabin& marriedCab = *(*mBeginI);
    dc << " ";
    dc << marriedCab.premiumCabin() << "/" << marriedCab.marriedCabin();

    if (marriedCab.loc1().loc().empty() && marriedCab.loc2().loc().empty())
    {
      if (marrySize + 1 == marriedCabin.size())
        dc << ".";
      else
        dc << ",";
      continue;
    }

    dc << marriedCab.loc1().locType() << "/" << marriedCab.loc1().loc() << "-"
       << marriedCab.loc2().locType() << "/" << marriedCab.loc2().loc() << " "
       << marriedCab.directionality();
    if (marrySize + 1 == marriedCabin.size())
      dc << ".";
    else
      dc << ",";
  }
  dc << " \n";
  return;
}

DifferentialData*
Diag980Collector::differentialData(const FareUsage* fu, const TravelSeg* tvlSeg)
{
  if (fu->differentialPlusUp().empty())
    return 0;

  bool diffrentialFound = false;
  std::vector<TravelSeg*>::const_iterator i, iEnd;
  std::vector<PaxTypeFare::SegmentStatus>::const_iterator iter, iterEnd;

  std::vector<DifferentialData*>::const_iterator diffI = fu->differentialPlusUp().begin();
  std::vector<DifferentialData*>::const_iterator diffIEnd = fu->differentialPlusUp().end();

  DifferentialData* diff = 0;
  for (; diffI != diffIEnd; ++diffI)
  {
    diff = *diffI;
    if (diff == 0)
      continue;

    DifferentialData::STATUS_TYPE aStatus = diff->status();

    if (!(aStatus == DifferentialData::SC_PASSED ||
          aStatus == DifferentialData::SC_CONSOLIDATED_PASS))
    {
      diff = 0;
      continue;
    }

    // std::pair<const TravelSeg*, std::string>(*i, diff->fareClassHigh()));

    for (i = diff->travelSeg().begin(),
        iEnd = diff->travelSeg().end(),
        iter = diff->fareHigh()->segmentStatus().begin(),
        iterEnd = diff->fareHigh()->segmentStatus().end();
         i != iEnd && iter != iterEnd;
         ++i, ++iter)
    {
      if ((*i) != tvlSeg)
        continue;
      diffrentialFound = true;
      break;
    }
    if (diffrentialFound)
      break;
  }
  if (!diffrentialFound)
    diff = 0;
  return diff;
}

const PaxTypeFare::SegmentStatus&
Diag980Collector::diffSegStatus(const DifferentialData* diff, const TravelSeg* tvlSeg)
{
  std::vector<TravelSeg*>::const_iterator i = diff->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator iEnd = diff->travelSeg().end();

  std::vector<PaxTypeFare::SegmentStatus>::const_iterator iter =
      diff->fareHigh()->segmentStatus().begin();
  std::vector<PaxTypeFare::SegmentStatus>::const_iterator iterEnd =
      diff->fareHigh()->segmentStatus().end();

  for (; i != iEnd && iter != iterEnd; ++i, ++iter)
  {
    if ((*i) != tvlSeg)
      continue;
    return *iter;
  }

  return *(diff->fareHigh()->segmentStatus().begin());
}

int
Diag980Collector::getMileage(const FarePath& farePath) const
{
  int result = 0;
  std::vector<PricingUnit*>::const_iterator puIt = farePath.pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puItEnd = farePath.pricingUnit().end();
  for (; puIt != puItEnd; ++puIt)
  {
    std::vector<FareUsage*>::const_iterator fuIt = (*puIt)->fareUsage().begin();
    const std::vector<FareUsage*>::const_iterator fuItEnd = (*puIt)->fareUsage().end();
    for (; fuIt != fuItEnd; ++fuIt)
    {
      result += (*fuIt)->paxTypeFare()->mileage();
    }
  }

  return result;
}

void
Diag980Collector::printItinNum(const Itin& itin)
{
  IntIndex num = itin.itinNum();
  bool isValid = (num != -1);
  if (isValid)
  {
    *this << "ITIN " << num;
  }
}

void
Diag980Collector::printFareInfo(const PaxTypeFare* const fare, const bool hasNoDifferentials)
{
  if (!_active || !fare)
    return;

  DiagCollector& dc = *this;
  // Print fare class only by default
  dc << " " << setw(hasNoDifferentials ? 14 : 10) << fare->fareClass();

  if (!_showFareDetails)
    return;

  // Display a few details that - together with the information given by
  // printFarePath() - will help to identify the fare in the database.
  dc << " " << setw(2) << cnvFlags(*fare);
  // Note: footnote1 oftenly differentiates fares on market (while footnote2 is almost no used, so it is not printed)
  dc << " " << setw(2) << fare->footNote1();

}

void
Diag980Collector::printFarePath(const FarePath* const fpath)
{
  if (!_active || !_showFareDetails || !fpath)
    return;

  DiagCollector& dc = *this;
  dc << "FAREPATH:\n" << *fpath;
}
}
