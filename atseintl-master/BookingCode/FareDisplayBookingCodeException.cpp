//-------------------------------------------------------------------
//
//  File: FareDisplayBookingCodeException
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "BookingCode/FareDisplayBookingCodeException.h"

#include "BookingCode/RBData.h"
#include "BookingCode/RBDataItem.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FareDisplayUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/PaxTypeFareRuleDataCast.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/BookingCodeExceptionSegment.h"
#include "DBAccess/BookingCodeExceptionSequenceList.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "Rules/RuleUtil.h"

#include <vector>

namespace tse
{
static Logger
logger("atseintl.BookingCode.FareDisplayBookingCodeException");

namespace
{
ConfigurableValue<bool>
useBKGExceptionIndex("FARESV_SVC", "USE_BOOKINGCODEEXCEPTION_INDEX");
}

const CarrierCode FareDisplayBookingCodeException::BCE_DOLLARDOLLARCARRIER = "$$";
const CarrierCode FareDisplayBookingCodeException::BCE_XDOLLARCARRIER = "X$";
const CarrierCode FareDisplayBookingCodeException::BCE_ANYCARRIER = "";

const EquipmentType FareDisplayBookingCodeException::BCE_EQUIPBLANK = "";

FareDisplayBookingCodeException::FareDisplayBookingCodeException()
{
  setUseBKGExceptionIndex();
}

FareDisplayBookingCodeException::FareDisplayBookingCodeException(FareDisplayTrx* trx,
                                                                 PaxTypeFare* ptf,
                                                                 RBData* rbData)
  : _trx(trx), _paxTypeFare(ptf), _rbData(rbData)
{
  setUseBKGExceptionIndex();
}

void
FareDisplayBookingCodeException::setUseBKGExceptionIndex()
{
  _useBKGExceptionIndex = useBKGExceptionIndex.getValue();
}

bool
FareDisplayBookingCodeException::getBookingCodeException(std::vector<BookingCode>& bkgCodes)
{
  LOG4CXX_DEBUG(logger,
                "\n Start FareDisplayBookingCodeException::getBookingCodeException() - vendor:"
                    << _paxTypeFare->vendor()
                    << " bkgCodeItemNo:" << _paxTypeFare->fcasBookingCodeTblItemNo());

  if (isRBRequest() && _rbData != nullptr && _rbData->isSecondary())
  {
    bkgCodes.clear();
  }

  const BookingCodeExceptionSequenceList& lists = getBookingCodeExceptionSequence(
      _paxTypeFare->vendor(), _paxTypeFare->fcasBookingCodeTblItemNo());

  if (!lists.empty())
  {
    VendorCode& vendorCode = (VendorCode&)_paxTypeFare->vendor();
    processTable999Seq(lists, bkgCodes, true, vendorCode);
    if (bkgCodes.empty())
    {
      return false;
    }
    return true;
  }
  else
  {
    return false;
  }
}

bool
FareDisplayBookingCodeException::getBookingCodeException(const VendorCode& vendorCode,
                                                         std::vector<BookingCode>& bkgCodes,
                                                         const AirSeg* airSeg,
                                                         bool convention2)
{
  const DateTime& travelDate = airSeg->departureDT();
  CarrierCode cxr = _paxTypeFare->fareMarket()->governingCarrier();

  // SPR105891 For FRB, we pass FBR fare, but we use Base Fare
  // to get booking code exception for Convention2
  int cat25 = 25;
  const PaxTypeFareRuleData* fbrPTFare = _paxTypeFare->paxTypeFareRuleData(cat25);
  PaxTypeFare* basePaxTypeFare = _paxTypeFare;

  bool isFbrSmf(false);
  if (fbrPTFare)
  {
    const FBRPaxTypeFareRuleData* fbrPTFBaseFare = PTFRuleData::toFBRPaxTypeFare(fbrPTFare);
    TSE_ASSERT(fbrPTFBaseFare);
    if (!fbrPTFBaseFare->isSpecifiedFare())
    {
      if (!(_paxTypeFare->vendor() == ATPCO_VENDOR_CODE ||
            _paxTypeFare->vendor() == SITA_VENDOR_CODE))
      {
        const FareByRuleItemInfo* fbrItemInfo =
            dynamic_cast<const FareByRuleItemInfo*>(fbrPTFBaseFare->ruleItemInfo());
        if (fbrItemInfo != nullptr && fbrItemInfo->primeSector() == 'X')
          isFbrSmf = true;
      }
      basePaxTypeFare = fbrPTFBaseFare->baseFare();
      if (basePaxTypeFare == nullptr)
        basePaxTypeFare = _paxTypeFare;
    }
  }
  bool isSmf(false);
  if (!(_paxTypeFare->vendor() == ATPCO_VENDOR_CODE || _paxTypeFare->vendor() == SITA_VENDOR_CODE))
    isSmf = true;

  if (isRBRequest() && _rbData != nullptr && _rbData->isSecondary())
  {
    bkgCodes.clear();
    airSeg = _rbData->airSeg();
    cxr = airSeg->carrier();
    LOG4CXX_DEBUG(logger,
                  "\n Start FareDisplayBookingCodeException::getBookingCodeException() - Secondary "
                  "-- carrierCode="
                      << cxr << " Rule=" << _paxTypeFare->fare()->ruleNumber()
                      << " Tariff=" << _paxTypeFare->fare()->tcrRuleTariff());
  }
  else
  {
    LOG4CXX_DEBUG(logger,
                  "\n Start FareDisplayBookingCodeException::getBookingCodeException() - Primary "
                  "-- carrierCode="
                      << cxr << " Rule=" << basePaxTypeFare->fare()->ruleNumber()
                      << " Tariff=" << basePaxTypeFare->fare()->tcrRuleTariff());
  }

  if (convention2)
  {
    LOG4CXX_DEBUG(logger, "\n ------- CONVENTION 2 --------");
    const BookingCodeExceptionSequenceList& lists =
        isFbrSmf ? getBookingCodeExceptionSequence(_paxTypeFare->vendor(),
                                                   _paxTypeFare->carrier(),
                                                   _paxTypeFare->tcrRuleTariff(),
                                                   _paxTypeFare->ruleNumber(),
                                                   CONV_2,
                                                   travelDate)
                 : getBookingCodeExceptionSequence(vendorCode,
                                                   basePaxTypeFare->fare()->carrier(),
                                                   basePaxTypeFare->fare()->tcrRuleTariff(),
                                                   basePaxTypeFare->fare()->ruleNumber(),
                                                   CONV_2,
                                                   travelDate);

    if (!lists.empty())
    {
      processTable999Seq(
          lists, bkgCodes, convention2, isFbrSmf ? _paxTypeFare->vendor() : vendorCode);
    }
    // If booking code exception not found
    if (bkgCodes.empty() && isRBRequest() && _rbData != nullptr && _rbData->isSecondary() &&
        (cxr != _paxTypeFare->carrier() || _paxTypeFare->fare()->isIndustry()))
    {
      if (isFbrSmf || isSmf)
      {
        const BookingCodeExceptionSequenceList& lists =
            getBookingCodeExceptionSequence(_paxTypeFare->vendor(),
                                            _paxTypeFare->carrier(),
                                            _paxTypeFare->tcrRuleTariff(),
                                            "0000",
                                            CONV_2,
                                            travelDate);
        if (!lists.empty())
        {
          bkgCodes.clear();
          processTable999Seq(lists, bkgCodes, true, _paxTypeFare->vendor());
        }
      }
      else
      {
        VendorCode vndCd = (vendorCode.equalToConst("SITA")) ? "ATP" : vendorCode;

        LOG4CXX_DEBUG(logger, "\n ------- CONVENTION 1 ------- " << cxr);
        const BookingCodeExceptionSequenceList& lists =
            getBookingCodeExceptionSequence(vndCd, cxr, 0, "0000", REC1, travelDate);
        if (!lists.empty())
        {
          bkgCodes.clear();
          processTable999Seq(lists, bkgCodes, false, vndCd);
        }
      }
    }
    return true;
  }

  else
  {
    if (isSmf)
    {
      LOG4CXX_DEBUG(logger, "\n ------- CONVENTION 2/RULE 0000 --------");
      const BookingCodeExceptionSequenceList& lists =
          getBookingCodeExceptionSequence(_paxTypeFare->vendor(),
                                          _paxTypeFare->carrier(),
                                          _paxTypeFare->tcrRuleTariff(),
                                          "0000",
                                          CONV_2,
                                          travelDate);
      if (!lists.empty())
      {
        bkgCodes.clear();
        processTable999Seq(lists, bkgCodes, true, _paxTypeFare->vendor());
        return true;
      }
    }
    else
    {
      LOG4CXX_DEBUG(logger, "\n ------- CONVENTION 1 --------");
      TariffNumber ruleTariff = 0;
      RuleNumber rule = "0000";
      const BookingCodeExceptionSequenceList& lists =
          getBookingCodeExceptionSequence(vendorCode, cxr, ruleTariff, rule, REC1, travelDate);
      if (!lists.empty())
      {
        bkgCodes.clear();
        processTable999Seq(lists, bkgCodes, convention2, vendorCode);
        return true;
      }
    }
  }

  return false;
}

bool
FareDisplayBookingCodeException::isValidSequence(const BookingCodeExceptionSequence& sequence)
{
  bool foundSequence = true;
  switch (sequence.constructSpecified())
  {
  case BCE_FARE_CONSTRUCTED:
    if (!_paxTypeFare->isConstructed())
    {
      foundSequence = false;
    }
    break; // process this sequence

  case BCE_FARE_SPECIFIED:
    if (_paxTypeFare->isConstructed())
    {
      foundSequence = false;
    }
    break; // process this sequence

  default: // process this sequence
    break;
  }

  return foundSequence;
}

bool
FareDisplayBookingCodeException::processTable999Seq(const BookingCodeExceptionSequenceList& lists,
                                                    std::vector<BookingCode>& bkgCodes,
                                                    bool convention2,
                                                    const VendorCode& vendorCode)
{
  LOG4CXX_DEBUG(logger, "\n Start FareDisplayBookingCodeException::processTable999Seq()");
  LOG4CXX_DEBUG(logger,
                "\nFare Class=" << _paxTypeFare->fareClass() << " vendor=" << vendorCode
                                << " Sequence size=" << lists.size());
  BookingCodeExceptionSequenceListCI start(lists.begin()), end(lists.end()), i(lists.begin());
  _convention2 = convention2;
  _bceBkgUpdated = false;
  if (!lists.empty())
  {
    bool conditionalSegmentFound = false;
    bool allSegmentsValid = false;
    uint16_t seqCount = 0;
    bool bookingCodeFound = false;

    std::vector<segmentStatus> validExceptionList;

    for (i = start; i != end; ++i, ++seqCount)
    {
      BookingCodeExceptionSequence& sequence = **i;
      if (isValidSequence(**i))
      {
        LOG4CXX_DEBUG(logger,
                      "\nSequence Data Found: " << seqCount << " itemno=" << sequence.itemNo()
                                                << " seqNo=" << sequence.seqNo() << " seg count="
                                                << sequence.segmentVector().size());
        BookingCodeExceptionSegmentVector::const_iterator j(sequence.segmentVector().begin()),
            last(sequence.segmentVector().end());
        uint16_t segCount = 0;
        _forceConditional = false;
        _condDirValState = BCE_NO_VALIDATION;
        for (; j != last; ++j, ++segCount)
        {
          BookingCodeExceptionSegment& segment = **j;
          if (isValidSegment(segment, sequence))
          {
            LOG4CXX_DEBUG(logger,
                          "\n --- Segment Data Found: "
                              << segCount << " itemno=" << sequence.itemNo()
                              << " seqNo=" << sequence.seqNo() << " segNo=" << segment.segNo()
                              << " bc1=" << segment.bookingCode1()
                              << " bc2=" << segment.bookingCode2());
            conditionalSegmentFound = isRBSegmentConditional(segment);
            allSegmentsValid = true;
          }
          else
          {
            LOG4CXX_DEBUG(logger,
                          "\n --- Segment Data Failed: "
                              << segCount << " itemno=" << sequence.itemNo()
                              << " seqNo=" << sequence.seqNo() << " segNo=" << segment.segNo()
                              << " bc1=" << segment.bookingCode1()
                              << " bc2=" << segment.bookingCode2());
            allSegmentsValid = false;
            break; // Segment failed, abondone the rest if any
          }
          if (!isRBRequest() && bkgCodes.size() > 1)
          {
            bookingCodeFound = true;
            break;
          }
        } // j
        if (allSegmentsValid) // For RB rule text
        {
          validExceptionList.push_back(
              segmentStatus(&sequence, conditionalSegmentFound, _forceConditional));
          //            addRBBCESeqments(vendorCode, sequence, conditionalSegmentFound, bkgCodes);
        }
      }
      else
      {
        LOG4CXX_DEBUG(logger,
                      "\nSequence Data Failed: " << seqCount << " itemno=" << sequence.itemNo()
                                                 << " seqNo=" << sequence.seqNo());
      }

      // If all segments valid and last segment is unconditional
      // do not continue to next sequences
      if ((allSegmentsValid && !conditionalSegmentFound) || bookingCodeFound)
      {

        break;
      }
    } // i
    std::vector<segmentStatus>::iterator itSeqBegin = validExceptionList.begin(), itSeq;
    for (; itSeqBegin != validExceptionList.end(); itSeqBegin++)
    {
      itSeq = itSeqBegin;
      for (; itSeq != validExceptionList.end(); itSeq++)
      {
        if (itSeq == itSeqBegin)
          continue;
        if ((*itSeq)._isForcedConditionalSeg || (*itSeqBegin)._isForcedConditionalSeg)
          continue;
        if ((*itSeqBegin)._sequence->segmentVector().size() !=
            (*itSeq)._sequence->segmentVector().size())
          continue;
        BookingCodeExceptionSegmentVector::const_iterator seg1(
            (*itSeqBegin)._sequence->segmentVector().begin()),
            seg1E((*itSeqBegin)._sequence->segmentVector().end()),
            seg2((*itSeq)._sequence->segmentVector().begin());
        bool canRemoveSeq = true;
        for (; seg1 != seg1E && canRemoveSeq; seg1++, seg2++)
        {
          if (!canRemoveSegment(**seg1, **seg2, *((*itSeqBegin)._sequence), *((*itSeq)._sequence)))
            canRemoveSeq = false;
        }
        if (canRemoveSeq)
        {
          validExceptionList.erase(itSeq);
          itSeq = itSeqBegin;
        }
      }
      addRBBCESeqments(
          vendorCode, *(*itSeqBegin)._sequence, (*itSeqBegin)._conditionalSegmentFound, bkgCodes);
    }
  }
  LOG4CXX_DEBUG(logger, "\n End FareDisplayBookingCodeException::processTable999Seq()");
  return true;
}

bool
FareDisplayBookingCodeException::canRemoveSegment(const BookingCodeExceptionSegment& seg1,
                                                  const BookingCodeExceptionSegment& seg2,
                                                  const BookingCodeExceptionSequence& seq1,
                                                  const BookingCodeExceptionSequence& seq2)
{
  // check if this is conditional segment
  if (((seq1.segCnt() == 1) || (seq1.segCnt() == 2 && seg1.segNo() == 2)) &&
      ((seg1.restrictionTag() != BCE_REQUIRED) || (seg2.restrictionTag() != BCE_REQUIRED)))
    return false;
  // conditional parameters
  if ((seg1.fltRangeAppl() != seg2.fltRangeAppl()) || (seg1.flight1() != seg2.flight1()) ||
      (seg1.flight2() != seg2.flight2()) || (seg1.equipType() != seg2.equipType()) ||
      (seg1.posTsi() != seg2.posTsi()) || (seg1.posLocType() != seg2.posLocType()) ||
      (seg1.posLoc() != seg2.posLoc()) || (seg1.sellTktInd() != seg2.sellTktInd()) ||
      (seg1.tvlEffYear() != seg2.tvlEffYear()) || (seg1.tvlEffMonth() != seg2.tvlEffMonth()) ||
      (seg1.tvlEffDay() != seg2.tvlEffDay()) || (seg1.tvlDiscYear() != seg2.tvlDiscYear()) ||
      (seg1.tvlDiscMonth() != seg2.tvlDiscMonth()) || (seg1.daysOfWeek() != seg2.daysOfWeek()) ||
      (seg1.tvlStartTime() != seg2.tvlStartTime()) || (seg1.tvlEndTime() != seg2.tvlEndTime()) ||
      (seg1.restrictionTag() != seg2.restrictionTag()) || (seg1.arbZoneNo() != seg2.arbZoneNo()))
    return false;

  return true;
}
bool
FareDisplayBookingCodeException::isValidSegment(const BookingCodeExceptionSegment& segment,
                                                const BookingCodeExceptionSequence& sequence)
{
  // Get statuses for each segment in the Fare Market
  TravelSegPtrVecI iterTvl = _paxTypeFare->fareMarket()->travelSeg().begin();
  AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);

  if (isRBRequest())
  {
    airSeg = _rbData->airSeg();
  }
  if (airSeg == nullptr)
  {
    return false;
  }

  if (sequence.primeInd() == BCE_PRIME && airSeg->carrier() == _paxTypeFare->carrier())
  {
    return false;
  }

  if (!validatePrimarySecondary(segment, sequence, airSeg))
  {
    LOG4CXX_DEBUG(logger,
                  "\n FareDisplayBookingCodeException::validatePrimarySecondary() - FAILED");
    return false;
  }

  if (!validateCarrier(segment, sequence, airSeg))
  {
    LOG4CXX_DEBUG(logger, "\n FareDisplayBookingCodeException::validateCarrier() - FAILED");
    return false;
  }

  if (!segment.tvlPortion().empty() && !validatePortionOfTravel(segment, sequence, airSeg))
  {
    LOG4CXX_DEBUG(logger, "\n FareDisplayBookingCodeException::validatePortionOfTravel() - FAILED");
    return false;
  }

  if (!validateTSI(segment, sequence, airSeg))
  {
    LOG4CXX_DEBUG(logger, "\n FareDisplayBookingCodeException::validateTSI() - FAILED");
    return false;
  }

  if (!validateLocation(segment, sequence))
  {
    LOG4CXX_DEBUG(logger, "\n FareDisplayBookingCodeException::validateLocation() - FAILED");
    return false;
  }

  if (!validatePointOfSale(segment, sequence))
  {
    LOG4CXX_DEBUG(logger, "\n FareDisplayBookingCodeException::validatePointOfSale() - FAILED");
    return false;
  }

  if (!validateSoldTag(segment, sequence))
  {
    LOG4CXX_DEBUG(logger, "\n FareDisplayBookingCodeException::validateSoldTag() - FAILED");
    return false;
  }

  if (!validateDateTimeDOW(segment, sequence))
  {
    LOG4CXX_DEBUG(logger, "\n FareDisplayBookingCodeException::validateDateTimeDOW() - FAILED");
    return false;
  }

  if (!validateFareClassType(segment, sequence))
  {
    LOG4CXX_DEBUG(logger, "\n FareDisplayBookingCodeException::validateFareClassType() - FAILED");
    return false;
  }

  return true;
}

bool
FareDisplayBookingCodeException::isRBSegmentConditional(const BookingCodeExceptionSegment& segment)
{
  return (_forceConditional || (segment.fltRangeAppl() != ' ') || (segment.flight1() > 0) ||
          (segment.flight2() > 0) || (!segment.equipType().empty()) || (segment.posTsi() > 0) ||
          (segment.posLocType() != ' ' && segment.posLocType() != 0) ||
          (!segment.posLoc().empty()) || (segment.sellTktInd() != ' ') ||
          (segment.tvlEffYear() > 0) || (segment.tvlEffMonth() > 0) || (segment.tvlEffDay() > 0) ||
          (segment.tvlDiscYear() > 0) || (segment.tvlDiscMonth() > 0) ||
          (segment.tvlDiscDay() > 0) || (!segment.daysOfWeek().empty()) ||
          (segment.tvlStartTime() != 0 && segment.tvlStartTime() != 65535) ||
          (segment.tvlEndTime() != 0 && segment.tvlEndTime() != 65535) ||
          (!segment.arbZoneNo().empty()) || (segment.restrictionTag() == BCE_PERMITTED) ||
          (segment.restrictionTag() == BCE_PERMITTED_WHEN_PRIME_NOT_OFFERED) ||
          (segment.restrictionTag() == BCE_PERMITTED_WHEN_PRIME_NOT_AVAIL) ||
          (segment.restrictionTag() == BCE_REQUIRED_WHEN_PRIME_NOT_OFFERED) ||
          (segment.restrictionTag() == BCE_REQUIRED_WHEN_PRIME_NOT_AVAIL) ||
          (segment.restrictionTag() == BCE_REQUIRED_WHEN_OFFERED) ||
          (segment.restrictionTag() == BCE_REQUIRED_WHEN_AVAIL) ||
          (_convention2 &&
           (segment.restrictionTag() == BCE_RBD2_PERMITTED_IF_RBD1_AVAILABLE ||
            segment.restrictionTag() == BCE_RBD2_REQUIRED_IF_RBD1_AVAILABLE    )));
}

bool
FareDisplayBookingCodeException::validateCarrier(const BookingCodeExceptionSegment& segment,
                                                 const BookingCodeExceptionSequence& sequence,
                                                 const AirSeg* airSeg)
{
  if (segment.viaCarrier() == BCE_DOLLARDOLLARCARRIER || segment.viaCarrier() == BCE_ANYCARRIER)
  {
    return true;
  }

  if (sequence.ifTag() == BCE_IF_FARECOMPONENT && segment.segNo() == 1)
  {
    if (segment.viaCarrier() == BCE_XDOLLARCARRIER)
    {
      if (_paxTypeFare->fare()->isIndustry())
      {
        return true;
      }
      if (airSeg->carrier() != _paxTypeFare->carrier())
      {
        return true;
      }
    }
    if (segment.viaCarrier() == INDUSTRY_CARRIER)
    {
      if (_paxTypeFare->fare()->isIndustry())
      {
        return true;
      }
    }

    if (segment.viaCarrier() == _paxTypeFare->carrier())
    {
      return true;
    }
    else
    {
      if (isRBRequest() && !_rbData->isSecondaryCityPairSameAsPrime() &&
          segment.viaCarrier() == airSeg->carrier())
      {
        return true;
      }
    }
    return false;
  }
  else // if(bceSequence._ifTag == BCE_MATCH_ANY_TVLSEG || blank)
  {
    if (segment.viaCarrier() == BCE_XDOLLARCARRIER)
    {
      if (_paxTypeFare->fare()->isIndustry() || (airSeg->carrier() != _paxTypeFare->carrier()))
      {
        return true;
      }
    }

    if (segment.viaCarrier() == _paxTypeFare->carrier())
    {
      if (isRBRequest() && segment.viaCarrier() != airSeg->carrier() &&
          (sequence.ifTag() == BCE_CHAR_BLANK || segment.segNo() > 1))
      {
        return false;
      }
      return true;
    }

    if (segment.viaCarrier() == airSeg->carrier() ||
        segment.viaCarrier() == _paxTypeFare->fareMarket()->governingCarrier())
    {
      return true;
    }

    return false;
  }
}

bool
FareDisplayBookingCodeException::validatePortionOfTravel(
    const BookingCodeExceptionSegment& segment,
    const BookingCodeExceptionSequence& sequence,
    const AirSeg* aSeg)
{
  const std::string tvlPortion = segment.tvlPortion();
  // check if this is if condition, if not - validate airSeg
  bool isSec = (sequence.ifTag() == BCE_CHAR_BLANK) || (segment.segNo() > 1);
  const TravelSeg* airSeg = isSec ? aSeg : _paxTypeFare->fareMarket()->travelSeg().front();
  const IATAAreaCode& origArea = airSeg->origin()->area();
  const IATAAreaCode& destArea = airSeg->destination()->area();
  switch (tvlPortion[0])
  {
  case 'A': // if tvlPortion = "AT"
  {
    // pass if one end point of the sector is in area 1 and other point of the
    // sector is in  area 2/3 AND global direction of fare is Atlantic ocean
    if (!isSec && _paxTypeFare->globalDirection() != GlobalDirection::AT)
      return false;
    if (origArea != destArea && (origArea == IATA_AREA1 || destArea == IATA_AREA1))
    {
      if (destArea == IATA_AREA2 || destArea == IATA_AREA3 || origArea == IATA_AREA2 ||
          origArea == IATA_AREA3)
      {
        return true;
      }
    }
    break;
  }

  case 'C':
  {
    if (tvlPortion[1] == 'A') // if tvlPortion == "CA"
    {
      // Canadian Domestic
      if (airSeg->origin()->nation() == CANADA && airSeg->destination()->nation() == CANADA)
      {
        return true;
      }
    }
    else
    {
      if (tvlPortion[1] == 'O') // if tvlPortion = "CO"
      {
        // Controlling Portion
        if ((origArea != destArea) ||
            (airSeg->origin()->nation() != airSeg->destination()->nation()))
        {
          return true;
        }
      }
    }
    break;
  }

  case 'D': // if tvlPortion = "DO"
  {
    // Domestic
    if (LocUtil::isDomestic(*(airSeg->origin()), *(airSeg->destination())) ||
        LocUtil::isForeignDomestic(*(airSeg->origin()), *(airSeg->destination())))
    {
      return true;
    }
    break;
  }

  case 'E': // if tvlPortion = "EH"
  {
    // eastern hemisphere
    // Both end points of the flight are in Area 02 or One end point is in area 02
    // and other in Area 03 then pass

    if ((origArea == IATA_AREA2 && destArea == IATA_AREA2) ||
        (origArea == IATA_AREA2 && destArea == IATA_AREA3) ||
        (destArea == IATA_AREA2 && origArea == IATA_AREA3))
    {
      return true;
    }
    break;
  }

  case 'F':
  {
    if (tvlPortion[1] == 'D') // if tvlPortion = "FD"
    {
      // Domestic except US/CA
      if (LocUtil::isForeignDomestic(*(airSeg->origin()), *(airSeg->destination())) &&
          !LocUtil::isDomestic(*(airSeg->origin()), *(airSeg->destination())))
      {
        return true;
      }
    }
    else
    {
      if (tvlPortion[1] == 'E') // if tvlPortion = "FE"
      {
        // Fare East
        // Pass if all points are in area 3
        if (origArea == IATA_AREA3 && destArea == IATA_AREA3)
        {
          return true;
        }
      }
    }
    break;
  }

  case 'P': // if tvlPortion = "PA"
  {
    // pass if one end point of the sector is in area 1 and other point of the
    // sector is in  area 2/3 AND global direction of fare refers to Pacific ocean
    if (!isSec && _paxTypeFare->globalDirection() != GlobalDirection::PA)
      return false;
    if (origArea != destArea && (origArea == IATA_AREA1 || destArea == IATA_AREA1))
    {
      if (destArea == IATA_AREA2 || destArea == IATA_AREA3 || origArea == IATA_AREA2 ||
          origArea == IATA_AREA3)
      {
        return true;
      }
    }
    break;
  }

  case 'T': // if tvlPortion = "TB"
  {
    // Transborder
    if (LocUtil::isTransBorder(*(airSeg->origin()), *(airSeg->destination())))
    {
      return true;
    }
    break;
  }
  case 'U': // if tvlPortion = "US"
  {
    // US Domestic
    if (airSeg->origin()->nation() == UNITED_STATES &&
        airSeg->destination()->nation() == UNITED_STATES)
    {
      return true;
    }
    break;
  }
  case 'W': // if tvlPortion = "WH"
  {
    // Western Hemisphere
    if (origArea == IATA_AREA1 && destArea == IATA_AREA1)
    {
      return true;
    }
    break;
  }

  default:
  {
    return true;
  }
  } // end switch
  return false;
}

bool
FareDisplayBookingCodeException::validatePrimarySecondary(
    const BookingCodeExceptionSegment& segment,
    const BookingCodeExceptionSequence& sequence,
    const AirSeg* airSeg)
{
  const TravelSeg* travelSeg = airSeg;

  if (segment.primarySecondary() == BCE_CHAR_BLANK)
  {
    return true;
  }
  if ((segment.primarySecondary() == BCE_SEG_PRIMARY ||
       segment.primarySecondary() == BCE_SEG_SECONDARY ||
       segment.primarySecondary() == BCE_SEG_FROMTO_PRIMARY) &&
      (_paxTypeFare->isDomestic() || _paxTypeFare->isTransborder() ||
       _paxTypeFare->isForeignDomestic()))
  {
    return false;
  }

  CarrierCode primeCxr = _paxTypeFare->fareMarket()->governingCarrier();

  CarrierCode travelCxr = airSeg->carrier();
  if (travelCxr.empty())
  {
    travelCxr = _paxTypeFare->fareMarket()->governingCarrier();
  }
  // for any flight segment pass if segment carrier match governing carrier
  if (sequence.ifTag() == BCE_CHAR_BLANK && primeCxr == travelCxr)
    return true;

  if (segment.primarySecondary() == BCE_SEG_PRIMARY)
  {

    /*    if (_rbData != 0 && _rbData->isSecondary())
        {
          return true;
        }
    */
    if (travelCxr == primeCxr && travelSeg->origin() == _paxTypeFare->fareMarket()->origin() &&
        travelSeg->destination() == _paxTypeFare->fareMarket()->destination())
    {
      return true;
    }

    if (_rbData != nullptr && _rbData->isSecondary() &&
        (travelSeg->origin() == _paxTypeFare->fareMarket()->origin() ||
         airSeg->origAirport() == _paxTypeFare->fareMarket()->boardMultiCity()))
    {
      return true;
    }
  }
  else // must be Secondary because we already checked for blank
  {
    if (_rbData != nullptr && _rbData->isSecondary())
    {
      return true;
    }
  }
  return false;
}

bool
FareDisplayBookingCodeException::validateTSI(const BookingCodeExceptionSegment& segment,
                                             const BookingCodeExceptionSequence& sequence,
                                             const AirSeg* airSeg)
{
  const TSICode bceTSI = segment.tsi();
  RuleUtil::TravelSegWrapperVector applTvlSegs;

  LocKey locKey1;
  LocKey locKey2;

  locKey1.locType() = LOCTYPE_NONE;
  locKey2.locType() = LOCTYPE_NONE;

  if (bceTSI == 0)
  {
    return true;
  }
  const TravelSeg* tvlSeg = ((sequence.ifTag() == BCE_CHAR_BLANK) || (segment.segNo() != 1))
                                ? airSeg
                                : _paxTypeFare->fareMarket()->travelSeg().front();

  FarePath* farePath = nullptr;
  _trx->dataHandle().get(farePath);
  if (farePath == nullptr)
  {
    LOG4CXX_DEBUG(
        logger,
        "FareDisplayBookingCodeException::validateTSI() - UNABLE TO ALLOCATE MEMORY FOR FAREPATH");
    return false;
  }

  PricingUnit* pu = nullptr;
  if (!FareDisplayUtil::initializeFarePath(*_trx, farePath))
  {
    farePath = nullptr;
    pu = nullptr;
  }
  else
  {
    updateTravelSeg(farePath->itin()->travelSeg(), tvlSeg);
    pu = farePath->pricingUnit().front();
  }

  bool tsiResult = scopeTSIGeo(bceTSI,
                               locKey1,
                               locKey2,
                               RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT,
                               *_trx,
                               farePath,
                               pu,
                               _paxTypeFare->fareMarket(),
                               _trx->getRequest()->ticketingDT(),
                               applTvlSegs,
                               Diagnostic405);

  if (tsiResult)
  {
    if (applTvlSegs.empty())
    {
      return false;
    }
  }
  else
  {
    return false;
  }

  return true;
}

bool
FareDisplayBookingCodeException::validateLocation(const BookingCodeExceptionSegment& segment,
                                                  const BookingCodeExceptionSequence& sequence)
{

  if ((segment.loc1().empty() && segment.loc2().empty()) ||
      (BCE_CONDITIONAL_VALIDATION_MATCH == _condDirValState))
  {
    // fail location if this is last segment and previous segment don't match
    if (BCE_CONDITIONAL_VALIDATION_NO_MATCH == _condDirValState &&
        segment.segNo() == sequence.segCnt())
      return false;
    return true;
  }

  const Loc* fareOrigLoc = _paxTypeFare->fareMarket()->origin();
  const Loc* fareDestLoc = _paxTypeFare->fareMarket()->destination();
  TravelSeg* airSeg = _paxTypeFare->fareMarket()->travelSeg().front();
  if (isRBRequest())
  {
    airSeg = (TravelSeg*)_rbData->airSeg();
  }

  if (fareOrigLoc == nullptr || fareDestLoc == nullptr)
  {
    return false;
  }
  bool failSequence = false;
  bool failFlight = false;
  switch (segment.directionInd())
  {
  case '1': // Travel from Loc1 to Loc2
  {
    if (sequence.ifTag() == BCE_IF_FARECOMPONENT && segment.segNo() == 1) // if IFTag == 1
    {
      if (!segment.loc1().empty() && // validate loc 1
          !isInLoc(*fareOrigLoc, segment.loc1Type(), segment.loc1(), "ATP", RESERVED))
      {
        if (!isRBRequest() || !_paxTypeFare->isRoundTrip() || segment.loc1().empty() ||
            !isInLoc(*fareDestLoc, segment.loc1Type(), segment.loc1(), "ATP", RESERVED))
        {
          failSequence = true;
          break;
        }
      }

      if (!segment.loc2().empty() && // validate loc 2
          !isInLoc(*fareDestLoc, segment.loc2Type(), segment.loc2(), "ATP", RESERVED))
      {
        if (!isRBRequest() || !_paxTypeFare->isRoundTrip() || segment.loc2().empty() ||
            !isInLoc(*fareOrigLoc, segment.loc2Type(), segment.loc2(), "ATP", RESERVED))
        {
          failSequence = true;
          break;
        }
      }
      if (isRBRequest() && _paxTypeFare->isRoundTrip())
        _forceConditional = true;
    }
    else // if IFTag == blank or 2
    {
      //        if(!segment.loc1().empty() &&                 // validate loc 1
      //           (!LocUtil::isInLoc(airSeg->origAirport(),segment.loc1Type(),
      if (!segment.loc1().empty() && // validate loc 1
          !isInLoc(airSeg->origAirport(), segment.loc1Type(), segment.loc1(), "ATP", RESERVED))
      {
        failFlight = true;
        break;
      }

      if (!segment.loc2().empty() && // validate loc 2
          !isInLoc(airSeg->destAirport(), segment.loc2Type(), segment.loc2(), "ATP", RESERVED))
      {
        failFlight = true;
        break;
      }
    }
    break;
  }
  case '2': // Travel from Loc2 to Loc1
  {
    if (sequence.ifTag() == BCE_IF_FARECOMPONENT && segment.segNo() == 1) // if IFTag == 1
    {
      if (!segment.loc1().empty() && // validate loc 1
          !isInLoc(*fareDestLoc, segment.loc1Type(), segment.loc1(), "ATP", RESERVED))
      {
        if (!isRBRequest() || !_paxTypeFare->isRoundTrip() || segment.loc1().empty() ||
            !isInLoc(*fareOrigLoc, segment.loc1Type(), segment.loc1(), "ATP", RESERVED))
        {
          failSequence = true;
          break;
        }
      }

      if (!segment.loc2().empty() && // validate loc 2
          !isInLoc(*fareOrigLoc, segment.loc2Type(), segment.loc2(), "ATP", RESERVED))
      {
        if (!isRBRequest() || !_paxTypeFare->isRoundTrip() ||
            (segment.loc1().empty() &&
             !isInLoc(*fareDestLoc, segment.loc1Type(), segment.loc1(), "ATP", RESERVED)))
        {
          failSequence = true;
          break;
        }
      }
      if (isRBRequest() && _paxTypeFare->isRoundTrip())
        _forceConditional = true;
    }
    else // if IFTag == blank or 2
    {
      if (!segment.loc1().empty() && // validate loc 1
          !isInLoc(airSeg->destAirport(), segment.loc1Type(), segment.loc1(), "ATP", RESERVED))
      {
        failFlight = true;
        break;
      }

      if (!segment.loc2().empty() && // validate loc 2
          !isInLoc(airSeg->origAirport(), segment.loc2Type(), segment.loc2(), "ATP", RESERVED))
      {
        failFlight = true;
        break;
      }
    }
    break;
  }

  case '3': // Fares originating Loc1
  {
    if (_paxTypeFare->fareMarket()->direction() == FMDirection::INBOUND)
    {
      // if the fare is INBOUND then LOC1 should be checked against the
      // destination because destination is actually the origination point
      // in this case. This is valid only for directionality values 3 and 4
      // and not for values 1 and 2
      fareDestLoc = _paxTypeFare->fareMarket()->origin();
      fareOrigLoc = _paxTypeFare->fareMarket()->destination();
    }
    if (sequence.ifTag() == BCE_IF_FARECOMPONENT && segment.segNo() == 1) // if IFTag == 1
    {
      if (!segment.loc1().empty() && // validate loc 1
          !isInLoc(*fareOrigLoc, segment.loc1Type(), segment.loc1(), "ATP", RESERVED))
      {
        failSequence = true;
        break;
      }

      if (!segment.loc2().empty() && // validate loc 2
          !isInLoc(*fareDestLoc, segment.loc2Type(), segment.loc2(), "ATP", RESERVED))
      {
        failSequence = true;
        break;
      }
    }
    else // When directional indicator == 3 the IFTag must not be blank or 2
    {
      failFlight = true;
      break;
    }
    break;
  }
  case '4': // Fares originating Loc2
  {
    if (_paxTypeFare->fareMarket()->direction() == FMDirection::INBOUND)
    {
      // if the fare is INBOUND then LOC1 should be checked against the
      // destination because destination is actually the origination point
      // in this case. This is valid only for directionality values 3 and 4
      // and not for values 1 and 2
      fareDestLoc = _paxTypeFare->fareMarket()->origin();
      fareOrigLoc = _paxTypeFare->fareMarket()->destination();
    }
    if (sequence.ifTag() == BCE_IF_FARECOMPONENT && segment.segNo() == 1) // if IFTag == 1
    {
      if (!segment.loc1().empty() && // validate loc 1
          !isInLoc(*fareDestLoc, segment.loc1Type(), segment.loc1(), "ATP", RESERVED))
      {
        failSequence = true;
        break;
      }

      if (!segment.loc2().empty() && // validate loc 2
          !isInLoc(*fareOrigLoc, segment.loc2Type(), segment.loc2(), "ATP", RESERVED))
      {
        failSequence = true;
        break;
      }
    }
    else // When directional indicator == 3 the IFTag must not blank or 2
    {
      failFlight = true;
      break;
    }
    break;
  }
  case ' ': // Travel/fare between Loc 1 and Loc2
  {
    bool origInLoc1 = false;
    bool origInLoc2 = false;
    bool destInLoc1 = false;
    bool destInLoc2 = false;
    if (sequence.ifTag() == BCE_IF_FARECOMPONENT && segment.segNo() == 1) // if IFTag == 1
    {
      if (segment.loc1() == segment.loc2() && segment.loc1Type() == segment.loc2Type() &&
          !segment.loc1().empty())
      {
        // WITHIN loc1
        if (!(isInLoc(*fareOrigLoc, segment.loc1Type(), segment.loc1(), "ATP", RESERVED) &&
              isInLoc(*fareDestLoc, segment.loc1Type(), segment.loc1(), "ATP", RESERVED)))
        {
          failSequence = true;
          break;
        }
      }
      else
      {
        // BETWEEN loc1/loc2
        if (segment.loc1().empty() ||
            isInLoc(*fareOrigLoc, segment.loc1Type(), segment.loc1(), "ATP", RESERVED))
        {
          origInLoc1 = true;
        }

        if (segment.loc2().empty() ||
            isInLoc(*fareDestLoc, segment.loc2Type(), segment.loc2(), "ATP", RESERVED))
        {
          destInLoc2 = true;
        }

        if (segment.loc2().empty() ||
            isInLoc(*fareOrigLoc, segment.loc2Type(), segment.loc2(), "ATP", RESERVED))
        {
          origInLoc2 = true;
        }

        if (segment.loc1().empty() ||
            isInLoc(*fareDestLoc, segment.loc1Type(), segment.loc1(), "ATP", RESERVED))
        {
          destInLoc1 = true;
        }

        if (!((origInLoc1 && destInLoc2) || (origInLoc2 && destInLoc1)))
        {
          failSequence = true;
          break;
        }
      }
    }
    else // if IFTag == blank or 2
    {
      if (segment.loc1() == segment.loc2() && segment.loc1Type() == segment.loc2Type() &&
          !segment.loc1().empty())
      {
        // WITHIN loc1
        if (!(isInLoc(airSeg->origAirport(), segment.loc1Type(), segment.loc1(), "ATP", RESERVED) &&
              isInLoc(airSeg->destAirport(), segment.loc1Type(), segment.loc1(), "ATP", RESERVED)))
        {
          failFlight = true;
          break;
        }
      }
      else
      {
        // BETWEEN loc1/loc2
        if (segment.loc1().empty() ||
            isInLoc(airSeg->origAirport(), segment.loc1Type(), segment.loc1(), "ATP", RESERVED))
        {
          origInLoc1 = true;
        }

        if (segment.loc2().empty() ||
            isInLoc(airSeg->destAirport(), segment.loc2Type(), segment.loc2(), "ATP", RESERVED))
        {
          destInLoc2 = true;
        }

        if (segment.loc2().empty() ||
            isInLoc(airSeg->origAirport(), segment.loc2Type(), segment.loc2(), "ATP", RESERVED))
        {
          origInLoc2 = true;
        }

        if (segment.loc1().empty() ||
            isInLoc(airSeg->destAirport(), segment.loc1Type(), segment.loc1(), "ATP", RESERVED))
        {
          destInLoc1 = true;
        }

        if (!((origInLoc1 && destInLoc2) || (origInLoc2 && destInLoc1)))
        {
          _condDirValState = BCE_CONDITIONAL_VALIDATION_NO_MATCH;
          failFlight = true;
          break;
        }
        // this air segment match to the whole sequence
        _condDirValState = BCE_CONDITIONAL_VALIDATION_MATCH;
        // if more then one segment in sequence, then make this conditional
      }
    }
    break;
  }
  default: // if direction indicaor is anyrhing else then 1,2,3,4 or blank
  {
    failSequence = true;
    break;
  }
  } // end SWITCH

  // no match in last segment
  if (BCE_CONDITIONAL_VALIDATION_NO_MATCH == _condDirValState &&
      segment.segNo() == sequence.segCnt())
    return false;

  if (failSequence)
  {
    return false;
  }

  if (failFlight)
  {
    if (_condDirValState == BCE_NO_VALIDATION) // IFTag <> blank
      return false;
    return true;
  }

  return true;
}

bool
FareDisplayBookingCodeException::validatePointOfSale(const BookingCodeExceptionSegment& segment,
                                                     const BookingCodeExceptionSequence& sequence)
{
  return true;
}

bool
FareDisplayBookingCodeException::validateSoldTag(const BookingCodeExceptionSegment& segment,
                                                 const BookingCodeExceptionSequence& sequence)
{
  return true;
}

bool
FareDisplayBookingCodeException::validateDateTimeDOW(const BookingCodeExceptionSegment& segment,
                                                     const BookingCodeExceptionSequence& sequence)
{
  return true;
}

bool
FareDisplayBookingCodeException::validateFareClassType(const BookingCodeExceptionSegment& segment,
                                                       const BookingCodeExceptionSequence& sequence)
{
  const FareClassCode bceFareClass = segment.fareclass();
  const FareClassCode paxFareClass = _paxTypeFare->fareClass();
  if (bceFareClass.empty())
    return true;

  switch (segment.fareclassType())
  {
  case 'T':
  {
    if (RuleUtil::matchFareType(bceFareClass.c_str(), _paxTypeFare->fcaFareType()))
      return true;
    break;
  } // end case 'T'

  case 'F':
  {
    if (bceFareClass == paxFareClass)
      return true;
    break;
  } // end case 'F'

  case 'M':
  {
    if (RuleUtil::matchFareClass(bceFareClass.c_str(), paxFareClass.c_str()))
      return true;
    break;
  } // end case 'M'
  case 'A':
  {
    if (bceFareClass[0] == paxFareClass[0])
      return true;
    break;
  } // end case 'A'

  default:
    break;
  }

  return false;
}

bool
FareDisplayBookingCodeException::changeFareBasisCode(const BookingCodeExceptionSegment& segment,
                                                     BookingCode bkg)
{
  bool conditionalData = false;
  conditionalData =
      (segment.fltRangeAppl() != BCE_CHAR_BLANK || segment.flight1() > 0 ||
       (!segment.equipType().empty() && segment.equipType() != BCE_EQUIPBLANK) ||
       !segment.tvlPortion().empty() || segment.posTsi() > 0 || !segment.posLoc().empty() ||
       (segment.posLocType() != BCE_CHAR_BLANK && segment.posLocType() != '\0') ||
       (segment.soldInOutInd() != BCE_CHAR_BLANK && segment.soldInOutInd() != '\0') ||
       segment.tvlEffYear() > 0 || segment.tvlEffMonth() > 0 || segment.tvlEffDay() > 0 ||
       segment.tvlDiscYear() > 0 || segment.tvlDiscMonth() > 0 || segment.tvlDiscDay() > 0 ||
       !segment.daysOfWeek().empty() ||
       (segment.tvlStartTime() != 0 && segment.tvlStartTime() != 65535) ||
       (segment.tvlEndTime() != 0 && segment.tvlEndTime() != 65535) ||
       !segment.arbZoneNo().empty());

  if (conditionalData || segment.restrictionTag() == BCE_STANDBY || //  S
      segment.restrictionTag() == BCE_NOT_PERMITTED) //  X
  {
    return false;
  }

  if (!conditionalData && (segment.restrictionTag() == BCE_PERMITTED_WHEN_PRIME_NOT_OFFERED || // O
                           segment.restrictionTag() == BCE_PERMITTED_WHEN_PRIME_NOT_AVAIL || // A
                           segment.restrictionTag() == BCE_REQUIRED_WHEN_PRIME_NOT_OFFERED || // G
                           segment.restrictionTag() == BCE_REQUIRED_WHEN_PRIME_NOT_AVAIL || // H
                           segment.restrictionTag() == BCE_ADDITIONAL_DATA_APPLIES || // U
                           segment.restrictionTag() == BCE_DOES_NOT_EXIST)) // N
  {
    _paxTypeFare->setChangeFareBasisBkgCode("");
    return true;
  }

  if (!conditionalData && (segment.restrictionTag() == BCE_PERMITTED || // P
                           segment.restrictionTag() == BCE_REQUIRED || // R
                           segment.restrictionTag() == BCE_REQUIRED_WHEN_OFFERED || // W
                           segment.restrictionTag() == BCE_REQUIRED_WHEN_AVAIL)) // V
  {
    _paxTypeFare->setChangeFareBasisBkgCode(bkg);
    return true;
  }
  return false;
}
//--------------------------------------------------------------------
// Return true if RB this is transaction
//--------------------------------------------------------------------
bool
FareDisplayBookingCodeException::isRBRequest()
{
  if (_rbData == nullptr)
    return false;

  return (_trx->getRequest()->requestType() == "RB");
}

//--------------------------------------------------------------------
// Add RB Booking code exception segments
//--------------------------------------------------------------------
void
FareDisplayBookingCodeException::addRBBCESeqments(const VendorCode& vendorCode,
                                                  BookingCodeExceptionSequence& sequence,
                                                  bool& conditionalSegmentFound,
                                                  std::vector<BookingCode>& bkgCodes)
{
  BookingCode bkg;
  BookingCodeExceptionSegmentVector segmentVector = sequence.segmentVector();
  BookingCodeExceptionSegmentVector::const_iterator j(segmentVector.begin()),
      last(segmentVector.end());
  for (; j != last; ++j)
  {
    BookingCodeExceptionSegment& segment = **j;
    if (isRBRequest())
    {
      RBDataItem rbDataItem;
      RBDataItem* item = rbDataItem.getRBDataItem(*_trx);
      item->_vendor = vendorCode;
      item->_itemNo = sequence.itemNo();
      item->_seqNo = sequence.seqNo();
      item->_segNo = segment.segNo();
      _rbData->addItem(item);
      _rbData->setLastSegmentConditional(conditionalSegmentFound);

      if (!segment.bookingCode1().empty())
        bkgCodes.push_back(segment.bookingCode1());
      if (!segment.bookingCode2().empty())
        bkgCodes.push_back(segment.bookingCode2());
    }
    else
    {
      if (!segment.bookingCode1().empty())
      {
        // Add bkg code exception to bkgCodes
        bkgCodes.push_back(segment.bookingCode1());
        if (!_convention2 && _paxTypeFare->carrier() == "YY" && !_bceBkgUpdated)
        {
          bkg = segment.bookingCode1();
          if (changeFareBasisCode(segment, bkg))
          {
            _bceBkgUpdated = true;
          }
        }
      }
      if (!segment.bookingCode2().empty())
      {
        // Add bkg code exception to bkgCodes
        bkgCodes.push_back(segment.bookingCode2());
        if (!_convention2 && _paxTypeFare->carrier() == INDUSTRY_CARRIER && !_bceBkgUpdated)
        {
          bkg = segment.bookingCode2();
          if (changeFareBasisCode(segment, bkg))
          {
            _bceBkgUpdated = true;
          }
        }
      }
    }
    //      LOG4CXX_DEBUG(logger, "\nv vendor="<<item->_vendor<<" itemno="<<item->_itemNo<<"
    //      seqNo="<<item->_seqNo<<" segNo="<<item->_segNo);
  }
}

void
FareDisplayBookingCodeException::updateTravelSeg(std::vector<TravelSeg*>& travelSeg,
                                                 const TravelSeg* tvlSeg)
{
  if (travelSeg.size() > 0)
  {
    travelSeg[0]->origAirport() = tvlSeg->origAirport();
    travelSeg[0]->origin() = tvlSeg->origin();
    travelSeg[0]->destAirport() = tvlSeg->destAirport();
    travelSeg[0]->destination() = tvlSeg->destination();
  }
  if (travelSeg.size() > 1)
  {
    travelSeg[1]->origAirport() = tvlSeg->destAirport();
    travelSeg[1]->origin() = tvlSeg->destination();
    travelSeg[1]->destAirport() = tvlSeg->origAirport();
    travelSeg[1]->destination() = tvlSeg->origin();
  }
}
const BookingCodeExceptionSequenceList&
FareDisplayBookingCodeException::getBookingCodeExceptionSequence(const VendorCode& vendor,
                                                                 const int itemNo)
{
  return _trx->dataHandle().getBookingCodeExceptionSequence(vendor, itemNo, !_useBKGExceptionIndex);
}
const BookingCodeExceptionSequenceList&
FareDisplayBookingCodeException::getBookingCodeExceptionSequence(const VendorCode& vendor,
                                                                 const CarrierCode& carrier,
                                                                 const TariffNumber& ruleTariff,
                                                                 const RuleNumber& rule,
                                                                 Indicator conventionNo,
                                                                 const DateTime& date)
{
  bool isRuleZero = false;
  return _trx->dataHandle().getBookingCodeExceptionSequence(
      vendor, carrier, ruleTariff, rule, conventionNo, date, isRuleZero, !_useBKGExceptionIndex);
}
bool
FareDisplayBookingCodeException::scopeTSIGeo(const TSICode tsi,
                                             const LocKey& locKey1,
                                             const LocKey& locKey2,
                                             const RuleConst::TSIScopeParamType& defaultScope,
                                             PricingTrx& trx,
                                             const FarePath* farePath,
                                             const PricingUnit* pricingUnit,
                                             const FareMarket* fareMarket,
                                             const DateTime& ticketingDate,
                                             RuleUtil::TravelSegWrapperVector& applTravelSegment,
                                             const DiagnosticTypes& callerDiag,
                                             const VendorCode& vendorCode)
{
  return RuleUtil::scopeTSIGeo(tsi,
                               locKey1,
                               locKey2,
                               defaultScope,
                               false,
                               false,
                               false,
                               trx,
                               farePath,
                               nullptr,
                               pricingUnit,
                               fareMarket,
                               ticketingDate,
                               applTravelSegment,
                               callerDiag,
                               vendorCode);
}
bool
FareDisplayBookingCodeException::isInLoc(const LocCode& city,
                                         const LocTypeCode& locTypeCode,
                                         const LocCode& locCode,
                                         const VendorCode& vendorCode,
                                         const ZoneType zoneType,
                                         const GeoTravelType geoTvlType,
                                         const LocUtil::ApplicationType locAppl,
                                         const DateTime& ticketingDate)
{
  return LocUtil::isInLoc(
      city, locTypeCode, locCode, vendorCode, zoneType, geoTvlType, locAppl, ticketingDate);
}
bool
FareDisplayBookingCodeException::isInLoc(const Loc& loc,
                                         const LocTypeCode& locTypeCode,
                                         const LocCode& locCode,
                                         const VendorCode& vendorCode,
                                         const ZoneType zoneType,
                                         LocUtil::ApplicationType applType,
                                         GeoTravelType geoTvlType,
                                         const CarrierCode& carrier,
                                         const DateTime& ticketingDate)
{
  return LocUtil::isInLoc(loc,
                          locTypeCode,
                          locCode,
                          vendorCode,
                          zoneType,
                          applType,
                          geoTvlType,
                          carrier,
                          ticketingDate);
}
} // namspace tse
