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
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{
class BaggageTravel;
class PricingTrx;
class OptionalServicesInfo;
class Diag852Collector;
class BaggageCharge;
class SubCodeInfo;
class Money;

class BaggageTextFormatter
{
  friend class BaggageTextFormatterTest;

public:
  static const std::string CARRY_ON_ALLOWANCE;
  static const std::string CARRY_ON_CHARGES;

  static const std::string UNKNOWN_FEE_MSG;
  static const std::string CARRY_ON_CHARGES_UNKNOWN;
  static const std::string EACH_PIECE;
  static const std::string AND;
  static const std::string NOT_PERMITTED;

  static const std::string NO_ALLOWANCE_DATA;
  static const std::string UNKNOWN_INDICATOR;
  static const std::string TWO_ASTERISKS;

  static const Indicator POUNDS_UNIT;
  static const Indicator POUNDS_UNIT_CODE;
  static const std::string POUNDS_UNIT_CODE_LONG;
  static const std::string KILOGRAMS_UNIT_CODE_LONG;
  static const Indicator PIECES_UNIT_CODE;

  static const Indicator NOT_PERMITTED_IND;

  static constexpr char NEW_LINE = '\n';
  static constexpr char DASH = '-';
  static constexpr char SLASH = '/';

  BaggageTextFormatter(PricingTrx& trx, Diag852Collector* diag = nullptr);

  static std::string formatPQAllowanceText(const OptionalServicesInfo* s7, bool isUsDot);
  std::string
  formatAllowanceText(const OptionalServicesInfo* s7, const std::string& carrierText) const;
  std::string formatChargeText(const BaggageCharge* baggageCharge,
                               const std::string& carrierText,
                               bool& addFeesApplyAtEachCheckInText,
                               bool& addAdditionalAllowancesMayApplyText) const;

  static std::string getTravelText(const BaggageTravel* baggageTravel);
  static std::string
  getCarrierText(const PricingTrx& trx, const BaggageTravel* baggageTravel, bool isUsDot);

  const SubCodeInfo*
  getS5Record(const CarrierCode& carrier, const std::vector<ServiceSubTypeCode>& subCodes) const;

protected:
  std::string formatAllowanceValueText(const OptionalServicesInfo* s7) const;
  std::string getDescriptionText(const SubCodeInfo* s5) const;
  std::string getChargeAmountAndCurrencyText(const BaggageCharge* baggageCharge) const;
  std::string
  getChargeAmountAndCurrencyText(const OptionalServicesInfo* s7, const Money& money) const;
  static bool populatePQWeight(const OptionalServicesInfo* s7, int32_t& quantity, Indicator& unit);
  static bool populatePQPieces(const OptionalServicesInfo* s7, int32_t& quantity, Indicator& unit);
  static bool populateWeight(const OptionalServicesInfo* s7, int32_t& quantity, std::string& unit);
  static bool populatePieces(const OptionalServicesInfo* s7, int32_t& quantity, std::string& unit);
  bool shouldAddApplicabilityIndicator(const OptionalServicesInfo* s7) const;
  bool shouldAddApplicabilityIndicator(const BaggageCharge* baggageCharge) const;
  std::string getAllowanceDescription(const OptionalServicesInfo* s7) const;
  std::string getChargeDescription(const OptionalServicesInfo* s7) const;
  const std::vector<SubCodeInfo*>&
  retrieveS5Records(const VendorCode& vendor, const CarrierCode& carrier) const;
  const SubCodeInfo* getS5Record(const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const std::vector<ServiceSubTypeCode>& subCodes) const;
  const SubCodeInfo* getS5Record(const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const ServiceSubTypeCode& subCode) const;
  const SubCodeInfo*
  getS5Record(const CarrierCode& carrier, const ServiceSubTypeCode& subCode) const;
  void getServiceSubCodes(int32_t freeBaggagePcs,
                          const std::vector<std::string>& txtMsgs,
                          std::vector<ServiceSubTypeCode>& result) const;

  PricingTrx& _trx;
  Diag852Collector* _diag;
};
} // tse

