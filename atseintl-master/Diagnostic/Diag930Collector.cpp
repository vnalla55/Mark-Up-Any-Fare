//----------------------------------------------------------------------------
//  Copyright Sabre 2005
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
#include "Diagnostic/Diag930Collector.h"

#include "Common/Assert.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/Money.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/Itin.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/GroupFarePath.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

#include <algorithm>
#include <iomanip>
#include <iosfwd>
#include <numeric>

using namespace std;

namespace tse
{
namespace
{
ConfigurableValue<int>
port("SERVER_SOCKET_ADP", "PORT");
}
Diag930Collector&
Diag930Collector::
operator<<(const ShoppingTrx& trx)
{
  if (!_active)
    return (*this);

  _shoppingTrx = &trx;
  DiagCollector& dc(*this);

  dc << "HOSTNAME/PORT:  ";
  std::string hostname = getenv("HOSTNAME");

  for (auto& elem : hostname)
    elem = std::toupper(elem);
  dc << hostname << "/" << port.getValue() << "\n";

  dc << std::endl;
  printAltDatesStatistics(dc);
  dc << std::endl;

  dc << "CABIN DEFINITION" << std::endl;
  CabinType cabinType;
  cabinType.setClass('1');
  dc << "1 - " << cabinType.printName() << std::endl;
  cabinType.setClass('2');
  dc << "2 - " << cabinType.printName() << std::endl;
  cabinType.setClass('4');
  dc << "4 - " << cabinType.printName() << std::endl;
  cabinType.setClass('5');
  dc << "5 - " << cabinType.printName() << std::endl;
  cabinType.setClass('7');
  dc << "7 - " << cabinType.printName() << std::endl;
  cabinType.setClass('8');
  dc << "8 - " << cabinType.printName() << std::endl;

  dc << trx.flightMatrix();

  return (*this);
}

namespace
{
typedef std::pair<SopIdVec, GroupFarePath*> Solution;
bool
comparePaths(const Solution& a, const Solution& b)
{
  if (a.second == nullptr && b.second == nullptr)
  {
    const int sum1 = accumulate(a.first.begin(), a.first.end(), 0);
    const int sum2 = accumulate(b.first.begin(), b.first.end(), 0);
    return sum1 < sum2;
  }

  if (b.second == nullptr)
  {
    return false;
  }

  if (a.second == nullptr)
  {
    return true;
  }

  if (a.second->getTotalNUCAmount() == b.second->getTotalNUCAmount())
  {
    const int sum1 = accumulate(a.first.begin(), a.first.end(), 0);
    const int sum2 = accumulate(b.first.begin(), b.first.end(), 0);
    return sum1 < sum2;
  }

  return a.second->getTotalNUCAmount() < b.second->getTotalNUCAmount();
}

int
getMileage(const GroupFarePath& groupFarePath)
{
  int mileage = 0;
  std::vector<FPPQItem*>::const_iterator x = groupFarePath.groupFPPQItem().begin();
  const std::vector<FPPQItem*>::const_iterator xEnd = groupFarePath.groupFPPQItem().end();

  for (; x != xEnd; ++x)
  {
    std::vector<PricingUnit*>::const_iterator y = (*x)->farePath()->pricingUnit().begin();
    const std::vector<PricingUnit*>::const_iterator yEnd = (*x)->farePath()->pricingUnit().end();

    for (; y != yEnd; ++y)
    {
      std::vector<FareUsage*>::const_iterator z = (*y)->fareUsage().begin();
      const std::vector<FareUsage*>::const_iterator zEnd = (*y)->fareUsage().end();

      for (; z != zEnd; ++z)
      {
        mileage += (*z)->paxTypeFare()->mileage();
      }
    }
  }

  return mileage;
}

bool
comparePathsByMileage(const Solution& a, const Solution& b)
{
  if (a.second == nullptr && b.second == nullptr)
  {
    const int sum1 = accumulate(a.first.begin(), a.first.end(), 0);
    const int sum2 = accumulate(b.first.begin(), b.first.end(), 0);
    return sum1 < sum2;
  }

  if (b.second == nullptr)
  {
    return false;
  }

  if (a.second == nullptr)
  {
    return true;
  }

  if (a.second->getTotalNUCAmount() == b.second->getTotalNUCAmount())
  {
    const int mileage1 = getMileage(*a.second);
    const int mileage2 = getMileage(*b.second);

    if (mileage1 == mileage2)
    {
      const int sum1 = accumulate(a.first.begin(), a.first.end(), 0);
      const int sum2 = accumulate(b.first.begin(), b.first.end(), 0);
      return sum1 < sum2;
    }

    return mileage1 < mileage2;
  }

  return a.second->getTotalNUCAmount() < b.second->getTotalNUCAmount();
}

} // namespace

Diag930Collector&
Diag930Collector::
operator<<(const ShoppingTrx::FlightMatrix& matrix)
{
  if (!_active)
    return (*this);

  DiagCollector& dc(*this);

  if (matrix.empty())
  {
    dc << "ATSEI - NO OPTIONS FOUND"
       << "\n"
       << "\n";
    return (*this);
  }

  std::vector<GroupFarePath*> paths;
  std::vector<const SopIdVec*> matrixCells;
  std::vector<Solution> myPaths(matrix.begin(), matrix.end());

  if (_shoppingTrx->awardRequest())
    std::sort(myPaths.begin(), myPaths.end(), comparePathsByMileage);
  else
    std::sort(myPaths.begin(), myPaths.end(), comparePaths);

  ShoppingUtil::preparePathsFromFlightMatrix(matrix, paths, &matrixCells);

  if (paths.empty())
  {
    displayFlightOnly(matrix);
    return (*this);
  }

  matrixCells.clear();
  paths.clear();
  std::vector<Solution>::iterator mbIter = myPaths.begin();
  std::vector<Solution>::iterator meIter = myPaths.end();
  size_t index = 1;

  for (; mbIter != meIter; ++mbIter, ++index)
  {
    SopIdVec* fmvPtr = &mbIter->first;
    const GroupFarePath& path = *mbIter->second;
    dc << "\n"
       << "\n";
    dc << "OPTION ";
    setf(std::ios::right, std::ios::adjustfield);
    dc << std::setw(3) << index;
    if (_shoppingTrx->getNumOfCustomSolutions() &&
        ShoppingUtil::isCustomSolution(*_shoppingTrx, *fmvPtr))
      dc << " [CUS]";
    dc << "\n";
    dc << "SOURCE: " << (mbIter->second ? path.sourcePqName() : "NULL")
       << "\n"; // It is possible to have null path

    if (fmvPtr->empty())
    {
      dc << "NO FARE PATH TO DISPLAY"
         << "\n";
    }
    else if ((mbIter->second == nullptr) || (path.groupFPPQItem().empty()))
    {
      if (_rootDiag->diagParamMapItem("DD") == "FAMILIES")
      {
        if (_shoppingTrx->estimateMatrix().begin() != _shoppingTrx->estimateMatrix().end())
        {
          dc << "FAMILY \n";
        }

        outputFlightData(*fmvPtr);
        dc << "--------------------------------------------------------------------------\n";
        int childCount = 0;

        for (const auto& elem : _shoppingTrx->estimateMatrix())
        {
          if (*fmvPtr == elem.second.first)
          {
            if (_shoppingTrx->getNumOfCustomSolutions() &&
                ShoppingUtil::isCustomSolution(*_shoppingTrx, *fmvPtr))
              dc << "[CUS]\n";
            childCount++;
            outputFlightData(elem.first);
            dc << "--------------------------------------------------------------------------\n";
          }
        }

        dc << "TOTAL NUMBER OF CHILDREN: " << childCount << "\n";
      }
      else
      {
        outputFlightData(*fmvPtr);
      }
    }
    else
    {
      if (_rootDiag->diagParamMapItem("DD") == "FAMILIES")
      {
        if (_shoppingTrx->estimateMatrix().begin() != _shoppingTrx->estimateMatrix().end())
        {
          dc << "FAMILY \n";
        }

        outputGroupFarePathData(fmvPtr, path);
        dc << "--------------------------------------------------------------------------\n";
        int childCount = 0;

        for (const auto& elem : _shoppingTrx->estimateMatrix())
        {
          if (*fmvPtr == elem.second.first)
          {
            if (_shoppingTrx->getNumOfCustomSolutions() &&
                ShoppingUtil::isCustomSolution(*_shoppingTrx, elem.first))
              dc << "[CUS]\n";
            childCount++;
            outputFlightData(elem.first);
            dc << "--------------------------------------------------------------------------\n";
          }
        }

        dc << "TOTAL NUMBER OF CHILDREN: " << childCount << "\n";
      }
      else
      {
        outputGroupFarePathData(fmvPtr, path);
      }
    }

    dc << "\n";
  }

  return (*this);
}

void
Diag930Collector::outputGroupFarePathData(const SopIdVec* fmv, const GroupFarePath& gpath)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);

  bool firstTime = true;
  std::string USDCur("USD");
  std::string USDCurStr(" USD");
  double totalAmountNUC = 0.0;
  int totalMileage = 0;
  MoneyAmount grandTotalTax = 0;

  for (const auto elem : gpath.groupFPPQItem())
  {
    const FarePath& fPath = *elem->farePath();

    if (firstTime)
    {
      // We only want to output the path once
      outputFarePathData(fmv, fPath);
      firstTime = false;
    }

    dc.setf(std::ios::right, std::ios::adjustfield);
    const PaxType* paxType = fPath.paxType();
    const uint16_t& paxNumber = paxType->number();
    dc << "        " << std::setw(5) << paxNumber << paxType->paxType();
    double singlePaxFareAmount = 0.0;
    int singleMileage = 0;

    for (const auto elem : fPath.pricingUnit())
    {
      for (std::vector<FareUsage*>::const_iterator y = elem->fareUsage().begin();
           y != elem->fareUsage().end();
           ++y)
      {
        const PaxTypeFare& fare = *(**y).paxTypeFare();
        singlePaxFareAmount += fare.totalFareAmount();
        singleMileage += fare.mileage();
      }
    }

    // Remove USD from end of the fare amount
    double fullPaxFareAmount = singlePaxFareAmount * static_cast<double>(paxNumber);
    totalMileage += singleMileage * static_cast<int>(paxNumber);
    totalAmountNUC += fPath.getTotalNUCAmount() * static_cast<double>(paxNumber);
    Money m(singlePaxFareAmount, USDCur);
    std::string monStr = ShoppingUtil::stripMoneyStr(m, USDCurStr);
    // Remove USD from end of the fare amount when number of passengers
    // are taken into account
    Money m2(fullPaxFareAmount, USDCur);
    std::string monTotStr = ShoppingUtil::stripMoneyStr(m2, USDCurStr);
    dc << "  " << std::setw(8) << monStr << " " << std::setw(8) << monTotStr;
    grandTotalTax += outputTaxData(fPath);
  }

  Money totalEstMon(totalAmountNUC, USDCur);
  std::string totalEstFare = ShoppingUtil::stripMoneyStr(totalEstMon, USDCurStr);
  dc << "TOTAL ESTIMATED FARE - USD " << std::setw(8) << totalEstFare;
  dc << "   PLUS TAX  " << std::setw(8) << gpath.getTotalNUCAmount() + grandTotalTax;

  if (_shoppingTrx->awardRequest())
  {
    dc << " " << std::setw(8) << totalMileage << " MIL\n";
  }

  dc << "\n";
}

MoneyAmount
Diag930Collector::outputTaxData(const FarePath& path)
{
  if (!_active)
    return 0;

  DiagCollector& dc(*this);

  MoneyAmount totalTax = 0;
  const Itin* itin = path.itin();

  if (itin == nullptr)
  {
    dc << "\n";
    return 0;
  }

  const TaxResponse* taxres = TaxResponse::findFor(&path);

  if ((taxres == nullptr) || (taxres->taxItemVector().empty()))
  {
    dc << "\n";
    return 0;
  }

  TaxResponse::TaxItemVector::const_iterator taxItemI = taxres->taxItemVector().begin();
  CurrencyCode curCode;

  for (; taxItemI != taxres->taxItemVector().end(); taxItemI++)
  {
    if ((*taxItemI)->taxAmount() != 0)
    {
      curCode = (*taxItemI)->paymentCurrency();
      totalTax += (*taxItemI)->taxAmount();
    }
  }

  Money nuc(NUC);
  CurrencyConversionFacade ccFacade;

  if ((curCode != "NUC") && (curCode != "") && (totalTax != 0))
  {
    try
    {
      Money taxCurrency(totalTax, curCode);
      ShoppingTrx& nonConstTrx = const_cast<ShoppingTrx&>(*_shoppingTrx);

      if (ccFacade.convert(nuc, taxCurrency, nonConstTrx))
        totalTax = nuc.value();
    }
    catch (...)
    {
      return 0;
    }
  }

  dc << "   PLUS TAX  " << std::setw(8) << path.getTotalNUCAmount() + totalTax << "\n";
  return totalTax;
}

void
Diag930Collector::outputFarePathData(const SopIdVec* fmv, const FarePath& path)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);

  if (fmv->empty())
  {
    dc << "NO FARE PATH TO DISPLAY"
       << "\n";
    return;
  }

  if (_shoppingTrx->isSumOfLocalsProcessingEnabled())
  {
    setComponentValidationForCarrier(*fmv, path);
  }

  size_t puIndex = 1;
  SopIdVec::const_iterator fmvIter = fmv->begin();
  SopIdVec::const_iterator fmvEIter = fmv->end();
  const std::vector<ShoppingTrx::Leg>& legs = _shoppingTrx->legs();

  for (uint32_t legIdx = 0; fmvIter != fmvEIter; ++fmvIter, ++puIndex, ++legIdx)
  {
    const ShoppingTrx::Leg& curLeg = legs[legIdx];
    const int& curMatVal = *fmvIter;
    const ShoppingTrx::SchedulingOption& curSop = curLeg.sop()[curMatVal];
    const Itin* curItin = curSop.itin();
    std::vector<TravelSeg*>::const_iterator tSegIter = curItin->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator tSegEIter = curItin->travelSeg().end();

    if (curSop.isLngCnxSop())
      dc << "--THIS IS LONG CONNECTION TIME OPTION--\n";

    for (; tSegIter != tSegEIter; ++tSegIter)
    {
      if ((*tSegIter)->segmentType() == Arunk)
      {
        continue;
      }

      const AirSeg* aSegPtr = dynamic_cast<const AirSeg*>(*tSegIter);

      if (nullptr == aSegPtr)
      {
        continue;
      }

      const AirSeg& aSeg = *aSegPtr;
      dc << std::setw(2) << puIndex << " " << std::setw(3) << aSeg.carrier();
      dc << " ";
      dc << std::setw(4) << aSeg.flightNumber() << " ";
      const DateTime& depDT = aSeg.departureDT();
      const DateTime& arrDT = aSeg.arrivalDT();
      DayOfWeek depDOW = depDT.dayOfWeek();
      ostringstream dowst;
      dowst << depDOW;
      std::string dowStr = dowst.str();
      // dowStr = dowStr.substr(0, 1);
      std::string dowTransStr;
      char dowCh0 = dowStr[0];
      char dowCh1 = dowStr[1];

      switch (toupper(dowCh0))
      {
      case 'T':
        if (toupper(dowCh1) == 'H')
        {
          dowTransStr = "Q";
        }
        else
        {
          dowTransStr = "T";
        }

        break;

      case 'S':
        if (toupper(dowCh1) == 'A')
        {
          dowTransStr = "J";
        }
        else
        {
          dowTransStr = "S";
        }

        break;

      default:
        dowTransStr = dowCh0;
        break;
      }

      std::string depDTStr = depDT.timeToString(HHMM_AMPM, "");
      std::string arrDTStr = arrDT.timeToString(HHMM_AMPM, "");
      // Cut off the M from the time
      depDTStr = depDTStr.substr(0, depDTStr.length() - 1);
      arrDTStr = arrDTStr.substr(0, arrDTStr.length() - 1);
      BookingCode bookingCode = " ";
      CabinType bookedCabin;
      CabinType bookedCabinSentBack;
      getBookingCodeCabin(
          path, *tSegIter, bookingCode, bookedCabin, fmv, curMatVal, bookedCabinSentBack);
      dc << std::setw(2) << bookingCode << " " << bookedCabin.getCabinIndicator() << " "
         << bookedCabinSentBack.getCabinIndicator() << " ";
      dc << std::setw(5) << depDT.dateToString(DDMMM, "") << " " << setw(2) << dowTransStr << " ";
      dc << std::setw(3) << aSeg.origin()->loc() << "  " << std::setw(3)
         << aSeg.destination()->loc() << " ";
      dc << std::setw(5) << depDTStr << " ";
      dc << std::setw(5) << arrDTStr << " ";
      dc << std::setw(4) << aSeg.equipmentType() << " ";
      dc << std::setw(2) << aSeg.hiddenStops().size() << " ";

      if (aSeg.eticket())
      {
        dc << setw(3) << "/E";
      }

      dc << "\n";
    }
  }
}

void
Diag930Collector::setComponentValidationForCarrier(const SopIdVec& fmv, const FarePath& path)
{
  shpq::CxrKeyPerLeg cxrKeyPerLeg;
  uint64_t duration = 0;
  ShoppingUtil::collectSopsCxrKeys(*_shoppingTrx, fmv, cxrKeyPerLeg);

  if (_shoppingTrx->isAltDates())
    duration = ShoppingAltDateUtil::getDuration(*_shoppingTrx, fmv);

  for (const PricingUnit* pu : path.pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      const uint16_t fuLegIndex = fu->paxTypeFare()->fareMarket()->legIndex();

      fu->paxTypeFare()->setComponentValidationForCarrier(
          cxrKeyPerLeg[fuLegIndex], _shoppingTrx->isAltDates(), duration);
    }
  }
}

void
Diag930Collector::getBookingCodeCabin(const FarePath& fpath,
                                      TravelSeg* tvlSeg,
                                      BookingCode& bookingCode,
                                      CabinType& bookedCabin,
                                      const SopIdVec* fmv,
                                      int sopId,
                                      CabinType& bookedCabinSentBack)
{
  std::vector<PricingUnit*>::const_iterator puI = fpath.pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puE = fpath.pricingUnit().end();
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
      // find if fare usage is across stopover
      const FareMarket* fuFareMarket = fu.paxTypeFare()->fareMarket();
      const std::vector<TravelSeg*>& travelSegs = fuFareMarket->travelSeg();
      TSE_ASSERT(travelSegs.empty() == false);
      uint16_t legIndex = fuFareMarket->legIndex();
      // uint16_t legIndex = ShoppingUtil::getLegTravelSeg(*_shoppingTrx, travelSegs);
      const ShoppingTrx::Leg& leg = _shoppingTrx->legs()[legIndex];
      const bool acrossStopOver = leg.stopOverLegFlag();
      tvlI = fu.travelSeg().begin();
      tvlE = fu.travelSeg().end();

      for (i = 0; tvlI != tvlE; tvlI++, i++)
      {
        if (tvlSeg == *tvlI)
        {
          // get to the right bitmap
          uint32_t bitmapNumber;

          if (acrossStopOver)
          {
            // build the vector of SOPs for this across stop over leg
            const IndexVector& jumped = _shoppingTrx->legs()[legIndex].jumpedLegIndices();
            SopIdVec legSops;

            for (const auto elem : jumped)
            {
              const size_t index = std::size_t(elem);

              if (index == ASOLEG_SURFACE_SECTOR_ID)
              {
                continue;
              }

              TSE_ASSERT(index < fmv->size());
              legSops.push_back((*fmv)[index]);
            }

            ShoppingTrx& nonConstTrx = const_cast<ShoppingTrx&>(*_shoppingTrx);
            // find the bitmap number for this set of SOPs
            bitmapNumber = ShoppingUtil::getBitmapForASOLeg(
                nonConstTrx, legIndex, fuFareMarket->governingCarrier(), legSops);
          }
          else
          {
            ShoppingUtil::ExternalSopId extId = ShoppingUtil::createExternalSopId(legIndex, sopId);
            // get bitmap number
            bitmapNumber = ShoppingUtil::getFlightBitIndex(*_shoppingTrx, extId);
          }

          TSE_ASSERT(bitmapNumber < fu.paxTypeFare()->flightBitmap().size());
          const PaxTypeFare::FlightBit& bitMap = fu.paxTypeFare()->flightBitmap()[bitmapNumber];
          const PaxTypeFare::SegmentStatus* segStat = nullptr;

          if (!bitMap._segmentStatus.empty())
          {
            segStat = &bitMap._segmentStatus[i];
          }

          getBookingCodeCabin(*tvlSeg, segStat, bookingCode, bookedCabin, bookedCabinSentBack);

          return;
        }
      }
    }
  }

  return;
}

void
Diag930Collector::getBookingCodeCabin(const TravelSeg& tvlSeg,
                                      const PaxTypeFare::SegmentStatus* segStatus,
                                      BookingCode& bookingCode,
                                      CabinType& bookedCabin,
                                      CabinType& bookedCabinSentBack)
{
  bool isReBooked = segStatus && !segStatus->_bkgCodeReBook.empty() &&
                    segStatus->_bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED);

  if (!isReBooked)
  {
    bookingCode = tvlSeg.getBookingCode();
    bookedCabin = tvlSeg.bookedCabin();
    bookedCabinSentBack = bookedCabin;
  }
  // IS doesn't allow for rebooking lower cabin
  else if (tvlSeg.bookedCabin() <= segStatus->_reBookCabin)
  {
    bookingCode = segStatus->_bkgCodeReBook;
    bookedCabin = segStatus->_reBookCabin;
    bookedCabinSentBack = bookedCabin;
  }
  else
  {
    std::vector<ClassOfService*>::const_iterator cosIt = tvlSeg.classOfService().begin();
    std::vector<ClassOfService*>::const_iterator cosEnd = tvlSeg.classOfService().end();

    for (; cosIt != cosEnd; ++cosIt)
    {
      const ClassOfService* cos = *cosIt;
      if (tvlSeg.bookedCabin() == cos->cabin())
      {
        bookingCode = cos->bookingCode();
        break;
      }
    }

    if (cosIt == cosEnd)
    {
      bookingCode = "Y";
    }

    bookedCabin = tvlSeg.bookedCabin();
    bookedCabinSentBack = bookedCabin;
  }
}

void
Diag930Collector::displayFlightOnly(const ShoppingTrx::FlightMatrix& matrix)
{
  if (!_active)
    return;

  DiagCollector& dc(*this);
  ShoppingTrx::FlightMatrix::const_iterator matrixIter = matrix.begin();
  ShoppingTrx::FlightMatrix::const_iterator matrixEIter = matrix.end();
  size_t index = 1;

  for (; matrixIter != matrixEIter; ++matrixIter, ++index)
  {
    dc << "\n\n";
    dc << "OPTION ";
    setf(std::ios::right, std::ios::adjustfield);
    dc << std::setw(3) << index;
    if (_shoppingTrx->getNumOfCustomSolutions() &&
        ShoppingUtil::isCustomSolution(*_shoppingTrx, (*matrixIter).first))
      dc << " [CUS]";
    dc << "\n";
    outputFlightData((*matrixIter).first);
  }
}
void
Diag930Collector::outputFlightData(const SopIdVec& fmv)
{
  DiagCollector& dc(*this);
  SopIdVec::const_iterator fmvIter = fmv.begin();
  SopIdVec::const_iterator fmvEIter = fmv.end();
  size_t fltIndex = 1;
  const std::vector<ShoppingTrx::Leg>& legs = _shoppingTrx->legs();

  for (uint32_t legIdx = 0; fmvIter != fmvEIter; ++fmvIter, ++fltIndex, ++legIdx)
  {
    const ShoppingTrx::Leg& curLeg = legs[legIdx];
    const int& curMatVal = *fmvIter;
    const ShoppingTrx::SchedulingOption& curSop = curLeg.sop()[curMatVal];
    const Itin* curItin = curSop.itin();
    std::vector<TravelSeg*>::const_iterator tSegIter = curItin->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator tSegEIter = curItin->travelSeg().end();

    if (curSop.isLngCnxSop())
      dc << "--THIS IS LONG CONNECTION TIME OPTION--\n";

    for (; tSegIter != tSegEIter; ++tSegIter)
    {
      const AirSeg* aSegPtr = dynamic_cast<const AirSeg*>(*tSegIter);
      const AirSeg& aSeg = *aSegPtr;
      dc << std::setw(2) << fltIndex << " " << std::setw(3) << aSeg.carrier();
      dc << " ";
      dc << std::setw(4) << aSeg.flightNumber() << " ";
      const DateTime& depDT = aSeg.departureDT();
      const DateTime& arrDT = aSeg.arrivalDT();
      DayOfWeek depDOW = depDT.dayOfWeek();
      ostringstream dowst;
      dowst << depDOW;
      std::string dowStr = dowst.str();
      dowStr = dowStr.substr(0, 1);
      std::string dowTransStr;
      char dowCh0 = dowStr[0];
      char dowCh1 = dowStr[1];

      switch (toupper(dowCh0))
      {
      case 'T':
        if (toupper(dowCh1) == 'H')
        {
          dowTransStr = "Q";
        }
        else
        {
          dowTransStr = "T";
        }

        break;

      case 'S':
        if (toupper(dowCh1) == 'A')
        {
          dowTransStr = "J";
        }
        else
        {
          dowTransStr = "S";
        }

        break;

      default:
        dowTransStr = dowCh0;
        break;
      }

      std::string depDTStr = depDT.timeToString(HHMM_AMPM, "");
      std::string arrDTStr = arrDT.timeToString(HHMM_AMPM, "");
      // Cut off the M from the time
      depDTStr = depDTStr.substr(0, depDTStr.length() - 1);
      arrDTStr = arrDTStr.substr(0, arrDTStr.length() - 1);
      dc << std::setw(2) << (*tSegIter)->getBookingCode() << " "
         << (*tSegIter)->bookedCabin().getCabinIndicator() << " ";
      dc << std::setw(5) << depDT.dateToString(DDMMM, "") << " " << setw(2) << dowTransStr << " ";
      dc << std::setw(3) << aSeg.origin()->loc() << "  " << std::setw(3)
         << aSeg.destination()->loc() << " ";
      dc << std::setw(5) << depDTStr << " ";
      dc << std::setw(5) << arrDTStr << " ";
      dc << std::setw(4) << aSeg.equipmentType() << " ";
      dc << std::setw(2) << aSeg.hiddenStops().size() << " ";

      if (aSeg.eticket())
      {
        dc << setw(3) << "/E";
      }

      dc << "\n";
    }
  }

  return;
}

void
Diag930Collector::printAltDatesStatistics(DiagCollector& dc)
{
  if (!_shoppingTrx || !_shoppingTrx->isAltDates())
    return;

  dc << "  ALT DATES STATISTICS" << std::endl;
  dc << "\nSOLUTIONS PER DATE PAIRS:" << std::endl;

  bool someMissed(false);
  uint32_t requested =
      static_cast<uint32_t>(_shoppingTrx->getOptions()->getRequestedNumberOfSolutions());
  PricingTrx::AltDatePairs::const_iterator datePairI = _shoppingTrx->altDatePairs().begin();
  for (; datePairI != _shoppingTrx->altDatePairs().end(); ++datePairI)
  {
    int16_t found = requested - datePairI->second->numOfSolutionNeeded;
    if (found)
    {
      dc << datePairI->first.first.dateToString(MMDDYY, "") << " "
         << datePairI->first.second.dateToString(MMDDYY, "") << " FOUND: " << found;

      if (datePairI->second->numOfSolutionNeeded)
        dc << " MISSING: " << datePairI->second->numOfSolutionNeeded << std::endl;
      else
        dc << std::endl;
    }
    else
      someMissed = true;
  }

  if (someMissed)
  {
    dc << "\nMISSING SOLUTIONS FOR DATE PAIRS:" << std::endl;

    datePairI = _shoppingTrx->altDatePairs().begin();
    for (; datePairI != _shoppingTrx->altDatePairs().end(); ++datePairI)
    {
      if (static_cast<uint32_t>(datePairI->second->numOfSolutionNeeded) == requested)
        dc << datePairI->first.first.dateToString(MMDDYY, "") << " "
           << datePairI->first.second.dateToString(MMDDYY, "") << std::endl;
    }
  }
}
}
