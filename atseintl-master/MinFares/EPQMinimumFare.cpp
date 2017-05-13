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
#include "MinFares/EPQMinimumFare.h"

#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"

#include <map>

namespace tse
{
EPQMinimumFare::EPQMinimumFare() {}

EPQMinimumFare::~EPQMinimumFare() {}

bool
EPQMinimumFare::process(PricingTrx& trx, const FarePath& farePath)
{
  int epqInd = 0;

  DCFactory* factory = DCFactory::instance();
  DiagCollector* diag = factory->create(trx);
  if (LIKELY(diag != nullptr))
  {
    diag->enable(Diagnostic711);
    if (LIKELY(!diag->isActive()))
    {
      diag = nullptr;
    }
  }

  if (UNLIKELY(diag != nullptr))
  {
    (*diag) << "\n******************** MINIMUM FARE EPQ DIAGNOSTIC **************\n"
            << "* EPQ CONDITION:                                              *\n"
            << "*  1. ISI IS SITI/SITO                                        *\n"
            << "*  2. SOLD IN US OR US TERRITORIES                            *\n"
            << "*  3. SOLD AND ORIGINATE IN AREA 1                            *\n"
            << "*  4. SOLD AND ORIGIN BF/BJ/CF/CG/CI/CM/GA/GQ/ML/NE/SN/TD/TG  *\n"
            << "***************************************************************\n";
  }

  const Itin* itin = farePath.itin();

  epqInd = (((farePath.intlSaleIndicator() == Itin::SITI) ||
             (farePath.intlSaleIndicator() == Itin::SITO))
                ? 1
                : 0);

  if (UNLIKELY(diag != nullptr))
  {
    (*diag) << "ISI/";

    switch (farePath.intlSaleIndicator())
    {
    case Itin::SITI:
      (*diag) << "SITI";
      break;
    case Itin::SOTI:
      (*diag) << "SOTI";
      break;
    case Itin::SITO:
      (*diag) << "SITO";
      break;
    case Itin::SOTO:
      (*diag) << "SOTO";
      break;
    default:
      break;
    }

    (*diag) << " \n";
  }

  const Loc* saleLoc = TrxUtil::saleLoc(trx);
  if (epqInd == 0)
  {
    if (diag != nullptr)
      (*diag) << "SALE/" << saleLoc->loc() << "," << saleLoc->nation() << "\n";

    epqInd = isSIUsTerritories(*saleLoc) ? 2 : 0;
  }

  const std::vector<TravelSeg*>& travelSegs = itin->travelSeg();
  const Loc* orig = origin(travelSegs);

  if (UNLIKELY(orig == nullptr))
  {
    if (diag != nullptr)
    {
      diag->flushMsg();
    }
    return false;
  }

  if (UNLIKELY(diag != nullptr && epqInd == 0))
    displayFarePath(farePath, *diag);

  if (epqInd == 0)
    epqInd = isSIAndOriginInArea1(*saleLoc, *orig) ? 3 : 0;

  if (epqInd == 0)
    epqInd = isOriginInWesternAfrica(*saleLoc, *orig) ? 4 : 0;

  if (UNLIKELY(diag != nullptr))
  {
    if (epqInd != 0)
      (*diag) << "\nEPQ SET: PASS CONDITION " << epqInd << " \n";
    else
      (*diag) << "\nEPQ NOT SET \n";

    diag->flushMsg();

  }

  if (epqInd == 0)
    return false;
  else
    return true;
}

bool
EPQMinimumFare::isSITI(const Itin& itin)
{
  if (itin.intlSalesIndicator() == Itin::SITI)
    return true;
  else
    return false;
}

bool
EPQMinimumFare::isSIUsTerritories(const Loc& saleLoc)
{
  return LocUtil::isMinFareUSTerritory(saleLoc);
}

bool
EPQMinimumFare::isSIAndOriginInArea1(const Loc& saleLoc, const Loc& orig)
{
  if ((saleLoc.area() == IATA_AREA1) && (orig.area() == IATA_AREA1))
    return true;

  return false;
}

bool
EPQMinimumFare::isOriginInWesternAfrica(const Loc& saleLoc, const Loc& orig)
{
  // Check if travel origin and ticket sold in Western Africa...
  // Western Africa countries are Benin, Burkina Faso, Cameroon,
  // Central African Republic, Chad, Congo, Cote d'Ivoire,
  // Equatorial Guinea, Gabon, Mali, Niger, Senegal or Togo.
  // Sabre zone 1100 for western Africa

  if (LocUtil::isInLoc(saleLoc, LOCTYPE_ZONE, SABRE_WESTERN_AFRICA, Vendor::SABRE, MANUAL) &&
      LocUtil::isInLoc(orig, LOCTYPE_ZONE, SABRE_WESTERN_AFRICA, Vendor::SABRE, MANUAL))
    return true;

  return false;
}

bool
EPQMinimumFare::hasHip(const FareUsage& fu)
{
  // Check if there is HIP plus up
  return (fu.minFarePlusUp().getSum(HIP) > 0);
}

void
EPQMinimumFare::displayFarePath(const FarePath& farePath, DiagCollector& diag)
{
  const std::vector<PricingUnit*>& pricingUnits = farePath.pricingUnit();
  std::vector<PricingUnit*>::const_iterator puIter = pricingUnits.begin();
  for (; puIter != pricingUnits.end(); puIter++)
  {
    // Display PU
    PricingUnit& pu = **puIter;
    diag << " PU ";
    diag << DiagnosticUtil::pricingUnitTypeToShortString(pu.puType()) << "\n";

    std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
    for (; fuIter != pu.fareUsage().end(); fuIter++)
    {
      // Display Fare Usage
      FareUsage& fu = **fuIter;
      PaxTypeFare& fare = *fu.paxTypeFare();
      FareMarket& fm = *fare.fareMarket();
      diag << "  " << fm.boardMultiCity().c_str() << "," << fm.origin()->nation() << ","
           << fm.origin()->subarea() << "-" << fm.governingCarrier().c_str() << "-"
           << fm.offMultiCity().c_str() << "," << fm.destination()->nation() << ","
           << fm.destination()->subarea() << " ";
      diag.setf(std::ios::left, std::ios::adjustfield);
      diag << std::setw(9) << fare.fareClass() << std::setw(4) << fare.fcaFareType();
      diag << "NUC ";
      diag.setf(std::ios::right, std::ios::adjustfield);
      diag.setf(std::ios::fixed, std::ios::floatfield);
      diag.precision(2);
      diag << std::setw(7) << fare.nucFareAmount() << " ";

      if (fare.mileageSurchargePctg() > 0)
        diag << fare.mileageSurchargePctg() << "M";

      diag << " \n";

      MoneyAmount hipAmount = fu.minFarePlusUp().getSum(HIP);
      if (hipAmount > 0)
      {
        diag << "  HIP ";
        diag.setf(std::ios::right, std::ios::adjustfield);
        diag.setf(std::ios::fixed, std::ios::floatfield);
        diag.precision(2);
        diag << std::setw(7) << hipAmount << "\n";
      }
    }
  }
}

/**
 * This function is prevalidation called from Itinalyzer for simple trip
 */
bool
EPQMinimumFare::process(PricingTrx& trx, Itin& itin)
{
  if (isSITI(itin))
    return true;

  const Loc* saleLoc = TrxUtil::saleLoc(trx);
  if (saleLoc == nullptr)
    return false;

  if (isSIUsTerritories(*saleLoc))
    return true;

  const std::vector<TravelSeg*>& travelSegs = itin.travelSeg();
  const Loc* orig = origin(travelSegs);

  if (isSIAndOriginInArea1(*saleLoc, *orig) || isOriginInWesternAfrica(*saleLoc, *orig))
    return true;

  return false;
}
}
