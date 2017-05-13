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

#include "Common/FreeBaggageUtil.h"
#include "FreeBagService/BaggageTextFormatter.h"

#include <string>
#include <vector>

namespace tse
{
class PricingTrx;
class BaggageTravel;
class BaggageCharge;
class Diag852Collector;
class OptionalServicesInfo;
class FarePath;
class SubCodeInfo;
class TaxText;

class CarryOnBaggageTextFormatter : public BaggageTextFormatter
{

  class T196DiagHandler
  {
  public:
    T196DiagHandler();
    T196DiagHandler(Diag852Collector* diag852,
                    const std::vector<uint32_t>& portionsToCheck,
                    const OptionalServicesInfo* s7,
                    const FarePath* farePath,
                    const PricingTrx& trx);

    void operator()(const SubCodeInfo* s5);

    void flush(const TaxText* taxText, const OptionalServicesInfo* s7);

  private:
    Diag852Collector* _diag852;
    std::vector<const SubCodeInfo*> _matchedS5s;
  };

public:
  class TravelDisclosureData
  {
  public:
    TravelDisclosureData(const BaggageTravel* bt,
                         const std::string& travelText,
                         uint32_t portionIndex);

    const BaggageTravel* getBagTravel() const;
    const std::string& getTravelText() const;
    const std::vector<uint32_t>& getPortions() const;
    void appendTravelTextAndPortion(const std::string& textToApend, uint32_t portionIndex);

  private:
    const BaggageTravel* _bt;
    std::string _travelText;
    std::vector<uint32_t> _matchingPortionsIndexes;
  };

  friend class CarryOnBaggageTextFormatterTest;

public:
  static const std::string CARRY_ON_CHARGES_UNKNOWN;
  static const std::string CARRY_ON_ALLOWANCE_UNKNOWN;
  static const std::string TO;
  static const std::string EACH_AND_ABOVE;
  static const std::string OVER;
  static const std::string PER_KILO;
  static const std::string PER_5_KILOS;
  static const std::string KILOS;
  static const std::string POUNDS;
  static const std::string SPACE;
  static const std::string NIL;
  static const std::string AND;
  static const std::string PET;
  static const std::string EACH_PIECE;
  static const std::string ADDITIONAL;

  CarryOnBaggageTextFormatter(PricingTrx& trx);

  std::string formatCarryOnChargeText(const BaggageCharge* baggageCharge,
                                      const bool singleS7Matched = false) const;
  std::string formatCarryOnAllowanceText(const TravelDisclosureData& data,
                                         Diag852Collector* t196TablePrinter) const;

  void getServiceSubCodes(const std::vector<std::string>& txtMsgs,
                          std::map<ServiceSubTypeCode, int>& result) const;
  static std::string getOperatingCarrierText(const BaggageTravel* baggageTravel);
  std::string getDescriptionsText(const OptionalServicesInfo* s7) const;

protected:
  std::string
  getDescriptionsText(const OptionalServicesInfo* s7, T196DiagHandler& t196DiagHandler) const;

  std::string getCarryOnChargeAmountAndCurrencyText(const BaggageCharge* baggageCharge) const;
  static std::string
  getChargeOccurrenceText(const OptionalServicesInfo* s7, const bool singleS7Matched = false);
  static std::string getOrdinalSuffixText(const int32_t ordinal);
  static std::string getExcessBaggageWeightText(const OptionalServicesInfo* s7);
  std::string getCarryOnChargeDescriptionText(const SubCodeInfo* s5) const;
  static std::string getChargeApplicactionText(const OptionalServicesInfo* s7);

  std::string getWeightPiecesText(const OptionalServicesInfo* s7) const;
  std::string getS5DescriptionsText(const SubCodeInfo* s5, int pieces) const;
  std::string getCommercialNameText(const SubCodeInfo* s5, int pieces) const;
  std::string getServiceSubGroupText(const SubCodeInfo* s5) const;

private:
  bool hasTaxTable(const OptionalServicesInfo* s7) const;
};
} // tse

