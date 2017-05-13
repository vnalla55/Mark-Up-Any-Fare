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

#include <boost/function.hpp>

namespace tse
{
class AncillaryPricingTrx;
class Itin;
class OCFees;
class OptionalServicesInfo;
class PricingTrx;
class SubCodeInfo;


class FreeBaggageUtil
{
public:
  class S5MatchLogic
  {
  public:
    S5MatchLogic(const std::vector<ServiceSubTypeCode>& subCodes);

    bool isFirstConditionOk(const SubCodeInfo* s5) const;
    static bool isSecondConditionOk(const SubCodeInfo* s5);
    bool isMatched(const SubCodeInfo* s5) const;

  private:
    const std::vector<ServiceSubTypeCode>& _subCodes;
  };

  class S5RecordsRetriever
  {
    friend class FreeBaggageUtilTest;

  public:
    struct S5Comparator : std::binary_function<const SubCodeInfo*, const SubCodeInfo*, bool>
    {
      bool operator()(const SubCodeInfo* s5first, const SubCodeInfo* s5second) const;
    };

  public:
    S5RecordsRetriever(const boost::function<bool(const SubCodeInfo* const)>& acceptS5Condition,
                       const CarrierCode& carrier,
                       const PricingTrx& trx);

    void get(std::vector<const SubCodeInfo*>& containerForFilteredS5s) const;

  protected:
    std::set<SubCodeInfo*, S5Comparator> _filteredSubCodes;
  };

  class CarryOnAllowanceS5RecordsForTable196Strategy : public S5RecordsRetriever
  {
  public:
    CarryOnAllowanceS5RecordsForTable196Strategy(const CarrierCode& carrier, const PricingTrx& trx);
    const SubCodeInfo* operator()(const ServiceSubTypeCode& subcodeType) const;
  };

public:
  static const unsigned int BaggageTagTotalSize = 14;
  static const unsigned int BaggageTagPaxCodeSize = 3;
  static const unsigned int BaggageTagPaxHeadSize = BaggageTagTotalSize - BaggageTagPaxCodeSize;
  static const std::string BaggageTagHead;

  explicit FreeBaggageUtil();

  static bool isAlpha(const std::string& str);

  static void getServiceSubCodes(const std::vector<std::string>& txtMsgs,
                                 std::multimap<ServiceSubTypeCode, int>& result);

  static void getServiceSubCodes(PricingTrx& trx,
                                 const OptionalServicesInfo* s7,
                                 std::multimap<ServiceSubTypeCode, int>& result);

  static void getServiceSubCodes(PricingTrx& trx,
                                 const OptionalServicesInfo* s7,
                                 std::vector<ServiceSubTypeCode>& result);

  static const SubCodeInfo* getS5Record(const CarrierCode& carrier,
                                        const std::vector<ServiceSubTypeCode>& subCodes,
                                        const PricingTrx& trx);

  static const SubCodeInfo* getS5RecordCarryOn(const CarrierCode& carrier,
                                               ServiceSubTypeCode subCodeType,
                                               const PricingTrx& trx);

  static void getS5Records(const VendorCode& vendor,
                           const CarrierCode& carrier,
                           std::vector<SubCodeInfo*>& s5vector,
                           const PricingTrx& trx);

  static const std::vector<SubCodeInfo*>&
  retrieveS5Records(const VendorCode& vendor, const CarrierCode& carrier, const PricingTrx& trx);

  static bool isSortNeeded(const AncillaryPricingTrx* trx);

  static bool isItBaggageDataTransaction(const AncillaryPricingTrx* trx);

  static uint32_t calcFirstChargedPiece(const OCFees* allowance);
  static bool matchOccurrence(const OptionalServicesInfo& s7, int32_t bagNo);

  static bool isItinHasEnoughKnownCharges(const PricingTrx& trx, Itin& itin);

protected:
  static const SubCodeInfo* getS5Record(const VendorCode& vendor,
                                        const CarrierCode& carrier,
                                        const std::vector<ServiceSubTypeCode>& subCodes,
                                        const PricingTrx& trx);
};

} /* namespace tse */
