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

#include "Diagnostic/Diag400Collector.h"

#include "Common/CabinType.h"
#include "Common/ClassOfService.h"
#include "Common/FareMarketUtil.h"
#include "Common/Money.h"
#include "Common/PaxTypeFareRuleDataCast.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
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
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/Diagnostic.h"

#include <iomanip>
#include <iostream>
#include <vector>

namespace tse
{
Diag400Collector&
Diag400Collector::operator << ( const FareMarket& mkt )
{
  if (!_active)
    return *this;
  DiagCollector& dc = (DiagCollector&)*this;
  dc << "************************************************************* \n";

  if (mkt.travelSeg().size() == 0 || mkt.governingCarrier().empty() || mkt.allPaxTypeFare().empty())
  {
    dc << " *** NO APPLICABLE FARES FOR FARE MARKET " << mkt.boardMultiCity() << "-"
       << mkt.offMultiCity() << " *** \n";
    dc << "----------------------------------------------------------- \n";

    return *this;
  }

  // Travel segment related data
  std::vector<TravelSeg*>::const_iterator iterTvl = mkt.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator iterTvlEnd = mkt.travelSeg().end();

  dc << "\n" << FareMarketUtil::getBoardMultiCity(mkt, **iterTvl) << "-" << mkt.governingCarrier()
     << "-" << FareMarketUtil::getOffMultiCity(mkt, **(iterTvlEnd - 1));

  if (mkt.travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA)) // It is Domestic fare component
    dc << "   US/CA   \n\n";
  else
    dc << "   INTERNATIONAL   \n\n";

  if (mkt.classOfServiceVec().size() != mkt.travelSeg().size()) // prevent segmentation violation
  {
    dc << "NO BOOKING CODE AVAILABLE - SIZE PROBLEM ";
    return *this;
  }

  std::vector<std::vector<ClassOfService*>*>::const_iterator iterCOS =
      mkt.classOfServiceVec().begin();

  LocCode destTemp;

  int num = 0;

  for (; iterTvl != iterTvlEnd; iterTvl++, iterCOS++)
  {
    dc << "     " << FareMarketUtil::getBoardMultiCity(mkt, **iterTvl) << "  ";
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*iterTvl);
    if (airSeg != nullptr)
    {
      dc << (*iterTvl)->getBookingCode() << "   AVL BKG CODES: ";
      // ClassOfService vector of vectors
      std::vector<ClassOfService*>::const_iterator classOfS = (*iterCOS)->begin();
      for (; classOfS != (*iterCOS)->end(); classOfS++)
      {
        num = (*classOfS)->numSeats();
        if (num > 0)
          dc << (*classOfS)->bookingCode();
      } // COS of Service inner loop
      dc << "\n";
    }
    else
    {
      dc << "ARUNK \n";
    }
    destTemp = FareMarketUtil::getOffMultiCity(mkt, **iterTvl);
  } // for TravelSeg Loop

  dc << "     " << destTemp << "\n\n";
  travelSegmentHeader();
  return *this;
}

// ----------------------------------------------------------------------------
Diag400Collector&
Diag400Collector::operator << (const std::pair<PricingTrx *,  PaxTypeFare *> &output)
{
  if (!_active)
    return *this;
  DiagCollector& dc = (DiagCollector&)*this;
  PricingTrx& trx = *(output.first);
  const PaxTypeFare& pTf = *(output.second);

  std::vector<BookingCode> bkgCodes;
  dc.setf(std::ios::left, std::ios::adjustfield);

  // display fare type flags
  dc << std::setw(2) << cnvFlags(pTf);

  // display fare class  or fare basis code
  FareClassCode fareClass;
  const std::string& diagFC = trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);
  if (!diagFC.empty())
    fareClass = pTf.fareClass();
  else
    fareClass = pTf.createFareBasis(trx, false);

  if (fareClass.size() > 12)
    fareClass = fareClass.substr(0, 12) + "*"; // Cross-of-lorraine?
  dc << std::setw(13) << fareClass << " ";

  // display carrier, vendor and rule number
  dc << std::setw(3) << pTf.carrier() << std::setw(2) << Vendor::displayChar(pTf.vendor())
     << std::setw(4) << pTf.ruleNumber();

  dc.unsetf(std::ios::left);
  dc.setf(std::ios::right, std::ios::adjustfield);

  // display tariff number
  dc << std::setw(4) << pTf.fareTariff() << " "; // tcrRuleTariff()

  dc.unsetf(std::ios::right);
  dc.setf(std::ios::left, std::ios::adjustfield);

  // display one way roundtrip
  dc << std::setw(2) << DiagnosticUtil::getOwrtChar(pTf);

  // display TO FROM
  if (pTf.directionality() == FROM)
    dc << std::setw(2) << "O";
  else if (pTf.directionality() == TO)
    dc << std::setw(2) << "I";
  else
    dc << "  ";

  dc.unsetf(std::ios::right);
  dc.setf(std::ios::left, std::ios::adjustfield);

  // display fare amount and currency
  dc << std::setw(9) << Money(pTf.fareAmount(), pTf.currency()) << " ";

  // display pax type
  if (!pTf.isFareClassAppSegMissing())
  {
    if (pTf.fcasPaxType().empty())
      dc << "*** ";
    else
      dc << std::setw(4) << pTf.fcasPaxType();
  }
  else
  {
    dc << "UNK ";
  }

  // display  prime booking code
  dc.unsetf(std::ios::left);
  dc.setf(std::ios::right, std::ios::adjustfield);

  if (pTf.getPrimeBookingCode(bkgCodes))
  {
    dc << std::setw(1) << bkgCodes[0];
  }
  else
  {
    const FBRPaxTypeFareRuleData* fbrPTFBaseFare = pTf.getFbrRuleData(RuleConst::FARE_BY_RULE);
    if (fbrPTFBaseFare != nullptr)
    {
      if ((!fbrPTFBaseFare->isSpecifiedFare()) &&
          (fbrPTFBaseFare->getBaseFarePrimeBookingCode(bkgCodes))) // Base Fare
      {
        dc << std::setw(1) << bkgCodes[0];
      }
      else
      {
        dc << std::setw(1) << "*";
        bkgCodes[0] = "*";
      }
    }
  }

  // display PASS, fare cabin, Published and Normal special
  CabinType fareCabin(pTf.cabin());
  if (pTf.bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS) ||
      pTf.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL))
  {
    if (pTf.bookingCodeStatus().isSet(PaxTypeFare::BKS_PASS))
      dc << bkgCodes[0] << " ";
    else
      dc << "  ";

    dc << fareCabin << " " << (pTf.fare()->isPublished() ? "P" : " ")
       << (pTf.isNormal() ? " NL" : " SP") << "\n" << addVendorLine(pTf);
  }
  else
  {
    dc << "  " << fareCabin;
    dc.unsetf(std::ios::left);
    mixClassMessage(trx, pTf);
  }
  dc.unsetf(std::ios::left);
  return *this;
}

// ----------------------------------------------------------------------------
void
Diag400Collector::mixClassMessage(PricingTrx& trx, const PaxTypeFare& pTf)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;

  _highCabin4 = false;
  _highCabin7 = false;

  int reBook = 0;
  int commonFail = 0;
  int lowerCabin = 0;
  int higherCabin = 0;
  int sameCabin = 0;
  bool availFound = false;
  CabinType fareCabin(pTf.cabin());
  CabinType segmentCabin(pTf.cabin());

  std::string fareType;
  BookingCode bkgCode;

  const FareMarket& mkt = *(pTf.fareMarket());

  typedef std::vector<PaxTypeFare::SegmentStatus>::const_iterator SegmentStatusVecCI;

  uint16_t sizeTs = mkt.travelSeg().size();
  std::vector<TravelSeg*>::const_iterator iterTvl = mkt.travelSeg().begin();
  SegmentStatusVecCI iterSegStat = pTf.segmentStatus().begin();
  if (pTf.segmentStatus().size() == 0)
  {
    dc << " "
       << "ERR -SEGSTATUS 0 "
       << "\n" << addVendorLine(pTf);
    return;
  }
  // ClassOfService vector of ClassOfServices vectors for each TravelSeg in the FareMarket
  std::vector<std::vector<ClassOfService*>*>::const_iterator iterCOS =
      mkt.classOfServiceVec().begin();

  for (; (iterTvl != mkt.travelSeg().end()) && (iterSegStat != pTf.segmentStatus().end()) &&
             (iterCOS != mkt.classOfServiceVec().end());
       iterTvl++, iterSegStat++, iterCOS++)
  {
    availFound = false; // no availability found
    const AirSeg* airSeg1 = dynamic_cast<const AirSeg*>(*iterTvl);
    if (airSeg1 == nullptr)
      continue;

    bkgCode = (**iterTvl).getBookingCode();
    segmentCabin = (**iterTvl).bookedCabin(); // cabin for the BC in travelSeg

    const PaxTypeFare::SegmentStatus& segStat = *iterSegStat;
    if (segStat._bkgCodeSegStatus.isNull())
    {
      dc << " "
         << "ERR -ISNULL "
         << "\n" << addVendorLine(pTf);
      continue;
    }
    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_SURFACE))
    {
      sizeTs--;
    }
    else
    {
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_T999) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_REC1_T999) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV1_T999) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV2_T999) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_MIXEDCLASS) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_LOCALMARKET) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_DOMESTIC) ||
          segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_INTERNATIONAL))
      {
        ++commonFail;

        if (trx.getRequest()->isLowFareRequested() && // WPNC entry
            segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
          ++reBook;

        // ClassOfService vector for the particular Travel Segment
        std::vector<ClassOfService*>::const_iterator classOfS = (*iterCOS)->begin();

        for (; classOfS != (*iterCOS)->end(); classOfS++)
        {
          ClassOfService& cOs = **classOfS;
          //@TODO Need to restor the original code

          if (cOs.bookingCode().find(bkgCode[0], 0) == 0)
          {
            availFound = true; // availability found
            if (fareCabin == cOs.cabin())
            {
              ++sameCabin;
            }
            else if (fareCabin < cOs.cabin())
            {
              ++lowerCabin;
            }
            else if (fareCabin > cOs.cabin())
            {
              ++higherCabin;
              collectHighCabin(cOs.cabin());
            }
          } // if found Booking Code in the Class Of Service
        } // loop for Class of service objects for the particular Travel Segment
        if (!availFound)
        {
          if (fareCabin == segmentCabin)
          {
            ++sameCabin;
          }
          else if (fareCabin < segmentCabin)
          {
            ++lowerCabin;
          }
          else if (fareCabin > segmentCabin)
          {
            ++higherCabin;
            collectHighCabin(segmentCabin);
          }
        }
      } // if FAIL segment status is Found
    } // if SURFACE segment status
  } // loop for all Travel Segment

  if (trx.getRequest()->isLowFareRequested()) // WPNC entry
  {
    fareType = "";
    if (pTf.isNormal())
    {
      fareType = "N";

      dc << " " << (pTf.fare()->isPublished() ? "P" : " ") << " NL"
         << "\n" << addVendorLine(pTf);

      if (!mkt.travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA) &&
          ((commonFail != sizeTs) || ((commonFail == sizeTs) && reBook != 0)))
      {
        if (lowerCabin == 0 && mkt.travelSeg().size() != 1)
        {
          if (((fareCabin.isBusinessClass() && isHighCabin4()) ||
               (fareCabin.isEconomyClass() && isHighCabin7())) &&
               !pTf.isFareByRule())
          {
            dc << "             HIGHER PREMIUM CABIN -NEED DIFF -SEE DIAG 420 413"
               << "\n";
          }
          else if (!pTf.isFareByRule())
          {
            if (commonFail == higherCabin)
              dc << "                HIGHER CABIN -NEED DIFF - SEE DIAG 413"
                 << "\n";
            else if (commonFail != sameCabin)
              dc << "                ALL CABIN -NEED DIFF - SEE DIAG 413"
                 << "\n";
          }
        }
      }
    }
    else // Special fare
    {
      fareType = "S";
      dc << " " << (pTf.fare()->isPublished() ? "P" : " ") << " SP"
         << "\n" << addVendorLine(pTf);
    }
  }
  else // WP entry
  {
    fareType = "";
    if (pTf.isNormal()) // Normal fare processing
    {
      fareType = "N";
      if (mkt.travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA) || (commonFail == sizeTs))
      {
        dc << " " << (pTf.fare()->isPublished() ? "P" : " ") << " NL"
           << "\n" << addVendorLine(pTf);
      }
      else
      {
        if (commonFail == higherCabin)
        {
          if (mkt.travelSeg().size() == 1)
          {
            dc << " " << (pTf.fare()->isPublished() ? "P" : " ") << " NL"
               << "\n" << addVendorLine(pTf);
          }
          else
          {
            dc << " " << (pTf.fare()->isPublished() ? "P" : " ") << " NL"
               << "\n" << addVendorLine(pTf);
            if (!pTf.isFareByRule())
            {
              if ((fareCabin.isBusinessClass() && isHighCabin4()) ||
                  (fareCabin.isEconomyClass() && isHighCabin7()))
                dc << "             HIGHER PREMIUM CABIN -NEED DIFF -SEE DIAG 420 413"
                   << "\n";
              else
                dc << "                HIGHER CABIN -NEED DIFF - SEE DIAG 413"
                   << "\n";
            }
          }
        }
        else if (commonFail == sameCabin)
        {
          dc << " " << (pTf.fare()->isPublished() ? "P" : " ") << " NL"
             << "\n" << addVendorLine(pTf);
        }
        else if ((sameCabin > 0) && (higherCabin > 0) && (lowerCabin == 0))
        {
          if (mkt.travelSeg().size() == 1)
          {
            dc << " " << (pTf.fare()->isPublished() ? "P" : " ") << " NL"
               << "\n" << addVendorLine(pTf);
          }
          else
          {
            dc << " " << (pTf.fare()->isPublished() ? "P" : " ") << " NL"
               << "\n" << addVendorLine(pTf);
            if (!pTf.isFareByRule())
            {
              if ((fareCabin.isBusinessClass() && isHighCabin4()) ||
                  (fareCabin.isEconomyClass() && isHighCabin7()))
                dc << "             HIGHER PREMIUM CABIN -NEED DIFF -SEE DIAG 420 413"
                   << "\n";
              else
                dc << "                ALL CABIN -NEED DIFF - SEE DIAG 413"
                   << "\n";
            }
          }
        }
      }
    }
    else // Special fare processing
    {
      fareType = "S";
      dc << " " << (pTf.fare()->isPublished() ? "P" : " ") << " SP"
         << "\n" << addVendorLine(pTf);
    }
  }
}

// ----------------------------------------------------------------------------

void
Diag400Collector::travelSegmentHeader()
{
  if (_active)
  {
    ((DiagCollector&)*this) << "  FARE BASIS   CXR V RULE TAR O O     FARE  CUR PAX BK C STAT\n"
                            << "       CLASS              NUM R I    AMOUNT     TYP    B\n"
                            << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n";
  }
}

// ----------------------------------------------------------------------------
Diag400Collector&
Diag400Collector::operator << ( const PaxTypeFare& paxTfare )
{
  if (_active)
  {

    std::vector<BookingCode> bkgCodes;

    DiagCollector& dc = (DiagCollector&)*this;

    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << std::setw(2) << cnvFlags(paxTfare);

    dc << std::setw(9) << paxTfare.fareClass() << std::setw(3) << paxTfare.carrier() << std::setw(2)
       << Vendor::displayChar(paxTfare.vendor()) << std::setw(5) << paxTfare.ruleNumber();

    dc.setf(std::ios::right, std::ios::adjustfield);

    dc << std::setw(4) << paxTfare.fareTariff() // tcrRuleTariff()
       << std::setw(2) << (paxTfare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED ? "R" : "O")
       << std::setw(2) << (paxTfare.directionality() == FROM ? "O" : "I");

    dc.setf(std::ios::right, std::ios::adjustfield);
    if (paxTfare.getPrimeBookingCode(bkgCodes))
    {
      dc << std::setw(3) << bkgCodes[0];
    }
    else
    {
      dc << std::setw(3) << "*";
    }
    dc.setf(std::ios::left, std::ios::adjustfield);

    PaxTypeFare::SegmentStatusVecCI iterSegStat = paxTfare.segmentStatus().begin();
    PaxTypeFare::SegmentStatusVecCI iterSegStatEnd = paxTfare.segmentStatus().end();

    int num = 0;

    for (; iterSegStat != iterSegStatEnd; iterSegStat++)
    {
      if (num == _lineCount)
      {
        const PaxTypeFare::SegmentStatus& segStat = *iterSegStat;

        if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED))
          dc << "       " << std::setw(15) << "NOT YET PROCESSED \n";
        if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH))
          dc << "       " << std::setw(15) << "NOMATCH \n";
        if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL))
          dc << "       " << std::setw(15) << "FAIL \n";
        if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_T999))
          dc << "       " << std::setw(15) << "FAIL T999 \n";
        if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_REC1_T999))
          dc << "       " << std::setw(15) << "FAIL REC1 T999 \n";
        if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV1_T999))
          dc << "       " << std::setw(15) << "FAIL CONV1 T999 \n";
        if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV2_T999))
          dc << "       " << std::setw(15) << "FAIL CONV2 T999 \n";
        if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_MIXEDCLASS))
          dc << "       " << std::setw(15) << "FAIL MIXEDCLASS \n";
        if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_LOCALMARKET))
          dc << "       " << std::setw(15) << "FAIL LOCALMARKET \n";
        if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_DOMESTIC))
          dc << "       " << std::setw(15) << "FAIL PRIME RBD DOMEST\n";
        if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_INTERNATIONAL))
          dc << "       " << std::setw(15) << "FAIL PRIME RBD INTERL\n";
        if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
        {
          dc << std::setw(2) << bkgCodes[0] << "       " << std::setw(15) << "PASS \n";
        }
      } // if

      ++num;

    } // for
    addVendorLine(paxTfare);
  }

  return *this;
}

//-------------------------------------------------------------------
bool
Diag400Collector::failedSegInEconomy(const FareMarket& mkt, const PaxTypeFare& paxTfare)
{
  typedef std::vector<TravelSeg*>::const_iterator TravelSegPtrVecIC;
  typedef std::vector<PaxTypeFare::SegmentStatus>::const_iterator SegmentStatusVecCI;
  TravelSegPtrVecIC iterTvl = mkt.travelSeg().begin();
  TravelSegPtrVecIC iterTvlEnd = mkt.travelSeg().end();
  SegmentStatusVecCI iterSegStat = paxTfare.segmentStatus().begin();
  SegmentStatusVecCI iterSegStatEnd = paxTfare.segmentStatus().end();

  for (; (iterTvl != iterTvlEnd) && (iterSegStat != iterSegStatEnd); iterTvl++, iterSegStat++)
  {
    const PaxTypeFare::SegmentStatus& segStat = *iterSegStat;
    const AirSeg* pAirSeg = dynamic_cast<const AirSeg*>(*iterTvl);
    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_SURFACE) || pAirSeg == nullptr)
    {
      continue;
    }
    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_T999) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_REC1_T999) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV1_T999) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV2_T999) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_MIXEDCLASS) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_LOCALMARKET) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_DOMESTIC) ||
        segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_INTERNATIONAL))
    {
      if (!(segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED)) &&
          pAirSeg->bookedCabin().isEconomyClass())
      {
        return true;
      }
    }
  }
  return false;
}

//-------------------------------------------------------------------
bool
Diag400Collector::tryRule2(PricingTrx& trx, const FareMarket& mkt)
{
  // if its not a WPNC entry - do not try Rule2
  if (!(trx.getRequest()->isLowFareRequested()))
    return false;

  // do not try Rule 2 if the customer does not want it
  if (trx.getRequest()->ticketingAgent() == nullptr ||
      trx.getRequest()->ticketingAgent()->agentTJR() == nullptr)
  {
    return false;
  }

  if (trx.getRequest()->ticketingAgent()->agentTJR()->availabilityIgRul2St() == YES)
    return false;

  // do not do Rule 2 if carrier does not want it
  if (mkt.governingCarrierPref()->availabilityApplyrul2st() == YES)
    return false;

  // if only one flight in this fare component then no need to try Rule 2
  if (mkt.travelSeg().size() < 2)
    return false;

  return true;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag400Collector::operator <<
//
// Description:  This method will be the override base operator << to handle the
//                FareMarket data for the Booking Code validation Diagnostic Display.
//
// @param  mkt - FareMarket
//
//
// </PRE>
// ----------------------------------------------------------------------------

bool
Diag400Collector::finalDiagTest(const PricingTrx& trx, const FareMarket& mkt)
{

  DiagCollector& dc = (DiagCollector&)*this;

  if (mkt.governingCarrier().empty())
  {
    dc << " *** THERE ARE NO APPLICABLE FARES FOR THIS FARE MARKET ***\n";
    return true;
  }

  // Travel segment related data
  std::vector<TravelSeg*>::const_iterator iterTvl = mkt.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator iterTvlEnd = mkt.travelSeg().end();

  dc << "\n" << FareMarketUtil::getBoardMultiCity(mkt, **iterTvl) << "-" << mkt.governingCarrier()
     << "-" << FareMarketUtil::getOffMultiCity(mkt, **(iterTvlEnd - 1));

  if (mkt.travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA)) // It is Domestic fare component
    dc << "   US/CA   \n\n";
  else
    dc << "   INTERNATIONAL   \n\n";

  // ClassOfService vector of ClassOfServices vectors for each TravelSeg in the FareMarket
  std::vector<std::vector<ClassOfService*>*>::const_iterator iterCOS =
      mkt.classOfServiceVec().begin();
  std::vector<std::vector<ClassOfService*>*>::const_iterator iterCOSEnd =
      mkt.classOfServiceVec().end();

  if (mkt.classOfServiceVec().size() != mkt.travelSeg().size()) // prevent segmentation violation
  {
    dc << "NO BOOKING CODE AVAILABLE - SIZE PROBLEM ";
    return true;
  }

  LocCode destTemp;

  for (; iterTvl != iterTvlEnd; iterTvl++)
  {

    dc << "    " << FareMarketUtil::getBoardMultiCity(mkt, **iterTvl) << "  ";

    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*iterTvl);

    if (airSeg != nullptr)
    {
      dc << (*iterTvl)->getBookingCode() << "   AVL BKG CODES: ";

      for (; iterCOS != iterCOSEnd; iterCOS++)
      {

        // ClassOfService vector of vectors
        std::vector<ClassOfService*>::const_iterator classOfS = (*iterCOS)->begin();
        std::vector<ClassOfService*>::const_iterator classOfSEnd = (*iterCOS)->end();

        for (; classOfS != classOfSEnd; classOfS++)
        {
          // ClassOfService& cos = (*classOfS);

          if ((*classOfS)->numSeats() > 0)
            dc << (*classOfS)->bookingCode();
        }
      }

      dc << "\n";
    }
    else
    {
      dc << "ARUNK \n";
    }

    destTemp = FareMarketUtil::getOffMultiCity(mkt, **iterTvl);
  } // for TravelSeg Loop

  dc << "    " << destTemp << "\n\n";

  // Travel segment related data  need to display all fare for particular TravelSegment
  std::vector<TravelSeg*>::const_iterator itTvl = mkt.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator itTvlEnd = mkt.travelSeg().end();

  _lineCount = 0;

  for (; itTvl != itTvlEnd; itTvl++)
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*itTvl);

    if (airSeg != nullptr)
    {
      dc << FareMarketUtil::getBoardMultiCity(mkt, **itTvl) << "-" << airSeg->carrier() << "-"
         << FareMarketUtil::getOffMultiCity(mkt, **itTvl) << "\n";
    }
    else
    {
      _lineCount++;
      continue;
    }

    travelSegmentHeader(); // Display Header for the fare

    if (mkt.allPaxTypeFare().empty())
    {
      dc << " *** THERE ARE NO APPLICABLE FARES FOR THIS FARE MARKET ***\n";
    }

    // Loop for all PaxTypeFare for that Travel segment
    std::vector<PaxTypeFare*>::const_iterator paxTFare = mkt.allPaxTypeFare().begin();
    std::vector<PaxTypeFare*>::const_iterator itPTFEnd = mkt.allPaxTypeFare().end();

    for (; paxTFare != itPTFEnd; paxTFare++) // Big LOOP for all PaxTypeFare
    {
      // Try to find the fareClassCode in the allPaxTypeFare vector
      // if it's provided in the trx.request->fareClassCode()

      PaxTypeFare& pTf = **paxTFare;

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

      dc << (**paxTFare) << "\n";
    }

    _lineCount++; // Bump to next Travel seg
  }

  dc << "--------------------------------------------------------\n";

  return true;
}

//-------------------------------------------------------------------
void
Diag400Collector::collectHighCabin(CabinType cabin)
{
  // this method is for Premium Cabin logic only
  if (cabin.isPremiumBusinessClass()) // cabin 4
    highCabin4() = true;
  else if (cabin.isPremiumEconomyClass())
    highCabin7() = true; // cabin 7

  return;
}

std::string
Diag400Collector::addVendorLine(const PaxTypeFare& ptf) const
{
  std::string res;
  if (Vendor::displayChar(ptf.vendor()) == '*')
  {
    res = "                   " + ptf.vendor() + "\n";
  }
  return res;
}
} // namespace tse
