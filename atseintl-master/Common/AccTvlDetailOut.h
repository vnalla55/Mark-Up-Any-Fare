//-------------------------------------------------------------------
//
//  File:        AccTvlDetailOut.h
//  Created:     Feb 10, 2006
//  Authors:     Simon Li
//
//  Updates:
//
//  Description: Object storing all data of FarePath needed for accompanied
//          travel restriction during group fare path validation
//
//  Copyright Sabre 2006
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

#pragma once

#include "Common/PaxTypeUtil.h"
#include "DataModel/AccTvlChkList.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/AccompaniedTravelInfo.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/DiscountInfo.h"
#include "FareCalc/CalcTotals.h"
#include "Rules/AccompaniedTravel.h"
#include "Rules/RuleConst.h"

namespace tse
{
class FareMarket;

template <class outputC>
class AccTvlDetailOut
{
public:
  enum MessageType
  { WPA_MSG_TYPE,
    WP_MSG_TYPE,
    WPN_MSG_TYPE };

  AccTvlDetailOut(char specialSeperator = ' ')
    : SpecialSeperator(specialSeperator),
      _collectTrailerMsg(true),
      _displayFareBreak(false),
      _reqAccTvl(false),
      _warnAccTvl(false),
      _tktGuaranteed(true),
      _accTvlNotValidated(false),
      _farePath(nullptr),
      _msgType(WPA_MSG_TYPE)
  {
  }

  bool& collectTrailerMsg() { return _collectTrailerMsg; }
  const bool& collectTrailerMsg() const { return _collectTrailerMsg; }

  bool& reqAccTvl() { return _reqAccTvl; }
  const bool& reqAccTvl() const { return _reqAccTvl; }

  bool& warnAccTvl() { return _warnAccTvl; }
  const bool& warnAccTvl() const { return _warnAccTvl; }

  bool& tktGuaranteed() { return _tktGuaranteed; }
  const bool& tktGuaranteed() const { return _tktGuaranteed; }

  MessageType& msgType() { return _msgType; }
  const MessageType& msgType() const { return _msgType; }

  // functions to store accompanied restriciton
  void storeAccTvlDetail(PricingTrx* trx,
                         outputC& outputObj,
                         const PaxTypeCode& truePaxType,
                         const FarePath& farePath)
  {
    if (_msgType == WPA_MSG_TYPE)
    {
      _collectTrailerMsg = false; // use one message for all of same PTC
    }
    else
    {
      _collectTrailerMsg = true;
    }
    _displayFareBreak = false;
    _farePath = &farePath;

    _truePaxType = truePaxType;
    outputObj << truePaxType << Seperator;
    storeAccTvlDetail(outputObj, *farePath.paxType());

    // gather all the paxTypeFares by the order of their starting segment
    std::map<uint16_t, PaxTypeFare*> ptfPool;

    std::vector<PricingUnit*>::const_iterator puIter = farePath.pricingUnit().begin();
    const std::vector<PricingUnit*>::const_iterator puIterEnd = farePath.pricingUnit().end();

    for (; puIter != puIterEnd; puIter++)
    {
      PricingUnit& pu = **puIter;

      std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
      const std::vector<FareUsage*>::const_iterator fuIterEnd = pu.fareUsage().end();

      for (; fuIter != fuIterEnd; fuIter++)
      {
        PaxTypeFare* paxTypeFare = (*fuIter)->paxTypeFare();
        uint16_t startSegNo = paxTypeFare->fareMarket()->travelSeg().front()->pnrSegment();
        // startSegNo is not required as accrate as Itin segmentOrder
        ptfPool[startSegNo] = paxTypeFare;
        if (paxTypeFare->isAccTvlWarning())
        {
          _warnAccTvl = true;
        }
      }
    }

    const uint16_t numOfFareBreak = ptfPool.size();
    outputObj << numOfFareBreak;
    if (_msgType != WPA_MSG_TYPE && (numOfFareBreak > 1))
    {
      _displayFareBreak = true;
    }
    else
    {
      _displayFareBreak = false;
    }

    // write detail required for accompanied travel restriction
    // for all fare usages
    std::map<uint16_t, PaxTypeFare*>::const_iterator ptfMapI = ptfPool.begin();
    const std::map<uint16_t, PaxTypeFare*>::const_iterator ptfMapIEnd = ptfPool.end();

    for (; ptfMapI != ptfMapIEnd; ptfMapI++)
    {
      storeAccTvlDetail(trx, outputObj, *(ptfMapI->second));
    }
    // If every segment need rule and use same rule, do not display SEG
    if (_displayFareBreak && _trailerMsgMap.size() == ptfPool.size())
    {
      const std::string& msg = _trailerMsgMap.begin()->first;
      const PtfMsgMapCI upperIter = _trailerMsgMap.upper_bound(msg);
      if (upperIter == _trailerMsgMap.end())
        _displayFareBreak = false;
    }

    if (_accTvlNotValidated)
      _warnAccTvl = true; // so to append trailer message
  }

  void printFormat(outputC& outputObj)
  {
    outputObj << "ACC TVL DATA FORMAT\nPAXNUM TYPE AGE NUMOFFAREBREAK\nSEG-FROM SEG-TO CABIN "
                 "FARECLASS RULENUM TARIFF PRIME-BKGCODE R FLAGS NUMOFOPT NUMOF-FARECLASS-BKGCODE "
                 "FARECLASSES/BKGCODES NUMOF-PSGTYPE PSGTYPES\n";
  }

  void appendTrailerMsg(CalcTotals& calcTotals)
  {
    if (_accTvlNotValidated)
      calcTotals.fcMessage.push_back(
          FcMessage(FcMessage::WARNING, 0, "SOME ACCOMPANIED TRAVEL RESTICTIONS NOT VALIDATED -"));

    PtfMsgMapCI msgMapI = _trailerMsgMap.begin();
    const PtfMsgMapCI msgMapIEnd = _trailerMsgMap.end();
    for (; msgMapI != msgMapIEnd;)
    {
      std::ostringstream tmpOStr;
      const std::string& msg = msgMapI->first;
      PtfMsgMapCI upperIter = _trailerMsgMap.upper_bound(msg);

      if (_displayFareBreak)
      {
        tmpOStr << "SEG";
        printSegmentOrder(tmpOStr, *(msgMapI->second));

        msgMapI++;

        for (; msgMapI != upperIter; msgMapI++)
        {
          tmpOStr << ",";
          printSegmentOrder(tmpOStr, *(msgMapI->second));
        }
        tmpOStr << " ";
      }
      else
      {
        msgMapI = upperIter;
      }
      tmpOStr << msg;
      // calcTotals.ptcWarningMessage.push_back(tmpOStr.str());
      calcTotals.fcMessage.push_back(FcMessage(FcMessage::WARNING, 0, tmpOStr.str()));
    }
  }

  typedef std::multimap<const std::string, const PaxTypeFare*> PtfMsgMap;
  typedef std::multimap<const std::string, const PaxTypeFare*>::const_iterator PtfMsgMapCI;

protected:
  void storeAccTvlDetail(outputC& outputObj, const PaxType& paxType)
  {
    outputObj << paxType.number() << Seperator << paxType.paxType() << Seperator << paxType.age()
              << Seperator;
  }

  const AccompaniedTravelInfo* getAccTvlInfo(PricingTrx* trx, const RuleItemInfo* ruleItemInfo)
  {
    if (ruleItemInfo)
    {
      const AccompaniedTravelInfo* accTvlInfo =
          dynamic_cast<const AccompaniedTravelInfo*>(ruleItemInfo);

      if (accTvlInfo)
      {
        PaxTypeCode accPsgTypeCode = accTvlInfo->accPsgType();
        if (!accPsgTypeCode.empty())
        {
          std::vector<PaxTypeCode> paxTypeCodes;
          paxTypeCodes.push_back(accPsgTypeCode);

          if (PaxTypeUtil::isPaxInTrx(*trx, paxTypeCodes))
            return accTvlInfo;
        }
      }
    }

    return nullptr;
  }

  void getAccompaniedTravelInfoList(PricingTrx* trx,
                                    const CategoryRuleInfo* categoryRuleInfo,
                                    std::vector<const AccompaniedTravelInfo*>& accTvlInfoList)
  {
    if (categoryRuleInfo)
    {
      for (const auto& setPtr: categoryRuleInfo->categoryRuleItemInfoSet())
      {
        for (const auto& item: *setPtr)
        {
          if ((item.itemcat() == RuleConst::ACCOMPANIED_PSG_RULE) &&
              ((item.relationalInd() == CategoryRuleItemInfo::THEN) ||
               (item.relationalInd() == CategoryRuleItemInfo::OR)))
          {
            const AccompaniedTravelInfo* accTvlInfo =
                getAccTvlInfo(trx, trx->dataHandle().getRuleItemInfo(categoryRuleInfo, &item));

            if (accTvlInfo)
              accTvlInfoList.push_back(accTvlInfo);
          }
        }
      }
    }
  }

  void storeAccTvlDetail(PricingTrx* trx, outputC& outputObj, const PaxTypeFare& paxTypeFare)
  {
    const FareMarket& fm = *paxTypeFare.fareMarket();
    outputObj << SpecialSeperator;
    outputObj << fm.travelSeg().front()->pnrSegment() << Seperator
              << fm.travelSeg().back()->pnrSegment() << Seperator << paxTypeFare.cabin()
              << Seperator << paxTypeFare.fareClass() << Seperator;

    // RuleNumber and RuleTariff
    outputObj << paxTypeFare.ruleNumber() << Seperator << paxTypeFare.tcrRuleTariff() << Seperator;

    // Booking Code(s)
    // Get booking code on Primary Sector
    const TravelSeg* primarySec = fm.primarySector();
    BookingCode bc = "-";
    if (primarySec && !primarySec->getBookingCode().empty())
    {
      bc = primarySec->getBookingCode();
    }

    uint16_t indx = 0;

    const PaxTypeFare::SegmentStatusVec& statusSVec = paxTypeFare.segmentStatus();

    if (LIKELY(statusSVec.size() == fm.travelSeg().size()))
    {
      std::vector<TravelSeg*>::const_iterator tvlSegIter = fm.travelSeg().begin();
      const std::vector<TravelSeg*>::const_iterator tvlSegIterE = fm.travelSeg().end();

      // check if we have new booking code
      for (; tvlSegIter != tvlSegIterE; ++tvlSegIter, ++indx)
      {
        if (*tvlSegIter == primarySec)
        {
          if (statusSVec[indx]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
          {
            bc = statusSVec[indx]._bkgCodeReBook;
          }

          break;
        }
      }
    }
    outputObj << bc << Seperator;

    const bool isAccTvlWarning = paxTypeFare.isAccTvlWarning();

    if (UNLIKELY(paxTypeFare.isAccTvlNotValidated()))
      _accTvlNotValidated = true;

    if (paxTypeFare.needAccSameFareBreak() || paxTypeFare.needAccSameCabin() ||
        paxTypeFare.isAccTvlNotValidated() || isAccTvlWarning)
    {
      std::ostringstream trailerMsg;

      //---------------------------------------------------------
      // See if there is Cat13 rule
      //---------------------------------------------------------
      PaxTypeFareRuleData* paxTypeFareRuleData =
          paxTypeFare.paxTypeFareRuleData(RuleConst::ACCOMPANIED_PSG_RULE);
      // do not collect cat13 rule once _warnAccTvl is set, (cat19 price
      // w/o adult)
      if (paxTypeFareRuleData != nullptr && !_warnAccTvl)
      {
        // Put the cat13 that has passed to the front
        uint32_t passedItemNo = 0;
        const AccompaniedTravelInfo* passedAccTvlInfo =
            dynamic_cast<const AccompaniedTravelInfo*>(paxTypeFareRuleData->ruleItemInfo());

        if (passedAccTvlInfo)
        {
          storeAccTvlDetail(outputObj, *passedAccTvlInfo);
          passedItemNo = passedAccTvlInfo->itemNo();

          _reqAccTvl = true;
          if (!_accTvlNotValidated) // exchange trx, still guarantee tkt
            _tktGuaranteed = false;

          if (_collectTrailerMsg)
          {
            trailerMsg << "EACH " << _truePaxType << " REQUIRES ACCOMPANYING";
            AccompaniedTravel::collectTrailerMsg(trailerMsg, *passedAccTvlInfo);
          }
        }

        // put the rest of cat13 info because we can't know in advance which will pass during WPA*
        std::vector<const AccompaniedTravelInfo*> accTvlInfoList;
        getAccompaniedTravelInfoList(trx, paxTypeFareRuleData->categoryRuleInfo(), accTvlInfoList);

        std::vector<const AccompaniedTravelInfo*>::const_iterator i = accTvlInfoList.begin();
        for (; i != accTvlInfoList.end(); i++)
        {
          const AccompaniedTravelInfo* accTvlInfo = *i;
          if (accTvlInfo->itemNo() != passedItemNo)
            storeAccTvlDetail(outputObj, *accTvlInfo);
        }
      }

      //---------------------------------------------------------
      // See if there is Cat19 rule with accompanied restriction
      //---------------------------------------------------------
      if (paxTypeFare.isDiscounted())
      {
        try
        {
          const DiscountInfo& discInfo(paxTypeFare.discountInfo());
          if (discInfo.accInd() == ' ' && // require Accompanied Travel
              discInfo.segs().size() != 0)
          {
            storeAccTvlDetail(outputObj, discInfo);
            _reqAccTvl = true;
            // to guarantee Price alone, keep tktGuaranteed true

            if (_collectTrailerMsg)
            {
              trailerMsg << "EACH " << _truePaxType << " REQUIRES ACCOMPANYING";
              AccompaniedTravel::collectTrailerMsg(trailerMsg, discInfo);
            }
          }
        }
        catch (...)
        {
          // no valid discount info
        }
      }

      // New memory for string will be allocated by multimap insert
      if (!trailerMsg.str().empty())
      {
        _trailerMsgMap.insert(
            std::pair<const std::string, const PaxTypeFare*>(trailerMsg.str(), &paxTypeFare));
      }
    }
  }

  uint32_t getAccompaniedPaxNumber(const PaxType* accompaniedPaxType,
                                   const AccompaniedTravelInfo& accTvlInfo)
  {
    if ((accompaniedPaxType->number() <= 1) || (accTvlInfo.minNoPsg() == 0) ||
        (accompaniedPaxType->paxType() != accTvlInfo.accPsgType()))
      return accompaniedPaxType->number();

    // Number of passengers is more than 1, and requires same paxtype, so this is group travel
    return 1;
  }

  void storeAccTvlDetail(outputC& outputObj, const AccompaniedTravelInfo& accTvlInfo)
  {
    outputObj << RuleHead << Seperator;

    AccTvlChkList chkList;
    if (accTvlInfo.accTvlSameCpmt() != AccompaniedTravel::notApply)
    {
      chkList.setChkSameCpmt();
    }
    if (accTvlInfo.accTvlSameRule() != AccompaniedTravel::notApply)
    {
      chkList.setChkSameRule();
    }
    if (accTvlInfo.accPsgAppl() != AccompaniedTravel::applyMust)
    {
      chkList.setPsgNegAppl();
    }
    if (accTvlInfo.fareClassBkgCdInd() != AccompaniedTravel::fareClass)
    {
      chkList.setChkBkgCode();
    }
    chkList.setChkNumPsg();

    uint32_t accompaniedPaxNumber = getAccompaniedPaxNumber(_farePath->paxType(), accTvlInfo);
    const uint32_t minNoAccPsg =
        AccompaniedTravel::minNoAccPsgReq(accompaniedPaxNumber, accTvlInfo);

    const uint16_t chkFlags = chkList.value();
    outputObj << chkFlags << Seperator << minNoAccPsg << Seperator << "1" << Seperator;
    // Always 1 numOfOpt for cat13, >=1 discountSegInfo for cat19

    // FareClass or BookingCode
    outputObj << accTvlInfo.fareClassBkgCds().size() << Seperator;
    std::vector<FareClassCode>::const_iterator fcBcIter = accTvlInfo.fareClassBkgCds().begin();
    const std::vector<FareClassCode>::const_iterator fcBcIterEnd =
        accTvlInfo.fareClassBkgCds().end();

    for (; fcBcIter != fcBcIterEnd; fcBcIter++)
    {
      outputObj << *fcBcIter << Seperator;
    }

    // Accompanied Passenger Types
    uint16_t numOfPsgTypes = 0;
    if (accTvlInfo.accPsgType().empty())
    {
      outputObj << numOfPsgTypes;
    }
    else
    {
      numOfPsgTypes++;
      outputObj << numOfPsgTypes << Seperator << accTvlInfo.accPsgType() << Seperator;
    }
  }

  void storeAccTvlDetail(outputC& outputObj, const DiscountInfo& discInfo)
  {

    AccTvlChkList chkList;
    if (discInfo.accTvlSameCpmt() != AccompaniedTravel::notApply)
    {
      chkList.setChkSameCpmt();
    }
    if (discInfo.accTvlSameRule() != AccompaniedTravel::notApply)
    {
      chkList.setChkSameRule();
    }
    chkList.setTktGuaranteed(); // always guarantee cat19 fares

    const uint16_t chkFlags = chkList.value();

    outputObj << RuleHead << Seperator;
    outputObj << chkFlags << Seperator;
    outputObj << discInfo.segs().size() << Seperator;

    // Read all DiscountSegInfo for FareClass or BookingCode
    // and Accompanied Passenger Type
    uint16_t numOfFareClassBkgCds = 0;
    if (discInfo.fareClassBkgCodeInd() != AccompaniedTravel::notApply)
    {
      numOfFareClassBkgCds = 1;
    }

    std::vector<DiscountSegInfo*>::const_iterator discSegI = discInfo.segs().begin();
    const std::vector<DiscountSegInfo*>::const_iterator discSegIEnd = discInfo.segs().end();

    for (; discSegI != discSegIEnd; discSegI++)
    {
      const DiscountSegInfo& segInfo = **discSegI;

      outputObj << numOfFareClassBkgCds << Seperator;
      if (discInfo.fareClassBkgCodeInd() == AccompaniedTravel::fareClass)
      {
        if (segInfo.fareClass().empty())
        {
          outputObj << "-" << Seperator;
        }
        else
        {
          outputObj << segInfo.fareClass() << Seperator;
        }
      }
      else if (discInfo.fareClassBkgCodeInd() == AccompaniedTravel::bookingCode)
      {
        if (segInfo.bookingCode().empty())
        {
          outputObj << "-" << Seperator;
        }
        else
        {
          outputObj << segInfo.bookingCode() << Seperator;
        }
      }

      uint16_t numOfPsgType = 1;
      if (!segInfo.accPsgType2().empty())
        numOfPsgType++;
      if (!segInfo.accPsgType3().empty())
        numOfPsgType++;

      outputObj << numOfPsgType << Seperator;

      if (segInfo.accPsgType1().empty())
      {
        outputObj << "ADT" << Seperator;
      }
      else
      {
        outputObj << segInfo.accPsgType1() << Seperator;
      }

      if (!segInfo.accPsgType2().empty())
      {
        outputObj << segInfo.accPsgType2() << Seperator;
      }
      if (!segInfo.accPsgType3().empty())
      {
        outputObj << segInfo.accPsgType3() << Seperator;
      }
    }
  }

  void printSegmentOrder(std::ostringstream& os, const PaxTypeFare& paxTypeFare)
  {
    std::vector<TravelSeg*>::const_iterator tvlSegI = paxTypeFare.fareMarket()->travelSeg().begin();
    const std::vector<TravelSeg*>::const_iterator tvlSegIEnd =
        paxTypeFare.fareMarket()->travelSeg().end();

    std::vector<TravelSeg*>::const_iterator tvlSegIBegin = tvlSegI;

    for (; tvlSegI != tvlSegIEnd; tvlSegI++)
    {
      if ((*tvlSegI)->segmentType() == Arunk)
        continue;

      if (tvlSegI != tvlSegIBegin)
      {
        os << ",";
      }
      os << (*tvlSegI)->pnrSegment();
    }
  }

  char SpecialSeperator;

  static constexpr char Seperator = ' ';
  static constexpr char RuleHead = 'R';

private:
  bool _collectTrailerMsg;
  bool _displayFareBreak;
  bool _reqAccTvl;
  bool _warnAccTvl;
  bool _tktGuaranteed;
  bool _accTvlNotValidated;
  const FarePath* _farePath;
  MessageType _msgType;
  PaxTypeCode _truePaxType;
  PtfMsgMap _trailerMsgMap;
};

} // tse namespace

