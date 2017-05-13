// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2012
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/TaxIN.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Taxes/Common/TaxUtility.h"
#include "Common/TseUtil.h"

namespace tse
{

uint16_t
TaxIN::findMirrorImage(TaxLocIterator locIt)
{
  TaxLocIterator locIt2 = locIt;
  locIt.toFront();
  locIt2.toBack();
  NationCode prevNation = locIt.loc()->nation();
  bool locMatch = true;
  bool international = false;

  while (locIt.nextSegNo() < locIt2.prevSegNo())
  {
    if (locIt.loc()->nation() != locIt2.loc()->nation())
      return 0;

    if (locIt.loc()->nation() != prevNation)
      international = true;

    if (locIt.loc() != locIt2.loc())
      locMatch = false;

    prevNation = locIt.loc()->nation();
    locIt.next();
    locIt2.previous();
  }

  if (locIt.nextSegNo() == locIt2.nextSegNo() && (international || locMatch))
    return locIt.nextSegNo();

  return 0;
}

bool
TaxIN::validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex)
{
  TaxLocIterator* locIt = getLocIterator(*(taxResponse.farePath()));
  locIt->setSkipHidden(true);
  if (LIKELY(!taxCodeReg.restrictionTransit().empty()))
  {
    TaxRestrictionTransit& restrictTransit = taxCodeReg.restrictionTransit().front();
    if (LIKELY(restrictTransit.transitHours()))
      locIt->setStopHours(restrictTransit.transitHours());
  }
  locIt->setSurfaceAsStop(true);

  locIt->toSegmentNo(startIndex);

  bool domesticTax = false;
  if (taxCodeReg.taxCode() == "IN3")
    domesticTax = true;

  // ARRIVAL
  if (taxCodeReg.loc2() == "DEL")
  {
    if (!(locIt->isInLoc1(taxCodeReg, trx) && locIt->isNextSegAirSeg()) ||
        locIt->isInLoc2(taxCodeReg, trx))
      return false;

    bool begin = !(locIt->hasPrevious() && locIt->isPrevSegAirSeg());

    while (locIt->hasNext() && locIt->isNextSegAirSeg() && !locIt->isInLoc2(taxCodeReg, trx))
    {
      locIt->next();
      if (domesticTax && locIt->loc()->nation() != "IN")
        return false;
    }

    if (locIt->isInLoc2(taxCodeReg, trx))
    {
      if (locIt->isStop() || locIt->nextSegNo() == findMirrorImage(*locIt))
      {
        uint16_t myEndIndex = locIt->prevSegNo();

        if (begin)
          return checkMileage(trx, taxResponse, taxCodeReg, startIndex, myEndIndex, endIndex);

        const Loc* loc2 = locIt->loc();
        locIt->toSegmentNo(startIndex);
        while (locIt->hasPrevious() && locIt->isPrevSegAirSeg() &&
               (domesticTax || locIt->loc()->nation() != "IN") && locIt->loc()->loc() != "DEL")
          locIt->previous();

        if (locIt->loc()->loc() == "DEL" || (!domesticTax && locIt->loc()->nation() == "IN"))
        {
          uint32_t maxMiles = 0;
          uint16_t maxMilesSeg = 0;
          uint16_t mirrorSeg = 0;
          std::vector<TravelSeg*> tsv;
          AirSeg ts;
          ts.origin() = loc2;
          tsv.push_back(&ts);

          locIt->next();
          while (locIt->hasNext() && locIt->isNextSegAirSeg() && !locIt->isInLoc2(taxCodeReg, trx))
          {
            if (domesticTax && locIt->loc()->nation() != "IN")
              return false;

            if (locIt->nextSegNo() == findMirrorImage(*locIt))
            {
              mirrorSeg = locIt->nextSegNo();
            }

            tsv[0]->destination() = locIt->loc();
            const uint32_t curMiles =
                taxUtil::calculateMiles(trx, taxResponse, *loc2, *(locIt->loc()), tsv);

            if (curMiles > maxMiles)
            {
              maxMilesSeg = locIt->nextSegNo();
              maxMiles = curMiles;
            }
            locIt->next();
          }

          if (mirrorSeg)
          {
            if (mirrorSeg == startIndex)
              return checkMileage(trx, taxResponse, taxCodeReg, startIndex, myEndIndex, endIndex);

            return false;
          }

          if (maxMilesSeg == startIndex)
            return checkMileage(trx, taxResponse, taxCodeReg, startIndex, myEndIndex, endIndex);
        }
      }
    }
  }
  else if (LIKELY(taxCodeReg.loc1() == "DEL"))
  {
    if (!(locIt->isInLoc1(taxCodeReg, trx) && locIt->hasNext() && locIt->isNextSegAirSeg()))
      return false;

    if (!(locIt->isStop() || locIt->nextSegNo() == findMirrorImage(*locIt)))
      return false;

    const Loc* loc1 = locIt->loc();

    locIt->next();

    while (locIt->hasNext() && locIt->isNextSegAirSeg() && locIt->loc()->nation() == "IN" &&
           locIt->loc()->loc() != "DEL")
      locIt->next();

    while (locIt->hasNext() && locIt->isNextSegAirSeg() && !domesticTax &&
           locIt->loc()->nation() != "IN")
      locIt->next();

    if (domesticTax && locIt->loc()->nation() != "IN")
      return false;

    if ((domesticTax && locIt->loc()->loc() != "DEL") || locIt->loc()->nation() != "IN")
    {
      if (locIt->isInLoc2(taxCodeReg, trx))
        return checkMileage(trx, taxResponse, taxCodeReg, startIndex, locIt->prevSegNo(), endIndex);
      else
        return false;
    }

    uint16_t myEndIndex = locIt->nextSegNo();

    locIt->toSegmentNo(startIndex);
    locIt->next();

    uint16_t maxMiles = 0;
    uint16_t maxMilesSeg = 0;
    std::vector<TravelSeg*> tsv;
    AirSeg ts;
    ts.origin() = loc1;
    tsv.push_back(&ts);

    while (locIt->hasNext() && locIt->isNextSegAirSeg() && locIt->nextSegNo() != myEndIndex)
    {
      if (locIt->nextSegNo() == findMirrorImage(*locIt))
      {
        if (locIt->isInLoc2(taxCodeReg, trx))
          return checkMileage(trx, taxResponse, taxCodeReg, startIndex, locIt->prevSegNo(), endIndex);
        else
          return false;
      }

      tsv[0]->destination() = locIt->loc();
      uint16_t curMiles = taxUtil::calculateMiles(trx, taxResponse, *loc1, *(locIt->loc()), tsv);

      if (curMiles > maxMiles)
      {
        maxMilesSeg = locIt->nextSegNo();
        maxMiles = curMiles;
      }
      locIt->next();
    }

    if (maxMilesSeg)
    {
      locIt->toSegmentNo(maxMilesSeg);
      if (locIt->isInLoc2(taxCodeReg, trx))
        return checkMileage(trx, taxResponse, taxCodeReg, startIndex, locIt->prevSegNo(), endIndex);
    }
  }
  return false;
}

}
