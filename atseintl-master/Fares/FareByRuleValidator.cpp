//-------------------------------------------------------------------
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
//-------------------------------------------------------------------
#include "Fares/FareByRuleValidator.h"

#include "Common/FareDisplayUtil.h"
#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "Diagnostic/Diag325Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Fares/FareByRuleProcessingInfo.h"
#include "Fares/FareTypeMatcher.h"
#include "Rules/RuleApplicationBase.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

#include <iostream>
#include <string>
#include <vector>

namespace tse
{
const char FareByRuleValidator::NO_DISCOUNT = 'X';

//----------------------------------------------------------------------------
// <PRE>
//

// @function int FareByRuleValidator::validate
//
// Description:  To match the Record 3 Category 25 against the user input
//               request. When a match is made, apply either:
//               - A 'No Discount' if the Record 3 Category 25 No Discount
//                 field is populated.
//               - The specified Fare By Rule amount.
//               - A calculated Fare By Rule amount, using Table 989 to locate
//                 the base fares information.
//               Once the fare amount has been determined, it must continue
//               checking the fare comparison fields in the rcord 3. When
//               populated, apply the data in the fare comparison fields to
//               the fare created.
//
// @param  itemNo        A Record 3 Category 25 item number
// @param  vendor        A vendor code
// @param  trx           Transaction info
// @param  itin          Itinerary info
// @param  fareMarket    A reference to the FareMarket object
// @param  fbrApp        A reference to the FareByRuleApp object
//
// @return Record3ReturnTypes
//         PASS - Record 3 Category 25 has passed the validation.
//                Processing should continue to create fare(s) by rule
//                from this Record 3.
//                When return to caller, continue to read next table or set.
//         FAIL - Record 3 Category 25 has failed the validation.
//                No fare will be created from this Record 3.
//                Return to caller to continue to read next table or set.
//         STOP - Record 3 Category 25 has passed the validation
//                with the 'No Discount' indicator on.
//                No fare will be created from this Record 3.
//                Return to caller to end the process, do not read next table or set.
//
// </PRE>
//----------------------------------------------------------------------------
Record3ReturnTypes
FareByRuleValidator::validate(FareByRuleProcessingInfo& fbrProcessingInfo)
{
  const FareByRuleItemInfo* fbrItemInfo = fbrProcessingInfo.fbrItemInfo();
  PricingTrx& trx = *(fbrProcessingInfo.trx());

  if (fbrItemInfo == nullptr)
  {
    if (UNLIKELY(fbrProcessingInfo.diagManager() &&
                 trx.diagnostic().diagnosticType() == Diagnostic325))
    {
      *fbrProcessingInfo.diagManager() << "R3 NOT FOUND\n";
    }
    return FAIL;
  }

  if (!isValid(fbrProcessingInfo))
  {
    return FAIL;
  }

  if (UNLIKELY((*fbrItemInfo).discountInd() == NO_DISCOUNT))
  {
    doDiag325Collector(fbrProcessingInfo, Diag325Collector::NO_DISC);
    return STOP;
  }

  if ((*fbrItemInfo).minMileage() != 0 || (*fbrItemInfo).maxMileage() != 0)
  {
    FareMarket& fareMarket = *fbrProcessingInfo.fareMarket();
    Itin& itin = *fbrProcessingInfo.itin();
    const uint32_t miles = LocUtil::getTPM(*fareMarket.origin(),
                                           *fareMarket.destination(),
                                           fareMarket.getGlobalDirection(),
                                           itin.travelDate(),
                                           trx.dataHandle());
    if (miles < (*fbrItemInfo).minMileage() || miles > (*fbrItemInfo).maxMileage())
    {
      doDiag325Collector(fbrProcessingInfo, Diag325Collector::FAIL_MILEAGE);
      return FAIL;
    }
  }

  if (!checkResultingInfo(fbrProcessingInfo))
  {
    return FAIL;
  }

  doDiag325Collector(fbrProcessingInfo, Diag325Collector::R3_PASS);
  return PASS;
}

//----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleValidator::isValid
//
// Description:  To match the Record 3 Category 25 against the user input
//               request.
//
// @param  fbrItemInfo   A reference to the FareByRuleItemInfo object
// @param  trx           Transaction info
// @param  fareMarket    A reference to the FareMarket object
// @param  fbrApp        A reference to the FareByRuleApp object
//
// @return bool
//        -true, if Record 3 Category 25 is matched.
//        -false otherwise.
//
// </PRE>
//----------------------------------------------------------------------------
bool
FareByRuleValidator::isValid(FareByRuleProcessingInfo& fbrProcessingInfo) const
{
  FareByRuleApp& fbrApp = *fbrProcessingInfo.fbrApp();
  PricingTrx& trx = *fbrProcessingInfo.trx();
  FareMarket& fareMarket = *fbrProcessingInfo.fareMarket();
  const FareByRuleItemInfo* fbrItemInfo = fbrProcessingInfo.fbrItemInfo();

  if (!matchPaxType(*fbrItemInfo, fbrApp))
  {
    doDiag325Collector(fbrProcessingInfo, Diag325Collector::FAIL_PTC);
    return false;
  }

  if (!matchOverrideDateTbl(fbrProcessingInfo))
  {
    doDiag325Collector(fbrProcessingInfo, Diag325Collector::FAIL_TBL994);
    return false;
  }

  if (UNLIKELY(!matchUnavaiTag(*fbrItemInfo)))
  {
    doDiag325Collector(fbrProcessingInfo, Diag325Collector::FAIL_UNAVLTAG);
    return false;
  }

  if (UNLIKELY(!matchPassengerStatus(*fbrItemInfo, trx, fbrProcessingInfo)))
  {
    doDiag325Collector(fbrProcessingInfo, Diag325Collector::FAIL_PAX_STATUS);
    return false;
  }

  if (UNLIKELY(trx.getTrxType() != PricingTrx::MIP_TRX && !matchNbrOfFltSegs(*fbrItemInfo, fareMarket)))
  {
    doDiag325Collector(fbrProcessingInfo, Diag325Collector::FAIL_FLT_SEG_CNT);
    return false;
  }

  if (UNLIKELY(!matchWhollyWithin(trx, *fbrItemInfo, fareMarket)))
  {
    doDiag325Collector(fbrProcessingInfo, Diag325Collector::FAIL_WHOLLY_WITHIN_LOC);
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleValidator::matchPaxType
//
// Description:  To match against the Record 8 primary PTC.
//
// @param  fbrItemInfo   A reference to the FareByRuleItemInfo object
// @param  fbrApp        A reference to the FareByRuleApp object
//
// @return bool
//        -true, if the passenger type is matched.
//        -false otherwise.
//
// </PRE>
//----------------------------------------------------------------------------
bool
FareByRuleValidator::matchPaxType(const FareByRuleItemInfo& fbrItemInfo, FareByRuleApp& fbrApp)
    const
{
  if (fbrItemInfo.paxType() == fbrApp.primePaxType())
  {
    return true;
  }

  return false;
}

//----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleValidator::matchOverrideDateTbl
//
// Description:  The Override Date Table data must match the reservation,
//               ticketing, and/or travel date of the pricing solution for the
//               discount to be applied.
//
// @param  fbrProcessingInfo   A wrapper class for most of the objects needed in FBR
//
// @return bool
//        -true, if the Override Date Table data is matched.
//        -false otherwise.
//
// </PRE>
//----------------------------------------------------------------------------
bool
FareByRuleValidator::matchOverrideDateTbl(FareByRuleProcessingInfo& fbrProcessingInfo) const
{
  const FareByRuleItemInfo& fbrItemInfo = *(fbrProcessingInfo.fbrItemInfo());
  PricingTrx& trx = *(fbrProcessingInfo.trx());

  if (fbrItemInfo.overrideDateTblItemNo() == 0)
  {
    return true;
  }

  DiagCollector* diag = nullptr;
  if (fbrProcessingInfo.diagManager())
    diag = &fbrProcessingInfo.diagManager()->collector();

  // For Cat 25, the validation of the travel date is against the departure date of
  // the first flight of the fare component being priced. The validation of ticketing
  // and reservation dates are against the travel commencement date of the itinerary.
  FareMarket& fareMarket = *fbrProcessingInfo.fareMarket();
  std::vector<TravelSeg*>::const_iterator i = fareMarket.travelSeg().begin();
  DateTime& travelDate = (*i)->departureDT();

  return RuleUtil::validateDateOverrideRuleItem(trx,
                                                fbrItemInfo.overrideDateTblItemNo(),
                                                fbrItemInfo.vendor(),
                                                travelDate,
                                                trx.getRequest()->ticketingDT(),
                                                trx.bookingDate(),
                                                diag,
                                                Diagnostic325);
}

//----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleValidator::matchUnavaiTag
//
// Description:  The indicator that this fare cannot be used for autopricing or
//               that this Record 3 contains only free-form text.
//
// @param  fbrItemInfo   A reference to the FareByRuleItemInfo object
//
// @return bool
//        -false, if incomplete data or text data only.
//        -true otherwise.
//
// </PRE>
//----------------------------------------------------------------------------
bool
FareByRuleValidator::matchUnavaiTag(const FareByRuleItemInfo& fbrItemInfo) const
{
  if (UNLIKELY(fbrItemInfo.unavailtag() == RuleApplicationBase::dataUnavailable ||
      fbrItemInfo.unavailtag() == RuleApplicationBase::textOnly))
  {
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleValidator::matchPassengerStatus
//
// Description:  To match the Record 3 Category 25 against the user input
//
// @param  fbrItemInfo   A reference to the FareByRuleItemInfo object
// @param  trx           Transaction info
//
// @return bool
//        -true, if the Passenger Status is matched.
//        -false otherwise.
//
// </PRE>
//----------------------------------------------------------------------------
bool
FareByRuleValidator::matchPassengerStatus(const FareByRuleItemInfo& fbrItemInfo,
                                          PricingTrx& trx,
                                          FareByRuleProcessingInfo& fbrProcessingInfo) const
{
  FareDisplayUtil fdUtil;
  FareDisplayTrx* fdTrx;

  if (UNLIKELY(fdUtil.getFareDisplayTrx(&trx, fdTrx)))
    return true;

  Indicator paxInd = fbrItemInfo.passengerInd();

  if (LIKELY(paxInd == BLANK))
    return true;

  StateCode stateCode("");
  fbrProcessingInfo.stateCodeFromFirstRequested(stateCode);

  GeoTravelType geoTvlType = fbrProcessingInfo.fareMarket()->geoTravelType();

  if (LocUtil::matchPaxStatus(fbrItemInfo.psgLoc1(),
                              fbrItemInfo.vendor(),
                              paxInd,
                              fbrItemInfo.negPsgstatusInd(),
                              stateCode,
                              trx,
                              geoTvlType))
  {
    if (fbrProcessingInfo.isSpanishPassengerType() &&
        !fbrProcessingInfo.isResidenceFiledInR8() &&
        paxInd == LocUtil::PAX_RESIDENCY &&
        fbrItemInfo.psgLoc1().locType() == LOCTYPE_CITY &&
        LocUtil::isCityInSpanishOverseaIslands(fbrItemInfo.psgLoc1().loc()))
    {
      fbrProcessingInfo.isResidenceFiledInR3Cat25() = true;
    }
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleValidator::matchNbrOfFltSegs
//
// Description:  To indicate the maximum number of flight segments per fare
//               component allowed by the matched Fare by Rule

// @param  fbrItemInfo   A reference to the FareByRuleItemInfo object
// @param  fareMarket    A reference to the FareMarket object
//
// @return bool
//        -true, if the maximum number of flight segments per fare component is matched.
//        -false otherwise.
//
// </PRE>
//----------------------------------------------------------------------------
bool
FareByRuleValidator::matchNbrOfFltSegs(const FareByRuleItemInfo& fbrItemInfo,
                                       FareMarket& fareMarket) const
{
  if (UNLIKELY(fbrItemInfo.fltSegCnt() != 0 &&
      fbrItemInfo.fltSegCnt() < fareMarket.travelSeg().size())) // lint !e574
  {
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleValidator::matchWhollyWithin
//
// Description:  To validate all ticketed points on the fare component must be
//               wholly within a specified location.
//
// @param  fbrItemInfo   A reference to the FareByRuleItemInfo object
// @param  fareMarket    A reference to the FareMarket object
//
// @return bool
//        -true, if Record 3 Category 25 is matched.
//        -false otherwise.
//
// </PRE>
//----------------------------------------------------------------------------
bool
FareByRuleValidator::matchWhollyWithin(PricingTrx& trx,
                                       const FareByRuleItemInfo& fbrItemInfo,
                                       FareMarket& fareMarket) const
{
  LocTypeCode whollyWithinLocType = fbrItemInfo.whollyWithinLoc().locType();

  if (LIKELY(whollyWithinLocType == BLANK))
  {
    return true;
  }

  if (whollyWithinLocType == LOCTYPE_CITY)
  {
    return false;
  }

  LocCode whollyWithinLoc = fbrItemInfo.whollyWithinLoc().loc();
  VendorCode vendor = fbrItemInfo.vendor();

  std::vector<TravelSeg*>::iterator iterTvl = fareMarket.travelSeg().begin();
  std::vector<TravelSeg*>::iterator iterTvlEnd = fareMarket.travelSeg().end();
  GeoTravelType geoTvlType = fareMarket.geoTravelType();

  for (; iterTvl != iterTvlEnd; iterTvl++)
  {
    if (!(LocUtil::isInLoc(*(*iterTvl)->origin(),
                           whollyWithinLocType,
                           whollyWithinLoc,
                           vendor,
                           RESERVED,
                           LocUtil::OTHER,
                           geoTvlType,
                           EMPTY_STRING(),
                           trx.getRequest()->ticketingDT())))
    {
      return false;
    }

    if (!(LocUtil::isInLoc(*(*iterTvl)->destination(),
                           whollyWithinLocType,
                           whollyWithinLoc,
                           vendor,
                           RESERVED,
                           LocUtil::OTHER,
                           geoTvlType,
                           EMPTY_STRING(),
                           trx.getRequest()->ticketingDT())))
    {
      return false;
    }

    const std::vector<const Loc*>& hiddenStops = (*iterTvl)->hiddenStops();
    std::vector<const Loc*>::const_iterator stopIter = hiddenStops.begin();
    for (; stopIter != hiddenStops.end(); stopIter++)
    {
      if (!(LocUtil::isInLoc(**stopIter,
                             whollyWithinLocType,
                             whollyWithinLoc,
                             vendor,
                             RESERVED,
                             LocUtil::OTHER,
                             geoTvlType,
                             EMPTY_STRING(),
                             trx.getRequest()->ticketingDT())))
      {
        return false;
      }
    }
  }

  return true;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleValidator::doDiag325Collector
//
// Description:  To activate the Diag325collector with Record 3 Cat 25 object and
//               reason code for the failed Record 3 Cat 25.
//
// @param  fbrItemInfo   A reference to the FareByRuleItemInfo object
// @param  trx           Transaction info
// @param  failCode      A failed reason code. Value R3_PASS if passes.
//
// </PRE>
// ----------------------------------------------------------------------------
void
FareByRuleValidator::doDiag325Collector(FareByRuleProcessingInfo& fbrProcessingInfo,
                                        const char* failCode) const
{
  const FareByRuleItemInfo& fbrItemInfo = *(fbrProcessingInfo.fbrItemInfo());
  PricingTrx& trx = *(fbrProcessingInfo.trx());
  Itin& itin = *fbrProcessingInfo.itin();

  Diagnostic& trxDiag = trx.diagnostic();

  if (trxDiag.diagnosticType() == Diagnostic325 && fbrProcessingInfo.diagManager())
  {
    DiagManager& diag = *fbrProcessingInfo.diagManager();
    Diag325Collector* diag325 = dynamic_cast<Diag325Collector*>(&diag.collector());
    diag325->diag325Collector(fbrItemInfo, trx, itin, failCode);
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int FareByRuleValidator::checkResultingInfo
//
// Description:  To ensure the Record 3 Category 25 Resulting Fare Information
//               is valid for the Cat 25 fare creation.
//
// @param  fbrProcessingInfo   A wrapper class for most of the objects needed in FBR
//
// @return bool
//        -true, if the resulting fare information is matched
//        -false otherwise.
// </PRE>
// ----------------------------------------------------------------------------
bool
FareByRuleValidator::checkResultingInfo(FareByRuleProcessingInfo& fbrProcessingInfo) const
{
  const FareByRuleItemInfo& fbrItemInfo = *(fbrProcessingInfo.fbrItemInfo());
  FareMarket& fareMarket = *fbrProcessingInfo.fareMarket();
  PricingTrx& trx = *fbrProcessingInfo.trx();

  if (!fbrProcessingInfo.isUsCaRuleTariff() &&
      fbrItemInfo.resultglobalDir() != GlobalDirection::XX &&
      fbrItemInfo.resultglobalDir() != GlobalDirection::ZZ &&
      fareMarket.getGlobalDirection() != GlobalDirection::ZZ &&
      fbrItemInfo.resultglobalDir() != fareMarket.getGlobalDirection())
  {
    doDiag325Collector(fbrProcessingInfo, Diag325Collector::FAIL_GLOBAL);
    return false;
  }

  // check for the NL ( Normal Fare) secondary action code in the pricing entry

  if (UNLIKELY(trx.getOptions()->isNormalFare() && fbrItemInfo.resultpricingcatType() == 'S'))
  {
    doDiag325Collector(fbrProcessingInfo, Diag325Collector::FAIL_PCT);
    return false;
  }

  // check for WPT/{NL,EX,IT} entry
  if (UNLIKELY(trx.getOptions()->isFareFamilyType()))
  {
    FareTypeMatcher fareTypeMatch(trx);
    if (!fareTypeMatch(fbrItemInfo.resultFareType1()))
    {
      doDiag325Collector(fbrProcessingInfo, Diag325Collector::FAIL_PCT);
      return false;
    }
  }

  if ((fbrItemInfo.resultDisplaycatType() == RuleConst::SELLING_FARE ||
       fbrItemInfo.resultDisplaycatType() == RuleConst::NET_SUBMIT_FARE ||
       fbrItemInfo.resultDisplaycatType() == RuleConst::NET_SUBMIT_FARE_UPD) &&
      fbrProcessingInfo.failCat35MSF())
  {
    doDiag325Collector(fbrProcessingInfo, Diag325Collector::FAIL_SECURITY);
    return false;
  }

  if (UNLIKELY(trx.getRequest()->ticketingAgent()->axessUser() &&
      trx.getTrxType() != PricingTrx::FAREDISPLAY_TRX))
  {
    // Do not create Cat25 type L, T or C fare for WP
    if ((fbrItemInfo.resultDisplaycatType() == RuleConst::SELLING_FARE ||
         fbrItemInfo.resultDisplaycatType() == RuleConst::NET_SUBMIT_FARE ||
         fbrItemInfo.resultDisplaycatType() == RuleConst::NET_SUBMIT_FARE_UPD) &&
        !(trx.getRequest()->isWpNettRequested() || trx.getRequest()->isWpSelRequested()))
    {
      doDiag325Collector(fbrProcessingInfo, Diag325Collector::FAIL_CAT35);
      return false;
    }
    // Do not create Cat 25 fare without Cat 35 security for WPNETT/WPSEL
    else if ((trx.getRequest()->isWpNettRequested() || trx.getRequest()->isWpSelRequested()) &&
             !(fbrItemInfo.resultDisplaycatType() == BLANK ||
               fbrItemInfo.resultDisplaycatType() == RuleConst::SELLING_FARE ||
               fbrItemInfo.resultDisplaycatType() == RuleConst::NET_SUBMIT_FARE ||
               fbrItemInfo.resultDisplaycatType() == RuleConst::NET_SUBMIT_FARE_UPD))
    {
      doDiag325Collector(fbrProcessingInfo, Diag325Collector::FAIL_NON_CAT35);
      return false;
    }
  }
  return true;
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
bool
FareByRuleValidator::checkSameTariffRule(const PricingUnit& pricingUnit)
{
  std::vector<const PaxTypeFare*> paxTypeFares;

  std::vector<FareUsage*>::const_iterator fuIt = pricingUnit.fareUsage().begin();
  const std::vector<FareUsage*>::const_iterator fuItEnd = pricingUnit.fareUsage().end();

  for (; fuIt != fuItEnd; fuIt++)
  {
    paxTypeFares.push_back((*fuIt)->paxTypeFare());
  }

  return checkSameTariffRule(paxTypeFares);
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
bool
FareByRuleValidator::checkSameTariffRule(const FarePath& farePath)
{
  std::vector<const PaxTypeFare*> paxTypeFares;

  std::vector<PricingUnit*>::const_iterator puIt = farePath.pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator puItEnd = farePath.pricingUnit().end();

  for (; puIt != puItEnd; puIt++)
  {
    PricingUnit& pricingUnit = **puIt;

    std::vector<FareUsage*>::const_iterator fuIt = pricingUnit.fareUsage().begin();
    const std::vector<FareUsage*>::const_iterator fuItEnd = pricingUnit.fareUsage().end();

    for (; fuIt != fuItEnd; fuIt++)
    {
      paxTypeFares.push_back((*fuIt)->paxTypeFare());
    }
  }

  return checkSameTariffRule(paxTypeFares);
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
bool
FareByRuleValidator::checkSameTariffRule(const std::vector<const PaxTypeFare*>& paxTypeFares)
{
  bool reqSameRule = false;
  bool reqSameTariff = false;
  bool reqPublicTariff = false;
  bool reqPrivateTariff = false;

  if (UNLIKELY(!getRuleTariffRestrictions(
          paxTypeFares, reqSameRule, reqSameTariff, reqPublicTariff, reqPrivateTariff)))
  {
    return false;
  }

  if (UNLIKELY(reqSameTariff && !RuleUtil::useSameTariffRule(paxTypeFares, reqSameRule)))
    return false;

  if (UNLIKELY(reqPublicTariff || reqPrivateTariff))
  {
    std::vector<const PaxTypeFare*> privateTariffPaxTypeFares;
    std::vector<const PaxTypeFare*> publicTariffPaxTypeFares;

    std::vector<const PaxTypeFare*>::const_iterator ptfIt = paxTypeFares.begin();
    const std::vector<const PaxTypeFare*>::const_iterator ptfItEnd = paxTypeFares.end();

    for (; ptfIt != ptfItEnd; ptfIt++)
    {
      if ((*ptfIt)->tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
      {
        if (reqPublicTariff)
          privateTariffPaxTypeFares.push_back(*ptfIt);
      }
      else
      {
        if (reqPrivateTariff)
          publicTariffPaxTypeFares.push_back(*ptfIt);
      }
    }

    const uint16_t numOfPrivateTariffPTF = privateTariffPaxTypeFares.size();
    const uint16_t numOfPublicTariffPTF = publicTariffPaxTypeFares.size();

    ptfIt = paxTypeFares.begin();
    for (; ptfIt != ptfItEnd; ptfIt++)
    {
      if (!(*ptfIt)->isFareByRule())
        continue;

      Indicator restrictAppl = BLANK;

      try
      {
        const FareByRuleItemInfo& fbrInfo = (*ptfIt)->fareByRuleInfo();
        restrictAppl = fbrInfo.sameTariffRule();
      }
      catch (...) { continue; }

      if (restrictAppl == BLANK)
        continue;

      if (restrictAppl == COMBINE_PUBLIC_TARIFF_FARES)
      {
        if (numOfPrivateTariffPTF == 0)
          continue;
        else if (numOfPrivateTariffPTF == 1)
        {
          if (privateTariffPaxTypeFares.front() == (*ptfIt))
            continue;
        }

        return false;
      }
      else if (restrictAppl == COMBINE_PRIVATE_TARIFF_FARES)
      {
        if (numOfPublicTariffPTF == 0)
          continue;
        else if (numOfPublicTariffPTF == 1)
        {
          if (publicTariffPaxTypeFares.front() == (*ptfIt))
            continue;
        }

        return false;
      }
    }
  }

  return true;
}

//---------------------------------------------------------------
//
//---------------------------------------------------------------
bool
FareByRuleValidator::getRuleTariffRestrictions(const std::vector<const PaxTypeFare*>& paxTypeFares,
                                               bool& reqSameRule,
                                               bool& reqSameTariff,
                                               bool& reqPublicTariff,
                                               bool& reqPrivateTariff)
{
  std::vector<const PaxTypeFare*>::const_iterator ptfIt = paxTypeFares.begin();
  const std::vector<const PaxTypeFare*>::const_iterator ptfItEnd = paxTypeFares.end();

  for (; ptfIt != ptfItEnd; ptfIt++)
  {
    if ((*ptfIt)->isFareByRule())
    {
      Indicator restrictAppl = BLANK;

      try
      {
        const FareByRuleItemInfo& fbrInfo = (*ptfIt)->fareByRuleInfo();
        restrictAppl = fbrInfo.sameTariffRule();
      }
      catch (...) { continue; }

      if (LIKELY(restrictAppl == BLANK))
        continue;

      if (restrictAppl == COMBINE_SAME_TARIFF_FARES)
      {
        reqSameTariff = true;
      }
      else if (restrictAppl == COMBINE_SAME_TARIFF_AND_RULE_FARES)
      {
        reqSameTariff = true;
        reqSameRule = true;
      }
      else if (restrictAppl == COMBINE_PRIVATE_TARIFF_FARES)
      {
        reqPrivateTariff = true;
      }
      else if (restrictAppl == COMBINE_PUBLIC_TARIFF_FARES)
      {
        reqPublicTariff = true;
      }
    }
  }

  return true;
}
}
