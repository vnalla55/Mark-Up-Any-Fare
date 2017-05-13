// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Taxes/Common/TaxOnTaxFilter.h"
#include <boost/core/noncopyable.hpp>

namespace tse
{
class Trx;
class TaxResponse;
class TaxItem;
class CalculationDetails;
class TaxSplitDetails;


class TaxOnTaxCommon : private boost::noncopyable
{
  friend class TaxOnTaxTest;
public:
  static constexpr char YES = 'Y';
  static constexpr char PERCENTAGE = 'P';

  TaxOnTaxCommon(bool skipTaxOnTaxIfNoFare,
                 bool requireTaxOnTaxSeqMatch,
                 bool requireTaxOnDomTaxMatch,
                 std::pair<uint16_t, uint16_t> indexRange,
                 CalculationDetails& _calculationDetails,
                 TaxSplitDetails& _taxSplitDetails);

  static bool
  useTaxOnTax(TaxTypeCode taxType,
              const std::vector<std::string>& taxOnTaxCode,
              Indicator taxOnTaxExcl);

  void
  calculateTaxOnTax(Trx& trx,
                    TaxResponse& taxResponse,
                    MoneyAmount& taxAmount,
                    MoneyAmount& taxableFare,
                    const NationCode& nation,
                    const std::vector<std::string>& taxOnTaxCode,
                    const Indicator& taxOnTaxExcl,
                    const MoneyAmount& taxAmt,
                    std::vector<TaxItem*>& taxOnTaxItems,
                    bool taxOnlyTaxesFromCurrentSegment = false,
                    const bool isShoppingTaxRequest = false);

  std::pair<uint16_t, uint16_t>
  getIndexRange() const
  {
    return _indexRange;
  }

  bool
  hasTaxOnTaxSeqMatched(const TaxItem* item) const;

  bool
  hasTaxOnDomTaxMatched(const TaxResponse& taxResponse,
                        const NationCode& nation,
                        const TaxItem* item) const;

  bool
  validateShoppingRestrictions(const std::string& taxCode,
                               const bool isShoppingTaxRequest) const;

  MoneyAmount
  calculateTaxFromTaxItem(const TaxResponse& taxResponse,
                          const std::vector<std::string>& taxOnTaxCode,
                          const NationCode& nation,
                          std::vector<TaxItem*>& taxOnTaxItems,
                          bool taxOnlyTaxesFromCurrentSegment,
                          const bool isShoppingTaxRequest = false) const;

  TaxOnTaxFilter& filter() { return _filter; };
  const TaxOnTaxFilter& filter() const { return _filter; };

private:

  bool _skipTaxOnTaxIfNoFare;
  bool _requireTaxOnTaxSeqMatch;
  bool _requireTaxOnDomTaxMatch;
  std::pair<uint16_t, uint16_t> _indexRange;
  CalculationDetails& _calculationDetails;
  TaxSplitDetails& _taxSplitDetails;
  TaxOnTaxFilter _filter;

  void
  calculateTaxAmtAndTaxableFare(Trx& trx,
                                MoneyAmount moneyAmount,
                                Indicator taxOnTaxExcl,
                                MoneyAmount& taxAmount,
                                MoneyAmount& taxableFare,
                                const MoneyAmount& taxAmt,
                                const bool isShoppingTaxRequest = false) const;

  bool
  checkTaxCode(const std::string& taxCode, TaxItem* taxItem) const;

  struct ItemCollector
  {
    ItemCollector(const TaxResponse& taxResponse,
                          const NationCode& nation,
                          CalculationDetails& details,
                          std::vector<TaxItem*>& taxOnTaxItems,
                          const bool isShoppingTaxRequest,
                          const TaxOnTaxCommon& taxOnTax,
                          bool taxOnlyTaxesFromCurrentSegment)
      : _taxResponse(taxResponse)
      , _nation(nation)
      , _details(details)
      , _taxOnTaxItems(taxOnTaxItems)
      , _isShoppingTaxRequest(isShoppingTaxRequest)
      , _moneyAmount(0.0)
      , _taxOnTax(taxOnTax)
      , _taxOnlyTaxesFromCurrentSegment(taxOnlyTaxesFromCurrentSegment)
    {}

    void collect(const std::string& taxCode,
                 const std::vector<TaxItem*>& taxesToCheck);

    MoneyAmount getMoneyAmount() const { return _moneyAmount; }

   private:
    const TaxResponse&     _taxResponse;
    const NationCode&      _nation;
    CalculationDetails&    _details;
    std::vector<TaxItem*>& _taxOnTaxItems;
    const bool             _isShoppingTaxRequest;
    MoneyAmount            _moneyAmount;
    const TaxOnTaxCommon&  _taxOnTax;
    bool                   _taxOnlyTaxesFromCurrentSegment;
  };
};

} // end of tse namespace
