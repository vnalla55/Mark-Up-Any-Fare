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
#include "FreeBagService/BaggageTextFormatter.h"

#include "Common/BaggageTripType.h"
#include "Common/FreeBaggageUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/ServicesDescription.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/TaxText.h"
#include "Diagnostic/Diag852Collector.h"
#include "FreeBagService/BaggageTextUtil.h"
#include "ServiceFees/OCFees.h"

#include <tuple>

#include <boost/foreach.hpp>

namespace tse
{
const std::string BaggageTextFormatter::CARRY_ON_ALLOWANCE = "CARRY ON ALLOWANCE";
const std::string BaggageTextFormatter::CARRY_ON_CHARGES = "CARRY ON CHARGES";

const std::string BaggageTextFormatter::UNKNOWN_FEE_MSG =
    "BAGGAGE ALLOWANCES/FEES UNKNOWN - CONTACT ";
const std::string BaggageTextFormatter::CARRY_ON_CHARGES_UNKNOWN =
    "CARRY ON FEES UNKNOWN-CONTACT CARRIER";
const std::string BaggageTextFormatter::EACH_PIECE = "/EACH PIECE ";
const std::string BaggageTextFormatter::AND = " AND ";
const std::string BaggageTextFormatter::NOT_PERMITTED = "NOT PERMITTED";

const std::string BaggageTextFormatter::NO_ALLOWANCE_DATA = "NIL";
const std::string BaggageTextFormatter::UNKNOWN_INDICATOR = "*";
const std::string BaggageTextFormatter::TWO_ASTERISKS = "**";

const Indicator BaggageTextFormatter::POUNDS_UNIT = 'P';
const Indicator BaggageTextFormatter::POUNDS_UNIT_CODE = 'L';
const std::string BaggageTextFormatter::POUNDS_UNIT_CODE_LONG = "LB";
const std::string BaggageTextFormatter::KILOGRAMS_UNIT_CODE_LONG = "KG";
const Indicator BaggageTextFormatter::PIECES_UNIT_CODE = 'P';

const Indicator BaggageTextFormatter::NOT_PERMITTED_IND = 'X';

BaggageTextFormatter::BaggageTextFormatter(PricingTrx& trx, Diag852Collector* diag)
  : _trx(trx), _diag(diag)
{
}

std::string
BaggageTextFormatter::formatPQAllowanceText(const OptionalServicesInfo* s7, bool isUsDot)
{
  std::string allowanceText;
  if (s7)
  {
    int32_t quantity(0);
    Indicator unit;

    if (isUsDot)
    {
      if (!populatePQPieces(s7, quantity, unit))
      {
        populatePQWeight(s7, quantity, unit);
      }
    }
    else
    {
      if (!populatePQWeight(s7, quantity, unit))
      {
        populatePQPieces(s7, quantity, unit);
      }
    }

    if (quantity)
    {
      std::ostringstream allowanceBuffer;
      allowanceBuffer.fill('0');
      allowanceBuffer << std::setw(2) << quantity << unit;

      allowanceText = allowanceBuffer.str();
    }
    else
      allowanceText = NO_ALLOWANCE_DATA;
  }
  return allowanceText;
}

std::string
BaggageTextFormatter::formatAllowanceText(const OptionalServicesInfo* s7,
                                          const std::string& carrierText) const
{
  std::string allowanceText;
  if (s7)
    allowanceText += formatAllowanceValueText(s7);
  else
    allowanceText += UNKNOWN_INDICATOR;

  allowanceText += SLASH + carrierText;

  if (TrxUtil::isBaggageDotPhase2ADisplayEnabled(_trx))
  {
    if (s7)
    {
      const std::string descriptionText = getAllowanceDescription(s7);

      if (!descriptionText.empty())
        allowanceText += EACH_PIECE + descriptionText;
    }
  }
  return allowanceText;
}

std::string
BaggageTextFormatter::formatAllowanceValueText(const OptionalServicesInfo* s7) const
{
  std::string allowanceText;
  if (s7)
  {
    std::ostringstream allowanceBuffer;
    allowanceBuffer.fill('0');

    if (s7->freeBaggagePcs() > 0)
      allowanceBuffer << std::setw(2) << s7->freeBaggagePcs() << PIECES_UNIT_CODE;

    if (s7->baggageWeight() > 0)
    {
      if (allowanceBuffer.tellp() > 0)
        allowanceBuffer << SLASH;

      allowanceBuffer << std::setw(2) << s7->baggageWeight()
                      << ((s7->baggageWeightUnit() == POUNDS_UNIT) ? POUNDS_UNIT_CODE_LONG
                                                                   : KILOGRAMS_UNIT_CODE_LONG);
    }

    allowanceText = allowanceBuffer.str();

    if (allowanceText.empty())
      allowanceText = NO_ALLOWANCE_DATA;
  }
  return allowanceText;
}

std::string
BaggageTextFormatter::formatChargeText(const BaggageCharge* baggageCharge,
                                       const std::string& carrierText,
                                       bool& addFeesApplyAtEachCheckInText,
                                       bool& addAdditionalAllowancesMayApplyText) const
{
  std::string chargeText;

  const OptionalServicesInfo* s7 = baggageCharge ? baggageCharge->optFee() : nullptr;
  Money chargeMoney((baggageCharge ? baggageCharge->feeAmount() : 0),
                    (baggageCharge ? baggageCharge->feeCurrency() : ""));

  chargeText += getChargeAmountAndCurrencyText(baggageCharge) + SLASH + carrierText;

  if (TrxUtil::isBaggageDotPhase2ADisplayEnabled(_trx))
  {
    const std::string descriptionText = getChargeDescription(s7);

    if (!descriptionText.empty())
      chargeText += SLASH + descriptionText;

    if (TrxUtil::isBaggage302GlobalDisclosureActivated(_trx)
            ? shouldAddApplicabilityIndicator(baggageCharge)
            : shouldAddApplicabilityIndicator(s7))
    {
      chargeText += TWO_ASTERISKS;
      addFeesApplyAtEachCheckInText = true;
    }
  }

  if ((s7 && s7->notAvailNoChargeInd() == NOT_PERMITTED_IND) || !chargeMoney.code().empty())
    addAdditionalAllowancesMayApplyText = true;
  else
    chargeText += NEW_LINE + UNKNOWN_INDICATOR + UNKNOWN_FEE_MSG + carrierText;

  return chargeText;
}

std::string
BaggageTextFormatter::getTravelText(const BaggageTravel* baggageTravel)
{
  return (*baggageTravel->getTravelSegBegin())->origin()->loc() +
         (*(baggageTravel->getTravelSegEnd() - 1))->destination()->loc();
}

std::string
BaggageTextFormatter::getCarrierText(const PricingTrx& trx,
                                     const BaggageTravel* baggageTravel,
                                     bool isUsDot)
{
  const AirSeg* carrierTravelSeg = dynamic_cast<const AirSeg*>(baggageTravel->_carrierTravelSeg);
  bool useMarkCxr = true;

  if (carrierTravelSeg)
    useMarkCxr = BaggageTextUtil::isMarketingCxrUsed(trx, isUsDot, baggageTravel->_defer);
  else
  {
    carrierTravelSeg = (**baggageTravel->getTravelSegBegin()).toAirSeg();
    useMarkCxr = BaggageTextUtil::isMarketingCxrUsed(trx, isUsDot, false);
  }

  return (useMarkCxr || carrierTravelSeg->segmentType() == Open)
             ? carrierTravelSeg->marketingCarrierCode()
             : carrierTravelSeg->operatingCarrierCode();
}

std::string
BaggageTextFormatter::getDescriptionText(const SubCodeInfo* s5) const
{
  std::string retText;
  if (s5)
  {
    if (!s5->description1().empty())
    {
      const ServicesDescription* svcDesc =
          _trx.dataHandle().getServicesDescription(s5->description1());
      if (svcDesc)
      {
        retText = svcDesc->description();
        if (!s5->description2().empty())
        {
          svcDesc = _trx.dataHandle().getServicesDescription(s5->description2());
          if (svcDesc)
            retText += AND + svcDesc->description();
        }
      }
    }
  }
  return retText;
}

std::string
BaggageTextFormatter::getChargeAmountAndCurrencyText(const BaggageCharge* baggageCharge) const
{
  const OptionalServicesInfo* s7 = baggageCharge ? baggageCharge->optFee() : nullptr;
  Money chargeMoney((baggageCharge ? baggageCharge->feeAmount() : 0),
                    (baggageCharge ? baggageCharge->feeCurrency() : ""));

  return getChargeAmountAndCurrencyText(s7, chargeMoney);
}

std::string
BaggageTextFormatter::getChargeAmountAndCurrencyText(const OptionalServicesInfo* s7,
                                                     const Money& money) const
{
  std::string retText;
  if (s7 && s7->notAvailNoChargeInd() == NOT_PERMITTED_IND)
    retText = NOT_PERMITTED;
  else if (!money.code().empty())
    retText = money.toString();
  else
    retText = UNKNOWN_INDICATOR;
  return retText;
}

bool
BaggageTextFormatter::populatePQWeight(const OptionalServicesInfo* s7,
                                       int32_t& quantity,
                                       Indicator& unit)
{
  if (s7->baggageWeight() > 0)
  {
    quantity = s7->baggageWeight();
    unit = (s7->baggageWeightUnit() == POUNDS_UNIT) ? POUNDS_UNIT_CODE : s7->baggageWeightUnit();
    return true;
  }
  return false;
}

bool
BaggageTextFormatter::populateWeight(const OptionalServicesInfo* s7,
                                     int32_t& quantity,
                                     std::string& unit)
{
  if (s7->baggageWeight() > 0)
  {
    quantity = s7->baggageWeight();
    unit =
        (s7->baggageWeightUnit() == POUNDS_UNIT) ? POUNDS_UNIT_CODE_LONG : KILOGRAMS_UNIT_CODE_LONG;
    return true;
  }
  return false;
}

bool
BaggageTextFormatter::populatePQPieces(const OptionalServicesInfo* s7,
                                       int32_t& quantity,
                                       Indicator& unit)
{
  if (s7->freeBaggagePcs() > 0)
  {
    quantity = s7->freeBaggagePcs();
    unit = PIECES_UNIT_CODE;
    return true;
  }
  return false;
}

bool
BaggageTextFormatter::populatePieces(const OptionalServicesInfo* s7,
                                     int32_t& quantity,
                                     std::string& unit)
{
  Indicator unitIndicator;
  const bool result = populatePQPieces(s7, quantity, unitIndicator);
  unit += unitIndicator;
  return result;
}

bool
BaggageTextFormatter::shouldAddApplicabilityIndicator(const BaggageCharge* baggageCharge) const
{
  if (baggageCharge && baggageCharge->farePath()->itin()->getBaggageTripType().isUsDot())
  {
    const OptionalServicesInfo* s7 = baggageCharge->optFee();
    return s7 && (s7->frequentFlyerMileageAppl() == '3');
  }
  return false;
}

bool
BaggageTextFormatter::shouldAddApplicabilityIndicator(const OptionalServicesInfo* s7) const
{
  return s7 && (s7->frequentFlyerMileageAppl() == '3');
}

std::string
BaggageTextFormatter::getAllowanceDescription(const OptionalServicesInfo* s7) const
{
  if (s7 && s7->taxTblItemNo() && s7->freeBaggagePcs() > 0)
  {
    const TaxText* taxText = _trx.dataHandle().getTaxText(s7->vendor(), s7->taxTblItemNo());
    if (taxText)
    {
      std::vector<ServiceSubTypeCode> subCodes;
      getServiceSubCodes(s7->freeBaggagePcs(), taxText->txtMsgs(), subCodes);

      if (_diag)
        _diag->printTable196DetailHeader(s7->taxTblItemNo(), taxText->txtMsgs());

      if (!subCodes.empty())
        return getDescriptionText(getS5Record(s7->carrier(), subCodes));
    }
  }

  return "";
}

std::string
BaggageTextFormatter::getChargeDescription(const OptionalServicesInfo* s7) const
{
  return s7 ? getDescriptionText(getS5Record(s7->carrier(), s7->serviceSubTypeCode())) : "";
}

const std::vector<SubCodeInfo*>&
BaggageTextFormatter::retrieveS5Records(const VendorCode& vendor, const CarrierCode& carrier) const
{
  return FreeBaggageUtil::retrieveS5Records(vendor, carrier, _trx);
}

const SubCodeInfo*
BaggageTextFormatter::getS5Record(const VendorCode& vendor,
                                  const CarrierCode& carrier,
                                  const std::vector<ServiceSubTypeCode>& subCodes) const
{
  const std::vector<SubCodeInfo*>& s5records = retrieveS5Records(vendor, carrier);
  const SubCodeInfo* matchedS5 = nullptr;

  FreeBaggageUtil::S5MatchLogic matchLogic(subCodes);
  for (const SubCodeInfo* subCodeInfo : s5records)
  {
    if (matchLogic.isFirstConditionOk(subCodeInfo))
    {
      if (_diag)
        _diag->printTable196Detail(subCodeInfo);

      if (matchLogic.isSecondConditionOk(subCodeInfo))
      {
        if (!matchedS5)
        {
          matchedS5 = subCodeInfo;
          if (_diag)
            _diag->printTable196Detail(true);
          else
            break;
        }
      }
    }
    if (_diag)
      _diag->printTable196Detail(false);
  }

  if (_diag)
    _diag->printTable196DetailEnd(matchedS5);
  return matchedS5;
}

const SubCodeInfo*
BaggageTextFormatter::getS5Record(const CarrierCode& carrier,
                                  const std::vector<ServiceSubTypeCode>& subCodes) const
{
  const SubCodeInfo* s5 = getS5Record(ATPCO_VENDOR_CODE, carrier, subCodes);

  if (!s5)
    s5 = getS5Record(MERCH_MANAGER_VENDOR_CODE, carrier, subCodes);

  return s5;
}

const SubCodeInfo*
BaggageTextFormatter::getS5Record(const VendorCode& vendor,
                                  const CarrierCode& carrier,
                                  const ServiceSubTypeCode& subCode) const
{
  const std::vector<SubCodeInfo*>& s5records = retrieveS5Records(vendor, carrier);

  for (const SubCodeInfo* subCodeInfo : s5records)
  {
    try
    {
      if (boost::lexical_cast<short>(subCodeInfo->description1()) &&
          subCodeInfo->serviceSubTypeCode() == subCode)
      {
        return subCodeInfo;
      }
    }
    catch (boost::bad_lexical_cast&) {}
  }
  return nullptr;
}

const SubCodeInfo*
BaggageTextFormatter::getS5Record(const CarrierCode& carrier, const ServiceSubTypeCode& subCode)
    const
{
  const SubCodeInfo* s5 = getS5Record(ATPCO_VENDOR_CODE, carrier, subCode);

  if (!s5)
    s5 = getS5Record(MERCH_MANAGER_VENDOR_CODE, carrier, subCode);

  return s5;
}

void
BaggageTextFormatter::getServiceSubCodes(int32_t freeBaggagePcs,
                                         const std::vector<std::string>& txtMsgs,
                                         std::vector<ServiceSubTypeCode>& result) const
{
  std::multimap<ServiceSubTypeCode, int> parsedMap;
  FreeBaggageUtil::getServiceSubCodes(txtMsgs, parsedMap);
  ServiceSubTypeCode serviceSubTypeCode;
  int pieces;

  BOOST_FOREACH(std::tie(serviceSubTypeCode, pieces), parsedMap)
  {
    if (pieces == freeBaggagePcs)
      result.push_back(serviceSubTypeCode);
  }
}
} // tse
