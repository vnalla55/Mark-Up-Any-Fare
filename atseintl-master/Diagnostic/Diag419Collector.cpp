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

#include "Diagnostic/Diag419Collector.h"

#include "Common/CabinType.h"
#include "Common/ClassOfService.h"
#include "Common/FareMarketUtil.h"
#include "Common/Money.h"
#include "Common/PaxTypeFareRuleDataCast.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/Diagnostic.h"

#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

namespace tse
{
// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag419Collector::bkgDiag
//
// Description:  This method will display diagnostic information to allow for a
//         quick debug of all FareBookingCodeValidation processes. Diagnostic number must be set in
//         the Transaction Orchestrator to apply the following methods:
//
//
// </PRE>
// ----------------------------------------------------------------------------
void
Diag419Collector::travelSegmentHeader()
{
  if (!_active)
    return;
  ((DiagCollector&)*this) << "  FARE BASIS   CXR V RULE TAR O O     FARE  CUR PAX BK C STAT\n"
                          << "       CLASS              NUM R I    AMOUNT     TYP    B\n";
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag419Collector::operator <<
//
// Description:  This method will override the base operator << to handle the
//               TravelSeg data for the Booking Code validation Diagnostic Display.
//
// @param  paxTfare - Diag419Collector
//
//
// </PRE>
// ----------------------------------------------------------------------------
Diag419Collector&
Diag419Collector::operator << ( const PaxTypeFare& paxTfare )
{
  if (!_active)
    return *this;
  vector<BookingCode> bkgCodes;
  DiagCollector& dc = (DiagCollector&)*this;

  showFareType(paxTfare);
  dc << setw(9) << paxTfare.fareClass() << setw(3) << paxTfare.carrier() << setw(2)
     << ((paxTfare.vendor() == Vendor::ATPCO) ? "A"
                                              : (paxTfare.vendor() == Vendor::SITA ? "S" : "?"))
     << setw(5) << paxTfare.ruleNumber();

  dc.setf(ios::right, ios::adjustfield);

  dc << setw(4) << paxTfare.fareTariff() // tcrRuleTariff()
     << setw(2) << ((paxTfare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED) ? "R" : "O") << setw(2)
     << (paxTfare.directionality() == FROM ? "O" : "I");

  dc.setf(ios::right, ios::adjustfield);
  (paxTfare.getPrimeBookingCode(bkgCodes)) ? dc << setw(3) << bkgCodes[0] : dc << setw(3) << "*";
  dc.setf(ios::left, ios::adjustfield);

  PaxTypeFare::SegmentStatusVecCI iterSegStat = paxTfare.segmentStatus().begin();
  PaxTypeFare::SegmentStatusVecCI iterSegStatEnd = paxTfare.segmentStatus().end();

  int num = 0;
  for (; iterSegStat != iterSegStatEnd; iterSegStat++)
  {
    if (num == _lineCount)
    {
      const PaxTypeFare::SegmentStatus& segStat = *iterSegStat;

      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED))
        dc << "       " << setw(15) << "NOT YET PROCESSED \n";
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH))
        dc << "       " << setw(15) << "NOMATCH \n";
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL))
        dc << "       " << setw(15) << "FAIL \n";
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_T999))
        dc << "       " << setw(15) << "FAIL T999 \n";
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_REC1_T999))
        dc << "       " << setw(15) << "FAIL REC1 T999 \n";
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV1_T999))
        dc << "       " << setw(15) << "FAIL CONV1 T999 \n";
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV2_T999))
        dc << "       " << setw(15) << "FAIL CONV2 T999 \n";
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_MIXEDCLASS))
        dc << "       " << setw(15) << "FAIL MIXEDCLASS \n";
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_LOCALMARKET))
        dc << "       " << setw(15) << "FAIL LOCALMARKET \n";
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_DOMESTIC))
        dc << "       " << setw(15) << "FAIL PRIME RBD DOMEST\n";
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_INTERNATIONAL))
        dc << "       " << setw(15) << "FAIL PRIME RBD INTERL\n";
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
      {
        dc << setw(2) << bkgCodes[0] << "       " << setw(15) << "PASS \n";
      }
    } // if
    ++num;
  } // for
  return *this;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag419Collector
//
// Description:  This method will override the base operator << to handle the
//               FareMarket data for the Booking Code validation Diagnostic Display.
//
// @param  trx - PricingTrx
// @param  mkt - FareMarket
//
//
// </PRE>
// ----------------------------------------------------------------------------
bool
Diag419Collector::finalDiag(PricingTrx& trx, const FareMarket& mkt)
{
  // process /FMDFWLON
  DiagParamMapVecIC end = trx.diagnostic().diagParamMap().end();
  DiagParamMapVecIC begin = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_MARKET);
  std::string specifiedFM("");
  std::string compFM("");

  const LocCode& fmBoardCity = FareMarketUtil::getBoardMultiCity(mkt, *(mkt.travelSeg().front()));
  const LocCode& fmOffCity = FareMarketUtil::getOffMultiCity(mkt, *(mkt.travelSeg().back()));
  const LocCode& fmBoardLoc = mkt.travelSeg().front()->origin()->loc();
  const LocCode& fmOffLoc = mkt.travelSeg().back()->destination()->loc();

  if (begin != end)
  {
    specifiedFM = (*begin).second;
    if (!(specifiedFM.empty()))
    {
      const std::string boardCity(specifiedFM.substr(0, 3));
      const std::string offCity(specifiedFM.substr(3, 3));

      if (((boardCity != fmBoardCity) && (boardCity != fmBoardLoc)) ||
          ((offCity != fmOffCity) && (offCity != fmOffLoc)))
        return true;
    }
  }

  string fareType;
  DiagCollector& dc = (DiagCollector&)*this;

  if (mkt.governingCarrier().empty())
  {
    //    dc << " *** THERE ARE NO APPLICABLE FARES FOR THIS FARE MARKET ***\n";
    return true;
  }

  // Travel segment related data

  TravelSegPtrVecIC iterTvl = mkt.travelSeg().begin();
  TravelSegPtrVecIC iterTvlEnd = mkt.travelSeg().end();

  dc << "\n" << FareMarketUtil::getBoardMultiCity(mkt, **iterTvl) << "-" << mkt.governingCarrier()
     << "-" << FareMarketUtil::getOffMultiCity(mkt, **(iterTvlEnd - 1));

  if (mkt.travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA)) // It is Domestic fare component
    dc << "   US/CA   \n\n";
  else
    dc << "   INTERNATIONAL   \n\n";

  // ClassOfService vector of ClassOfServices vectors for each TravelSeg in the FareMarket

  COSPtrVecIC iterCOS = mkt.classOfServiceVec().begin();
  LocCode destTemp;

  for (; iterTvl != iterTvlEnd; iterTvl++, iterCOS++)
  {
    dc << "    " << FareMarketUtil::getBoardMultiCity(mkt, **iterTvl) << "  ";

    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*iterTvl);

    if (airSeg)
    {
      dc << (*iterTvl)->getBookingCode() << "   AVL BKG CODES: ";

      // ClassOfService vector of vectors

      COSInnerPtrVecIC classOfS = (*iterCOS)->begin();

      for (; classOfS != (*iterCOS)->end(); classOfS++)
      {
        if ((*classOfS)->numSeats() > 0)
          dc << (*classOfS)->bookingCode();
      }
      dc << endl;
    }
    else
    {
      dc << "ARUNK \n";
    }
    destTemp = FareMarketUtil::getOffMultiCity(mkt, **iterTvl);
  } // for TravelSeg Loop
  dc << "    " << destTemp << "\n\n";

  // Travel segment related data  need to display all fare for the Fare Component

  travelSegmentHeader(); // Display Header for the fare
  if (mkt.allPaxTypeFare().empty())
  {
    dc << " *** THERE ARE NO APPLICABLE FARES FOR THIS FARE MARKET ***\n";
    dc << "--------------------------------------------------------\n";
    return true;
  }

  ((std::ostringstream&)*this) << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n";

  // Loop for all PaxTypeFare for that Travel segment
  vector<PaxTypeFare*>::const_iterator paxTfare = mkt.allPaxTypeFare().begin();
  vector<PaxTypeFare*>::const_iterator itPTFEnd = mkt.allPaxTypeFare().end();
  bool fareExist = false;

  for (; paxTfare != itPTFEnd; paxTfare++) // Big LOOP for all PaxTypeFare
  {
    // Try to find the fareClassCode in the allPaxTypeFare vector
    // if it's provided in the trx.request->fareClassCode()

    PaxTypeFare& pTf = **paxTfare;

    if (!pTf.isValid())
    {
      if (pTf.bookingCodeStatus().isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED))
        continue;
    }
    DiagParamMapVecIC end = trx.diagnostic().diagParamMap().end();
    DiagParamMapVecIC begin = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_CLASS_CODE);
    if (begin != end)
    {
      size_t len = ((*begin).second).size();
      if (len)
      {
        if (((*begin).second).substr(0, len) != pTf.fareClass())
          continue;
      }
    }

    fareExist = true;

    showFareType(pTf);
    FareClassCode fareBasis;
    const std::string& diagFC = trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);
    if (!diagFC.empty())
      fareBasis = pTf.fareClass();
    else
      fareBasis = pTf.createFareBasis(trx, false);

    if (fareBasis.size() > 12)
      fareBasis = fareBasis.substr(0, 12) + "*"; // Cross-of-lorraine?
    dc << setw(13) << fareBasis << " ";

    dc << setw(3) << pTf.carrier() << setw(2)
       << (pTf.vendor() == Vendor::ATPCO ? "A" : (pTf.vendor() == Vendor::SITA ? "S" : "?"))
       << setw(4) << pTf.ruleNumber();

    dc.setf(ios::right, ios::adjustfield);
    dc << setw(4) << pTf.fareTariff() << " "; // tcrRuleTariff()

    dc.setf(ios::left, ios::adjustfield);
    dc << std::setw(2) << DiagnosticUtil::getOwrtChar(pTf);

    if (pTf.directionality() == FROM)
      dc << setw(2) << "O";
    else if (pTf.directionality() == TO)
      dc << setw(2) << "I";
    else
      dc << "  ";

    dc.setf(ios::left, ios::adjustfield);
    dc << setw(9) << Money(pTf.fareAmount(), pTf.currency()) << " ";

    if (!pTf.isFareClassAppSegMissing())
    {
      (pTf.fcasPaxType().empty()) ? dc << "*** " : dc << setw(4) << pTf.fcasPaxType();
    }
    else
      dc << "UNK ";

    /*****************************************************
     *  DO NOT REMOVE THE NEXT BLOCK OF COMMENTED OUT CODE
     *  ITS USEFUL FOR DEBUGGING*/
    // if(!pTf.isValid())
    //{
    //  if(!pTf.areAllCategoryValid())
    //  {
    //    dc << "       FAILED RULES \n";
    //    continue;
    //  }

    // if(pTf.isRoutingProcessed())
    //{
    //  dc << "       RTG DONE";
    //}
    // else
    //{
    //  dc << "       RTG NODONE";
    //}

    // if(!pTf.isRoutingValid())
    //{
    //  dc << "/INVALID \n";
    //    continue;
    //}
    // else
    //{
    //  dc << "/VALID ";
    //}

    // if(pTf.bookingCodeStatus().isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED))
    //{
    //  dc << " BKG NODONE";
    //}
    // else
    //{
    //  dc << " BKG DONE";
    //}

    // if(!pTf.bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS))
    //{
    //  dc << "/FAIL \n";
    //    continue;
    //}
    // else
    //{
    //  dc << "/PASS ";
    //}
    // dc << " \n";
    // continue;
    //}

    vector<BookingCode> bkgCodes;
    dc.setf(ios::right, ios::adjustfield);
    if (pTf.getPrimeBookingCode(bkgCodes))
    {
      dc << setw(1) << bkgCodes[0];
    }
    else
    {
      const FBRPaxTypeFareRuleData* fbrPTFBaseFare = pTf.getFbrRuleData(RuleConst::FARE_BY_RULE);

      if (fbrPTFBaseFare)
      {
        if ((!fbrPTFBaseFare->isSpecifiedFare()) &&
            (fbrPTFBaseFare->getBaseFarePrimeBookingCode(bkgCodes))) // Base Fare
        {
          dc << setw(1) << bkgCodes[0];
        }
        else
        {
          dc << setw(1) << "*";
          bkgCodes[0] = "*";
        }
      }
    }
    dc.unsetf(ios::left);
    CabinType fareCabin(pTf.cabin());

    if (pTf.bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS)) // BKG CODE VALIDATION STATUS  - PASS
      dc << bkgCodes[0] << " " << fareCabin << " ";
    else
      dc << "  " << fareCabin << " ";

    fareType = (pTf.isNormal()) ? "N" : "S";
    dc << " " << fareType << " ";
    if (mkt.travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA))
    {
      dc << endl;
      continue;
    }
    string classStatus;
    switch (pTf.mixClassStatus())
    {
    case PaxTypeFare::MX_NOT_APPLICABLE:
      dc << endl;
      continue;
    case PaxTypeFare::MX_DIFF:
      classStatus = STAT_DIFF;
      break;
    default:
      break;
    }
    dc << classStatus << endl;
    if (!pTf.bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS))
      continue;

    vector<PaxTypeBucket>::const_iterator ptcB = mkt.paxTypeCortege().begin();
    for (; ptcB != mkt.paxTypeCortege().end(); ptcB++)
    {
      const PaxType& reqPaxType = *(*ptcB).requestedPaxType();
      dc << "    " << reqPaxType.paxType() << endl;
    }
  } // For loop for all PaxTypeFares in the Fare Market
  if (!fareExist)
  {
    dc << " *** THERE ARE NO APPLICABLE FARES FOR THIS FARE MARKET ***\n";
  }
  dc << "-----------------------------------------------------------\n";
  return true;
}

void
Diag419Collector::showFareType(const PaxTypeFare& pTf)
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << std::setw(2) << cnvFlags(pTf);
}
}
