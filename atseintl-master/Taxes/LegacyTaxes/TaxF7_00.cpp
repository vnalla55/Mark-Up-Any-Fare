// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Taxes/LegacyTaxes/TaxF7_00.h"

#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/BaseTaxOnTaxCollector.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

using namespace tse;
using namespace std;

bool
TaxF7_00::validateTransit(PricingTrx& trx, TaxResponse& taxResponse,
      TaxCodeReg& taxCodeReg, uint16_t travelSegIndex)
{
  return true;
}

bool
TaxF7_00::validateTripTypes(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg,
                            uint16_t& startIndex,
                            uint16_t& endIndex)

{
  if (handleHiddenPoints() != HIDDEN_POINT_NOT_HANDLED)
    return false;

  Diagnostic* diag = trx.diagnostic().diagnosticType() == Diagnostic818 ?
      &trx.diagnostic() : nullptr;

  if (diag)
  {
    std::ostringstream stream;
    stream << "***\n***F7 START SPN PROCESSING***\n***\n" <<
      "START SEGMENT IDX-" << startIndex << std::endl;
    diag->insertDiagMsg(stream.str());
  }

  bool ret = false;
  if (_lastAnalysedIndex >= startIndex)
  {
    if (diag)
      diag->insertDiagMsg("ALREADY ANALYSED SEGMENT\n");
  }
  else
  {
    TaxLocIterator& locIt = *getLocIterator(*(taxResponse.farePath()), startIndex, taxCodeReg);

    uint16_t segStart, segEnd;
    ret = findEnplaneSegment(trx, taxCodeReg, locIt, segStart, diag) &&
          findDeplaneSegment(trx, taxCodeReg, locIt, segEnd, diag);
    if (ret)
    {
      startIndex = segStart;
      endIndex = segEnd;
    }

    _lastAnalysedIndex = max(segStart, segEnd);
  }

  if (diag)
    diag->insertDiagMsg("***\n***F7 END SPN PROCESSING***\n***\n");

  return ret;
}

TaxLocIterator*
TaxF7_00::getLocIterator(FarePath& farePath, uint16_t startIndex, const TaxCodeReg& taxCodeReg)
{
  TaxLocIterator* locIt = Tax::getLocIterator(farePath);
  locIt->setSkipHidden(true);
  locIt->toSegmentNo(startIndex);

  if (!taxCodeReg.restrictionTransit().empty())
    locIt->setStopHours(taxCodeReg.restrictionTransit().front().transitHours());
  else
    locIt->setStopHours(6);

  return locIt;
}

bool
TaxF7_00::findEnplaneSegment(PricingTrx& trx, TaxCodeReg& taxCodeReg, TaxLocIterator& locIt, uint16_t& segStart, Diagnostic* diag) const
{
  while (locIt.hasNext())
  {
    segStart = locIt.nextSegNo();
    if (locIt.isInLoc1(taxCodeReg, trx))
    {
      if (diag)
      {
        std::ostringstream stream;
        stream << "SEGNO-" << segStart << " ENPLANE FROM INFECTED COUNTRY- " <<
            locIt.nextSeg()->origin()->loc() << std::endl;
        diag->insertDiagMsg(stream.str());
      }
      return true;
    }

    locIt.next();
  }

  if (diag)
    diag->insertDiagMsg("NO ENPLANE FROM INFECTED COUNTRY\n");

  return false;
}

bool
TaxF7_00::findDeplaneSegment(PricingTrx& trx, TaxCodeReg& taxCodeReg, TaxLocIterator& locIt, uint16_t& segEnd, Diagnostic* diag) const
{
  while (!locIt.isEnd())
  {
    segEnd = locIt.prevSegNo();
    if (locIt.isInLoc2(taxCodeReg, trx) && locIt.isStop())
    {
      if (diag)
      {
        std::ostringstream stream;
        stream << "SEGNO-" << segEnd << " DEPLANE IN EGYPT- " <<
            locIt.prevSeg()->destination()->loc() << std::endl;
        diag->insertDiagMsg(stream.str());
      }
      return true;
    }

    locIt.next();
  }

  if (diag)
    diag->insertDiagMsg("NO DEPLANE IN EGYPT\n");

  return false;
}
