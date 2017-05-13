// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "FreeBagService/CarryOnBaggageTextFormatter.h"

#include "Common/Money.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/ServicesDescription.h"
#include "DBAccess/ServicesSubGroup.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/TaxText.h"
#include "Diagnostic/Diag852Collector.h"

#include <algorithm>
#include <functional>
#include <tuple>

#include <boost/foreach.hpp>

namespace tse
{
CarryOnBaggageTextFormatter::T196DiagHandler::T196DiagHandler() : _diag852(nullptr) {}

CarryOnBaggageTextFormatter::T196DiagHandler::T196DiagHandler(
    Diag852Collector* diag852,
    const std::vector<uint32_t>& portionsToCheck,
    const OptionalServicesInfo* s7,
    const FarePath* farePath,
    const PricingTrx& trx)
  : _diag852(diag852)
{
  if (_diag852)
  {
    for (uint32_t portion : portionsToCheck)
    {
      if (_diag852->printTable196ForCarryOnDetailSetup(portion, s7, *farePath, trx))
        break;
    }
  }
}

void
CarryOnBaggageTextFormatter::T196DiagHandler::
operator()(const SubCodeInfo* s5)
{
  if (_diag852)
    _matchedS5s.push_back(s5);
}

void
CarryOnBaggageTextFormatter::T196DiagHandler::flush(const TaxText* taxText,
                                                    const OptionalServicesInfo* s7)
{
  if (_diag852)
  {
    _diag852->printTable196DetailHeader(s7->taxTblItemNo(), taxText->txtMsgs());

    for (const SubCodeInfo* s5 : _matchedS5s)
    {
      _diag852->printTable196Detail(s5);
      _diag852->printTable196Detail(true);
      _diag852->printTable196Detail(false);
    }

    for (const SubCodeInfo* s5 : _matchedS5s)
      _diag852->printTable196DetailEnd(s5);
  }
}

CarryOnBaggageTextFormatter::TravelDisclosureData::TravelDisclosureData(
    const BaggageTravel* bt, const std::string& travelText, uint32_t portionIndex)
  : _bt(bt), _travelText(travelText)
{
  _matchingPortionsIndexes.push_back(portionIndex);
}

const BaggageTravel*
CarryOnBaggageTextFormatter::TravelDisclosureData::getBagTravel() const
{
  return _bt;
}

const std::string&
CarryOnBaggageTextFormatter::TravelDisclosureData::getTravelText() const
{
  return _travelText;
}

const std::vector<uint32_t>&
CarryOnBaggageTextFormatter::TravelDisclosureData::getPortions() const
{
  return _matchingPortionsIndexes;
}

void
CarryOnBaggageTextFormatter::TravelDisclosureData::appendTravelTextAndPortion(
    const std::string& textToApend, uint32_t portionIndex)
{
  _travelText += textToApend;
  _matchingPortionsIndexes.push_back(portionIndex);
}

const std::string CarryOnBaggageTextFormatter::CARRY_ON_CHARGES_UNKNOWN =
    "CARRY ON FEES UNKNOWN-CONTACT CARRIER";
const std::string CarryOnBaggageTextFormatter::CARRY_ON_ALLOWANCE_UNKNOWN =
    "CARRY ON ALLOWANCE UNKNOWN-CONTACT CARRIER";
const std::string CarryOnBaggageTextFormatter::EACH_AND_ABOVE = " AND EACH ABOVE ";
const std::string CarryOnBaggageTextFormatter::OVER = "OVER ";
const std::string CarryOnBaggageTextFormatter::PER_KILO = " PER KILO";
const std::string CarryOnBaggageTextFormatter::PER_5_KILOS = " PER 5 KILOS";
const std::string CarryOnBaggageTextFormatter::KILOS = "KG";
const std::string CarryOnBaggageTextFormatter::POUNDS = "LB";
const std::string CarryOnBaggageTextFormatter::SPACE = " ";
const std::string CarryOnBaggageTextFormatter::NIL = "NIL";
const std::string CarryOnBaggageTextFormatter::AND = " AND ";
const std::string CarryOnBaggageTextFormatter::PET = "PET";
const std::string CarryOnBaggageTextFormatter::EACH_PIECE = "EACH PIECE";
const std::string CarryOnBaggageTextFormatter::ADDITIONAL = "ADDITIONAL ";

CarryOnBaggageTextFormatter::CarryOnBaggageTextFormatter(PricingTrx& trx)
  : BaggageTextFormatter(trx)
{
}

std::string
CarryOnBaggageTextFormatter::formatCarryOnChargeText(const BaggageCharge* baggageCharge,
                                                     const bool singleS7Matched) const
{
  return baggageCharge->optFee()
             ? (getChargeOccurrenceText(baggageCharge->optFee(), singleS7Matched) +
                getExcessBaggageWeightText(baggageCharge->optFee()) +
                getCarryOnChargeDescriptionText(baggageCharge->subCodeInfo()) + DASH +
                getCarryOnChargeAmountAndCurrencyText(baggageCharge) +
                getChargeApplicactionText(baggageCharge->optFee()))
             : (getCarryOnChargeDescriptionText(baggageCharge->subCodeInfo()) + DASH +
                CARRY_ON_CHARGES_UNKNOWN);
}

bool
CarryOnBaggageTextFormatter::hasTaxTable(const OptionalServicesInfo* s7) const
{
  return _trx.dataHandle().getTaxText(s7->vendor(), s7->taxTblItemNo()) != nullptr;
}

std::string
CarryOnBaggageTextFormatter::formatCarryOnAllowanceText(
    const CarryOnBaggageTextFormatter::TravelDisclosureData& data,
    Diag852Collector* t196TablePrinter) const
{
  std::string retText;
  const BaggageTravel* bt = data.getBagTravel();

  if (bt && bt->_allowance && bt->_allowance->optFee())
  {
    if (bt->_allowance->optFee()->freeBaggagePcs() == 0)
    {
      retText += DASH + NIL + SLASH + getOperatingCarrierText(bt);
    }
    else if (!hasTaxTable(bt->_allowance->optFee()))
    {

      retText += getWeightPiecesText(bt->_allowance->optFee()) + getOperatingCarrierText(bt);
    }
    else
    {
      T196DiagHandler t196Handler(
          t196TablePrinter, data.getPortions(), bt->_allowance->optFee(), bt->farePath(), _trx);

      retText += getWeightPiecesText(bt->_allowance->optFee()) + getOperatingCarrierText(bt) +
                 BaggageTextFormatter::NEW_LINE +
                 getDescriptionsText(bt->_allowance->optFee(), t196Handler);
      return retText;
    }
  }
  else
  {
    retText += DASH + getOperatingCarrierText(bt) + DASH + CARRY_ON_ALLOWANCE_UNKNOWN;
  }

  retText += BaggageTextFormatter::NEW_LINE;
  return retText;
}

std::string
CarryOnBaggageTextFormatter::getCarryOnChargeAmountAndCurrencyText(
    const BaggageCharge* baggageCharge) const
{
  ServiceFeeUtil util(_trx);
  return getChargeAmountAndCurrencyText(baggageCharge->optFee(),
                                        util.convertBaggageFeeCurrency(baggageCharge));
}

std::string
CarryOnBaggageTextFormatter::getChargeOccurrenceText(const OptionalServicesInfo* s7,
                                                     const bool singleS7Matched)
{
  std::ostringstream retStream;
  if (!singleS7Matched || (s7->notAvailNoChargeInd() != NOT_PERMITTED_IND))
  {
    int32_t firstOccurrence = s7->baggageOccurrenceFirstPc();
    if ((firstOccurrence > 1) && (s7->notAvailNoChargeInd() == NOT_PERMITTED_IND))
    {
      retStream << ADDITIONAL;
    }
    else if (firstOccurrence > 0)
    {
      retStream << firstOccurrence << getOrdinalSuffixText(firstOccurrence);
      int32_t lastOccurrence = s7->baggageOccurrenceLastPc();
      if (lastOccurrence > 0)
      {
        if (lastOccurrence != firstOccurrence)
        {
          retStream << DASH << lastOccurrence << getOrdinalSuffixText(lastOccurrence);
        }
      }
      else
      {
        retStream << EACH_AND_ABOVE << firstOccurrence << getOrdinalSuffixText(firstOccurrence);
      }
      retStream << SPACE;
    }
  }
  return retStream.str();
}

std::string
CarryOnBaggageTextFormatter::getOrdinalSuffixText(const int32_t ordinal)
{
  std::string retText;
  if (ordinal / 10 == 1)
  {
    retText = "TH";
  }
  else
  {
    switch (ordinal % 10)
    {
    case 1:
      retText = "ST";
      break;
    case 2:
      retText = "ND";
      break;
    case 3:
      retText = "RD";
      break;
    default:
      retText = "TH";
      break;
    }
  }
  return retText;
}

std::string
CarryOnBaggageTextFormatter::getExcessBaggageWeightText(const OptionalServicesInfo* s7)
{
  std::ostringstream retStream;
  int32_t excessBaggageWeight = s7->baggageWeight();
  if (excessBaggageWeight > 0)
  {
    retStream << OVER << excessBaggageWeight << (s7->baggageWeightUnit() == 'K' ? KILOS : POUNDS)
              << SPACE;
  }
  return retStream.str();
}

std::string
CarryOnBaggageTextFormatter::getCarryOnChargeDescriptionText(const SubCodeInfo* s5) const
{
  std::string retText;
  retText = getDescriptionText(s5);
  if (retText.empty())
  {
    retText = s5->commercialName();
  }
  return retText;
}

std::string
CarryOnBaggageTextFormatter::getChargeApplicactionText(const OptionalServicesInfo* s7)
{
  std::string retText;
  switch (s7->frequentFlyerMileageAppl())
  {
  case 'H':
  case 'C':
  case 'P':
  case 'K':
    retText = PER_KILO;
    break;
  case 'F':
    retText = PER_5_KILOS;
    break;
  default:
    break;
  }
  return retText;
}

std::string
CarryOnBaggageTextFormatter::getWeightPiecesText(const OptionalServicesInfo* s7) const
{
  std::string retText = EMPTY_STRING();
  if (s7)
  {
    retText += DASH;

    std::string unit;
    int32_t quantity;

    if (BaggageTextFormatter::populatePieces(s7, quantity, unit))
    {
      std::ostringstream oss;
      oss.fill('0');
      oss << std::setw(2) << quantity;
      retText += oss.str() + unit + SLASH;
    }

    if (BaggageTextFormatter::populateWeight(s7, quantity, unit))
    {
      std::ostringstream oss;
      oss.fill('0');
      oss << std::setw(2) << quantity;
      retText += oss.str() + unit + SLASH;
    }
  }
  return retText;
}

std::string
CarryOnBaggageTextFormatter::getOperatingCarrierText(const BaggageTravel* bgTravel)
{
  return static_cast<AirSeg*>(*bgTravel->_MSS)->operatingCarrierCode();
}

void
CarryOnBaggageTextFormatter::getServiceSubCodes(const std::vector<std::string>& txtMsgs,
                                                std::map<ServiceSubTypeCode, int>& result) const
{
  std::multimap<ServiceSubTypeCode, int> parsedMap;
  FreeBaggageUtil::getServiceSubCodes(txtMsgs, parsedMap);
  ServiceSubTypeCode serviceSubTypeCode;
  int pieces;

  BOOST_FOREACH(std::tie(serviceSubTypeCode, pieces), parsedMap)
  {
    if (pieces > 0)
      result[serviceSubTypeCode] = pieces;
  }
}

std::string
CarryOnBaggageTextFormatter::getCommercialNameText(const SubCodeInfo* s5, int pieces) const
{
  std::string retText;

  std::ostringstream oss;
  oss.fill('0');
  oss << std::setw(2) << pieces;
  retText += oss.str() + CarryOnBaggageTextFormatter::SLASH;

  if (pieces > 1)
    retText += CarryOnBaggageTextFormatter::EACH_PIECE + CarryOnBaggageTextFormatter::SPACE;

  retText += s5->commercialName() + BaggageTextFormatter::NEW_LINE;

  return retText;
}

std::string
CarryOnBaggageTextFormatter::getDescriptionsText(const OptionalServicesInfo* s7) const
{
  T196DiagHandler emptyHandler;
  return getDescriptionsText(s7, emptyHandler);
}

std::string
CarryOnBaggageTextFormatter::getDescriptionsText(const OptionalServicesInfo* s7,
                                                 T196DiagHandler& t196DiagHandler) const
{
  std::string retText;
  if (s7 && s7->freeBaggagePcs() > 0)
  {
    const TaxText* taxText = _trx.dataHandle().getTaxText(s7->vendor(), s7->taxTblItemNo());
    if (taxText)
    {
      std::map<ServiceSubTypeCode, int> subCodesMap;
      getServiceSubCodes(taxText->txtMsgs(), subCodesMap);

      FreeBaggageUtil::CarryOnAllowanceS5RecordsForTable196Strategy s5ForT196Strategy(s7->carrier(),
                                                                                      _trx);

      ServiceSubTypeCode serviceSubTypeCode;
      int pieces;

      BOOST_FOREACH(std::tie(serviceSubTypeCode, pieces), subCodesMap)
      {
        const SubCodeInfo* s5 = s5ForT196Strategy(serviceSubTypeCode);
        if (s5)
        {
          retText += getS5DescriptionsText(s5, pieces);
          t196DiagHandler(s5);
        }
      }

      t196DiagHandler.flush(taxText, s7);
    }
  }
  return retText;
}

std::string
CarryOnBaggageTextFormatter::getServiceSubGroupText(const SubCodeInfo* s5) const
{
  std::string result;

  if (!s5->serviceSubGroup().empty() && !s5->serviceSubGroup().equalToConst("CY"))
  {
    if (s5->serviceGroup().equalToConst("PT"))
      result += PET + SPACE;

    const ServicesSubGroup* servicesSubGroup =
        _trx.dataHandle().getServicesSubGroup(s5->serviceGroup(), s5->serviceSubGroup());

    if (servicesSubGroup)
      result += servicesSubGroup->definition() + SLASH;
  }
  return result;
}

std::string
CarryOnBaggageTextFormatter::getS5DescriptionsText(const SubCodeInfo* s5, int pieces) const
{
  std::string retText;
  if (!s5->description1().empty())
  {
    const ServicesDescription* svcDesc1 =
        _trx.dataHandle().getServicesDescription(s5->description1());
    if (svcDesc1)
    {
      std::ostringstream oss;
      oss.fill('0');
      oss << std::setw(2) << pieces;
      retText += oss.str() + SLASH;

      if (pieces > 1)
        retText += EACH_PIECE + SPACE;
      retText += getServiceSubGroupText(s5) + svcDesc1->description();

      if (!s5->description2().empty())
      {
        const ServicesDescription* svcDesc2 =
            _trx.dataHandle().getServicesDescription(s5->description2());
        if (svcDesc2)
        {
          retText += ((FreeBaggageUtil::isAlpha(svcDesc1->description()) ||
                       FreeBaggageUtil::isAlpha(svcDesc2->description()))
                          ? SPACE
                          : AND);
          retText += svcDesc2->description();
        }
        else
          return getCommercialNameText(s5, pieces);
      }
      retText += BaggageTextFormatter::NEW_LINE;
      return retText;
    }
  }

  return getCommercialNameText(s5, pieces);
}
} // tse
