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

#include "Diagnostic/Diag411Collector.h"

#include "Common/ClassOfService.h"
#include "Common/FareMarketUtil.h"
#include "Common/Money.h"
#include "Common/PaxTypeFareRuleDataCast.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CarrierMixedClass.h"
#include "Rules/RuleConst.h"

#include <iostream>
#include <vector>

namespace tse
{
// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag411Collector::bkg411Header
//
// Description:  This method will display diagnostic information to allow for a
//         quick debug of all FareBookingCodeValidation processes. Diagnostic number must be set in
//         the Transaction Orchestrator to apply the following methods:
//
//
// </PRE>
// ----------------------------------------------------------------------------
void
Diag411Collector::bkg411Header()
{
  if (_active)
  {
    ((DiagCollector&)*this) << "  FARE BASIS   CXR V RULE TAR O O  FARE AMT CUR  PAX C FARE \n"
                            << "       CLASS              NUM R I                TPE B TYPE \n";
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag411Collector::operator <<
//
// Description:  This method will be the override base operator << to handle the
//                FareMarket data for the Booking Code validation Diagnostic Display.
//
// @param  mkt - FareMarket
//
//
// </PRE>
// ----------------------------------------------------------------------------
Diag411Collector&
Diag411Collector::operator << ( const FareMarket& mkt )
{
  if (!_active)
    return *this;

  DiagCollector& dc = (DiagCollector&)*this;

  // If we dont have travel segments, we count output this line
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

  bkg411Header();

  return *this;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag411Collector::operator <<
//
// Description:  This method will be the override base operator << to handle the
//                TravelSeg data for the Booking Code validation Diagnostic Display.
//
// @param  pTfare - PaxTypeFare
//
//
// </PRE>
// ----------------------------------------------------------------------------
Diag411Collector&
Diag411Collector::operator << (const std::pair<PricingTrx *,  PaxTypeFare *> &output)
{
  if (_active)
  {
    ((std::ostringstream&)*this)
        << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n";

    PricingTrx* trx = output.first;
    PaxTypeFare* paxTfare = output.second;
    FareClassCode fareClass;
    DiagCollector& dc = (DiagCollector&)*this;

    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << std::setw(2) << cnvFlags(*paxTfare);

    const std::string& diagFC = trx->diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);
    if (!diagFC.empty())
      fareClass = paxTfare->fareClass();
    else
      fareClass = paxTfare->createFareBasis(trx, false);

    if (fareClass.size() > 12)
      fareClass = fareClass.substr(0, 12) + "*";
    dc << std::setw(13) << fareClass << " ";

    dc << std::setw(3) << paxTfare->carrier() << std::setw(2)
       << Vendor::displayChar(paxTfare->vendor()) << std::setw(4) << paxTfare->ruleNumber();
    dc.unsetf(std::ios::left);
    dc.setf(std::ios::right, std::ios::adjustfield);

    dc << std::setw(4) << paxTfare->fareTariff() << " "; // tcrRuleTariff()

    dc.unsetf(std::ios::right);
    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << std::setw(2) << DiagnosticUtil::getOwrtChar(*paxTfare);

    if (paxTfare->directionality() == FROM)
      dc << std::setw(2) << "O";
    else if (paxTfare->directionality() == TO)
      dc << std::setw(2) << "I";
    else
      dc << "  ";

    dc << std::setw(9) << Money(paxTfare->fareAmount(), paxTfare->currency()) << "  ";

    if (!paxTfare->isFareClassAppSegMissing())
    {
      if (paxTfare->fcasPaxType().empty())
        dc << "*** ";
      else
        dc << std::setw(4) << paxTfare->fcasPaxType();
    }
    else
    {
      dc << "UNK ";
    }

    dc << paxTfare->cabin() << " ";
    char tmpBuf[128];
    sprintf(tmpBuf, "%d", paxTfare->fareTypeDesignator().fareTypeDesig());
    FareType fareType = (paxTfare->fcaFareType() + tmpBuf).c_str();
    dc << std::setw(4) << fareType;

    dc << (paxTfare->isNormal() ? " NL" : " SP");
    if (Vendor::displayChar(paxTfare->vendor()) == '*')
    {
      dc << std::endl << "                   " << paxTfare->vendor();
    }

    ((std::ostringstream&)*this) << " \n";

    if (paxTfare->iAmAsBookedClone())
      dc << "  ASBOOKED / CLONE";

    ((std::ostringstream&)*this) << " \n";

    if (paxTfare->isFareByRule())
    {
      const FBRPaxTypeFareRuleData* fbrPaxTypeFareRuleData =
          paxTfare->getFbrRuleData(RuleConst::FARE_BY_RULE);

      if (fbrPaxTypeFareRuleData)
      {
        if (fbrPaxTypeFareRuleData->isBaseFareAvailBkcMatched())
        {
          dc << "\n      **BASE FARE PRIME RBD AVAILABILTY REQUIREMENT EXISTS**";
          ((std::ostringstream&)*this) << " \n";
        }
      }
    }
  }
  return *this;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag411Collector::operator <<
//
// Description:  This method will be the override base operator << to handle the
//               the BookingCode status in the Fare class.
//
// @param  bkgStatus - BookingCodeStatus
//
//
// </PRE>
// ----------------------------------------------------------------------------
Diag411Collector&
Diag411Collector::operator << ( const PaxTypeFare::BookingCodeStatus& bkgStatus )
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);
    if(pricingTrx && pricingTrx->getOptions() && pricingTrx->getOptions()->isRtw() && bkgStatus.isSet(PaxTypeFare::BKS_MIXED))
    {
      dc << "\n      RW BKG CODE VALIDATION STATUS          - FAIL";
      return *this;
    }

    dc << "\n      BKG CODE VALIDATION STATUS          - ";

    if (bkgStatus.isSet(PaxTypeFare::BKS_FAIL))
    {
      dc << "FAIL";
    }
    else if (bkgStatus.isSet(PaxTypeFare::BKS_PASS))
    {
      dc << "PASS";
    }
    else if (bkgStatus.isSet(PaxTypeFare::BKS_MIXED))
    {
      dc << "MIXED";
    }
    else if (bkgStatus.isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED))
    {
      dc << "NOMATCH";
    }
    else if (bkgStatus.isSet(PaxTypeFare::BKS_MIXED_FLOW_AVAIL))
    {
      dc << "MIXED FLOW";
    }
    else if (bkgStatus.isSet(PaxTypeFare::BKS_PASS_LOCAL_AVAIL))
    {
      dc << "PASS LOCAL";
    }
    else if (bkgStatus.isSet(PaxTypeFare::BKS_PASS_FLOW_AVAIL))
    {
      dc << "PASS FLOW";
    }
    dc << "\n";
  }

  return *this;
}

void
Diag411Collector::printIbfErrorMessage(IbfErrorMessage errorMsg)
{
  if (_active && (_trx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "BRAND"))
  {
    DiagCollector& dc = (DiagCollector&)*this;
    std::stringstream ibfErrorMessageTxt;
    ibfErrorMessageTxt << "\n      IBF ERROR MESSAGE           - " << errorMsg;
    dc << ibfErrorMessageTxt.str() << std::endl;
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag411Collector::operator <<
//
// Description:  This method will be the override base operator << to handle the
//               the BookingCode status in the Fare class.
//
// @param  bkgStatus - BookingCodeStatus
//
//
// </PRE>
// ----------------------------------------------------------------------------
Diag411Collector&
Diag411Collector::operator << ( const std::vector<BookingCode>& bookingCodeVec )
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    dc << "\n      PRIME RBD REC 1 BKG CODES   - ";

    if (bookingCodeVec.size() != 0)
    {
      for (const auto elem : bookingCodeVec)
      {
        dc << elem << " ";
      }
    }
    else
      dc << "N/A";

    dc << "\n";
  }
  return *this;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag411Collector::operator <<
//
// Description:  This method will sent messages to diag buffer. All messages are in the
//               enum  Diag411Message
//
// @param  Diag411Message - enum number
//
//
// </PRE>
// ----------------------------------------------------------------------------
void
Diag411Collector::printMessages(int msg, char convNum)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    switch (msg)
    {
    case 1:
      dc << "      PRIME RBD VALIDATION STATUS - PASS \n";
      break;
    case 2:
      dc << "      PRIME RBD VALIDATION STATUS - FAIL \n";
      break;
    case 3:
      dc << "\n      TABLE 999 VALIDATION STATUS FOR THE FARE - PASS\n";
      break;
    case 4:
      dc << "\n      TABLE 999 VALIDATION STATUS FOR THE FARE - FAIL\n";
      break;
    case 5:
      dc << "\n      TABLE 999 VALIDATION STATUS FOR THE FARE - MIXED\n";
      break;
    case 6:
      dc << "      REC 1 TABLE 999 ITEM NUMBER FOR ";
      break;
    case 7:
      dc << "\n\n      MIX CLASS VALIDATION STATUS - PASS \n";
      break;
    case 8:
      dc << "\n\n      MIX CLASS VALIDATION STATUS - FAIL \n";
      break;
    case 9:
      dc << "      SKIP PRIME RBD VALIDATION - CARRIERS DO NOT MATCH\n";
      break;
    case 10:
      dc << "      R6/CONV1 VALIDATION STATUS      - NO MATCH \n";
      break;
    case 11:
      dc << "      R6/CONV1 VALIDATION STATUS      - PASS \n";
      break;
    case 12:
      dc << "      R6/CONV1 VALIDATION STATUS      - FAIL \n";
      break;
    case 13:
      dc << " TABLE 999 ITEM NUMBER FOR ";
      break;
    case 14:
      dc << "      R6/CONV" << convNum;
      break;
    case 15:
      dc << " IS NOT RECOGNIZED\n";
      break;
    case 16:
      dc << "      LOCAL MARKET VALIDATION - FARE TYPE/FAMILY - ";
      break;
    case 17:
      dc << "      STATUS - FAIL, NO PUBLISHED FARES ON MARKET\n";
      break;
    case 18:
      dc << "      TRY PUBLISHED FARE - ";
      break;
    case 19:
      dc << "           FARE TYPE MATCH - ";
      break;
    case 20:
      dc << "         PRIME RBD VALIDATION STATUS - PASS\n"
         << "      LOCAL MARKET VALIDATION STATUS - PASS\n";
      break;
    case 21:
      dc << "        PRIME RBD VALIDATION STATUS - FAIL\n";
      break;
    case 22:
      dc << "           FARE FAMILY - MATCH\n";
      break;
    case 23:
      dc << "      LOCAL MARKET VALIDATION STATUS - FAIL\n"
         << "                   PUBLISHED FARES # - ";
      break;
    case 24:
      dc << "      MIXED CLASS VALIDATION: ";
      break;
    case 25:
      dc << "      MIXED CLASS DATA NOT FOUND FOR CXR - ";
      break;
    case 26:
      dc << "CXR - ";
      break;
    case 27:
      dc << "\n\n        CARRIER NOT THE SAME ON ALL FLIGHTS";
      break;
    case 28:
      dc << "\n        HIERARCHY              BOOKING CODE\n";
      break;
    case 29:
      dc << " *** THERE ARE NO APPLICABLE FARES FOR THIS FARE MARKET ***\n";
      break;
    case 30:
      dc << "      CAT 25 REC 2 CXR DOES NOT MATCH - PRIME RBD FAIL \n";
      break;
    case 31:
      dc << "      NO DISCOUNTED CAT25 FARE EXISTS - PRIME RBD FAIL \n";
      break;
    case 32:
      dc << "      NO PRIME RBD IN RESULTING CAT 25 FARE\n"
         << "      START BASE FARE BKG CODE VALIDATION\n";
      break;
    case 33:
      dc << "      NO PRIME RBD IN CAT 25 DISCOUNT SPECIFIED FARE\n"
         << "      START BASE FARE BKG CODE VALIDATION\n";
      break;
    case 34:
      dc << "\n        FARE BASIS CODE COULD BE CHANGED, GOV CXR - ";
      break;
    case 35:
      dc << "\n         REBOOK COS  - ";
      break;
    case 36:
      dc << "      REC 1 TABLE 990 ITEM NUMBER FOR ";
      break;
    case 37:
      dc << "\n       NO DATA FOUND FOR THE ABOVE ITEM NUMBER";
      break;
    case 38:
      dc << "\n      TABLE 999 VALIDATION STATUS FOR THE FARE - NOMATCH\n";
      break;
    case 39:
      dc << "   FARE TYPE UNRECOGNIZABLE ";
      break;
    case 40:
      dc << "        CANDIDATE TO CHANGE 1-ST CHAR WAS NOT FOUND\n\n";
      break;
    case 41:
      dc << "\n      COMMAND PRICING CHANGE STATUS FROM - FAIL TO - PASS \n";
      break;
    case 42:
      dc << "\n      FARE DID NOT PASS USING RULE 1 TRYING RULE2 NOW \n";
      break;
    case 43:
      dc << "\n      COMMAND PRICING CHANGE STATUS FROM - MIXED TO - PASS \n";
      break;
    case 44:
      dc << "\n      COMMAND PRICING CHANGE STATUS FROM - NOMATCH TO - PASS \n";
      break;
    case 45:
      dc << "\n        1ST CHAR OF THE FARE BASIS WILL BE        - ";
      break;
    case 46:
      dc << "        ";
      break;
    case 47:
      dc << "\n        TABLE 990 EXISTS, USE PRIME RBD TO CHANGE";
      break;
    case 48:
      dc << "\n      NO PRIME RBD IN RESULTING CAT 25 FARE\n"
         << "       GETS PRIME RBD OF THE BASE FARE";
      break;
    case 49:
      dc << "\n  SAME FARE COMPONENT - FOR FLOW FOR LOCAL JOURNEY CARRIER\n";
      break;
    case 50:
      dc << "\n      TABLE 999 VALIDATION STATUS FOR NEG FARE - FAIL\n";
      break;
    case 51:
      dc << "\n      R6/CONV2 VALIDATION STATUS FOR NEG FARE - FAIL\n";
      break;
    case 52:
      dc << "      TABLE 999 REC1 STATUS - PASS \n \n";
      break;
    case 53:
      dc << "      TABLE 999 REC1 STATUS - FAIL \n \n";
      break;
    case 54:
      dc << "      TABLE 999 REC1 STATUS - NOMATCH \n \n";
      break;
    case 55:
      dc << "      FLIGHT PASSED PRIOR TO PRIME RBD VALIDATION \n \n";
      break;
    case 56:
      dc << "      PASS    - ";
      break;
    case 57:
      dc << "      FAIL    - ";
      break;
    case 58:
      dc << "      NOMATCH - ";
      break;
    case 59:
      dc << "      INDUSTRY FARE - ALL SECTORS HAVE CARRIER - YY\n"
         << "      APPLY PRIME RBD\n";
      break;
    case 60:
      dc << "      INDUSTRY FARE - NOT ALL SECTORS HAVE CARRIER - YY\n"
         << "      DO NOT APPLY PRIME RBD\n";
      break;
    case 61:
      dc << "\n      BASE FARE R1/T999 ITEM NUMBER FOR ";
      break;
    case 62:
      dc << "      BKG CODE VALIDATION STATUS CHANGED  - FAIL* \n";
      dc << "          * -  WPA NO MATCH HIGHER CABIN OPTION VALUE:1 \n";
      break;
    case 63:
      dc << "      BKG CODE VALIDATION STATUS CHANGED  - FAIL* \n";
      dc << "          * -  WPA NO MATCH HIGHER CABIN OPTION VALUE:2 \n";
      break;
    case 64:
      dc << "      SKIP LOCAL MARKET PROCESSING SEG STATUS - FAIL \n";
      break;
    case 65:
      dc << "      NO DATA FOR CAT25 FARE EXISTS - PRIME RBD FAIL \n";
      break;
    case 66:
      dc << "      FOR INFANT RASSENGER TYPE\n";
      break;
    case 67:
      dc << "      SKIP    - ";
      break;
    case 68:
      dc << "      CABIN NOT MATCH \n ";
      break;
    case 69:
      dc << "      T999 STATUS FLOW FOR LOCAL JOURNEY: PASS FLOW AVAIL \n ";
      break;
    case 70:
      dc << "      RULE 2, LOCAL AVAILABLE\n ";
      break;
    case 71:
      dc << "      T999 STATUS FLOW FOR LOCAL JOURNEY: FAIL FLOW AVAIL \n ";
      break;
    case 72:
      dc << "      T999 NOT VALID FOR COMMAND PRICING \n ";
      break;

    default:
      dc << "\n  UNKNOWN MSG NUMBER PASSED TO DIAG: " << msg << std::endl;
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag411Collector::operator <<
//
// Description:  This method will sent messages to diag buffer. All messages are in the
//               enum  Diag411Message
//
// @param  segStat - reference to Segment Status param
//
//
// </PRE>
// ----------------------------------------------------------------------------
Diag411Collector&
Diag411Collector::operator << ( const PaxTypeFare::SegmentStatus&  segStat  )
{

  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    dc << "      BKG FOR TRAVEL SEGMENT  - ";

    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOT_YET_PROCESSED))
      dc << "NOT YET PROCESSED \n";
    if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_NOMATCH))
      dc << "NOMATCH \n";
    if ((segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL)) &&
        (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS)))
    {
      dc << "NEED MIX CLASS PROCESS\n";
    }
    else
    {
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_DOMESTIC))
        dc << "FAIL PRIME RBD DOMEST\n";
      else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_PRIME_RBD_INTERNATIONAL))
        dc << "FAIL PRIME RBD INTERL\n";
      else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_T999))
        dc << "FAIL T999 \n";
      else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_REC1_T999))
        dc << "FAIL REC1 T999 \n";
      else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV1_T999))
        dc << "FAIL CONV1 T999 \n";
      else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_CONV2_T999))
        dc << "FAIL CONV2 T999 \n";
      else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_MIXEDCLASS))
        dc << "FAIL MIXEDCLASS \n";
      else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_LOCALMARKET))
        dc << "FAIL LOCALMARKET \n";
      else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL))
        dc << "FAIL \n";
      else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS))
        dc << "PASS \n";
    } // if

    const PricingTrx *pricingTrx = dynamic_cast<const PricingTrx*>(trx());
    if (pricingTrx && pricingTrx->getRequest()->isBrandedFaresRequest())
    {
      if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_OFFER))
        dc << "      IBF ERROR MESSAGE - O\n";
      else if (segStat._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_AVAILABILITY))
        dc << "      IBF ERROR MESSAGE - A\n";
      else
        dc << "      IBF ERROR MESSAGE NOT SET\n";
    }
  }

  return *this;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag411Collector::operator <<
//
// Description:  This method will sent messages to diag buffer. All messages are in the
//               enum  Diag411Message
//
// @param  num - number
//
//
// </PRE>
// ----------------------------------------------------------------------------
Diag411Collector&
Diag411Collector::operator << ( const CarrierMixedClass& mixedClassIter )
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    dc << "          DATES: EFFECTIVE - " << (mixedClassIter).effDate().dateToString(MMDDYYYY, "/")
       << "  EXPIRE - ";

    if ((mixedClassIter).expireDate().isPosInfinity())
    {
      dc << "N/A\n";
    }
    else
    {
      dc << (mixedClassIter).expireDate().dateToString(MMDDYYYY, "/") << "\n";
    }

    if (!(mixedClassIter).discDate().isPosInfinity())
    {
      dc << "               DISCONTINUE - "
         << (mixedClassIter).discDate().dateToString(MMDDYYYY, "/") << "\n";
    }
  }
  return *this;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag411Collector::operator <<
//
// Description:  This method will sent messages to diag buffer. All messages are in the
//               enum  Diag411Message
//
// @param  num - number
//
//
// </PRE>
// ----------------------------------------------------------------------------
void
Diag411Collector::displayHierarchy(char previousHierarchy)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    dc << "\n          " << previousHierarchy << " - ";

    if (previousHierarchy == '1')
    {
      dc << "PREMIUM             - ";
    }
    else if (previousHierarchy == '2')
    {
      dc << "FIRST               - ";
    }
    else if (previousHierarchy == '3')
    {
      dc << "RESTRICTED FIRST    - ";
    }
    else if (previousHierarchy == '4')
    {
      dc << "BUSINESS            - ";
    }
    else if (previousHierarchy == '5')
    {
      dc << "RESTRICTED BUSINESS - ";
    }
    else if (previousHierarchy == '6')
    {
      dc << "COACH               - ";
    }
    else if (previousHierarchy == '7')
    {
      dc << "RESTRICTED COACH    - ";
    }
    else
      dc << "NIGHT               - ";
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag411Collector::operator <<
//
// Description:  This method will sent messages to diag buffer. All messages are in the
//               enum  Diag411Message
//
// @param  num - number
//
//
// </PRE>
// ----------------------------------------------------------------------------
void
Diag411Collector::lineSkip(int num)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    switch (num)
    {
    case 0:
      dc << "*************************************************************** \n";
      break;
    case 1:
      dc << "\n";
      break;
    case 2:
      dc << "\n\n";
      break;
    case 3:
      dc << "\n\n\n";
      break;
    case 4:
      ((std::ostringstream&)*this)
          << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n";
      break;
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag411Collector::operator <<
//
// Description:  This method will sent messages to diag buffer. All messages are in the
//               enum  Diag411Message
//
// @param  num - number
//
//
// </PRE>
// ----------------------------------------------------------------------------
void
Diag411Collector::table999ItemNumber(uint32_t table999ItemNumber)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    dc << " TABLE 999 ITEM NUMBER - ";
    if (table999ItemNumber == 0)
      dc << '0';
    else
    {
      dc << table999ItemNumber << "\n";
    }
  }
}

void
Diag411Collector::displayTagNSegs(const PaxTypeFare& ptf)
{
  typedef std::vector<PaxTypeFare::SegmentStatus>::const_iterator SegmentStatusVecCI;
  typedef std::vector<TravelSeg*>::const_iterator TvlSegVecCI;
  SegmentStatusVecCI segStatIter = ptf.segmentStatus().begin();
  const SegmentStatusVecCI segStatIterEnd = ptf.segmentStatus().end();
  TvlSegVecCI tvlSegIter = ptf.fareMarket()->travelSeg().begin();
  TvlSegVecCI tvlSegIterEnd = ptf.fareMarket()->travelSeg().end();

  bool foundAny = false;
  for (; segStatIter != segStatIterEnd, tvlSegIter != tvlSegIterEnd; ++segStatIter, ++tvlSegIter)
  {
    if ((*segStatIter)._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL_TAG_N))
    {
      if (!foundAny)
      {
        *this << "        *";
        foundAny = true;
      }
      *this << (*tvlSegIter)->origAirport() << (*tvlSegIter)->destAirport() << " ";
    }
  }
  if (foundAny)
  {
    *this << "- TAG N* ";
  }
}
} // namespace tse
