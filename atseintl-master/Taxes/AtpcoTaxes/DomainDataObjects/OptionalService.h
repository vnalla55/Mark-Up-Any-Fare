// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include <boost/optional.hpp>

#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/TaxInclIndProcessor.h"

namespace tax
{
class BusinessRule;
class Services;

class OptionalService
{
public:
  OptionalService(void);
  ~OptionalService(void);

  type::Index& index() { return _index; }
  const type::Index& index() const { return _index; }

  type::OcSubCode& subCode() { return _subCode; }
  const type::OcSubCode& subCode() const { return _subCode; }

  type::ServiceGroupCode& serviceGroup() { return _serviceGroup; }
  const type::ServiceGroupCode& serviceGroup() const { return _serviceGroup; }

  type::ServiceGroupCode& serviceSubGroup() { return _serviceSubGroup; }
  const type::ServiceGroupCode& serviceSubGroup() const { return _serviceSubGroup; }

  type::OptionalServiceTag& type() { return _type; }
  const type::OptionalServiceTag& type() const { return _type; }

  type::MoneyAmount& amount() { return _amount; }
  const type::MoneyAmount& amount() const { return _amount; }

  void setQuantity(unsigned int quantity) { _quantity = quantity; }
  unsigned int getQuantity() { return _quantity; }

  type::MoneyAmount& feeAmountInSellingCurrencyPlusTaxes()
  {
    return _feeAmountInSellingCurrencyPlusTaxes;
  }

  const type::MoneyAmount& feeAmountInSellingCurrencyPlusTaxes() const
  {
    return _feeAmountInSellingCurrencyPlusTaxes;
  }

  type::MoneyAmount& taxAmount() { return _taxAmount; }
  const type::MoneyAmount& taxAmount() const { return _taxAmount; }

  type::CarrierCode& ownerCarrier() { return _ownerCarrier; }
  const type::CarrierCode& ownerCarrier() const { return _ownerCarrier; }

  type::AirportCode& pointOfDeliveryLoc() { return _pointOfDeliveryLoc; }
  const type::AirportCode& pointOfDeliveryLoc() const { return _pointOfDeliveryLoc; }

  type::PassengerCode& outputPtc() { return _outputPtc; }
  const type::PassengerCode& outputPtc() const { return _outputPtc; }

  void setFailedRule(BusinessRule const* failedRule) { _failedRule = failedRule; }
  bool isFailed() const { return _isDuplicated || _failedRule != nullptr; }

  void setTaxEquivalentAmount(const type::MoneyAmount& value) { _taxEquivalentAmount = value; }
  const type::MoneyAmount& getTaxEquivalentAmount() const
  {
    return _taxEquivalentAmount.is_initialized() ? *_taxEquivalentAmount : _taxAmount;
  }

  void setTaxPointBegin(const Geo& begin);
  void setTaxPointEnd(const Geo& end);

  void setTaxPointLoc2(const Geo& geo);
  void setTaxPointLoc3(const Geo& geo);

  void setDuplicated() { _isDuplicated = true; }

  std::string getFailureReason(Services& services) const;

  const Geo& getTaxPointBegin() const;
  const Geo& getTaxPointEnd() const;
  const Geo& getTaxPointLoc2() const;

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

  type::SabreTaxCode& sabreTaxCode() { return _sabreTaxCode; }
  const type::SabreTaxCode& sabreTaxCode() const { return _sabreTaxCode; }

  bool& taxInclInd() { return _taxInclInd; }
  const bool& taxInclInd() const { return _taxInclInd; }

  TaxAndRounding& taxAndRounding() { return _taxAndRounding; }
  const TaxAndRounding& taxAndRounding() const { return _taxAndRounding; }

  void includeTax(const TaxAndRounding& taxAndRounding,
                  const TaxRoundingInfoService& taxRoundingInfoService);

  bool checkBackingOutTaxes() { return static_cast<bool>(_taxInclIndProcessor); }

private:
  type::Index _index;
  type::MoneyAmount _amount;
  type::MoneyAmount _feeAmountInSellingCurrencyPlusTaxes;
  type::MoneyAmount _taxAmount;
  boost::optional<type::MoneyAmount> _taxEquivalentAmount;
  type::OcSubCode _subCode;
  type::ServiceGroupCode _serviceGroup;
  type::ServiceGroupCode _serviceSubGroup;
  type::OptionalServiceTag _type;
  type::CarrierCode _ownerCarrier;
  type::AirportCode _pointOfDeliveryLoc;
  type::PassengerCode _outputPtc;
  const Geo* _taxPointBegin;
  const Geo* _taxPointEnd;
  // Temporary addition until OCs are separated as tax subject
  const Geo* _taxPointLoc2;
  const Geo* _taxPointLoc3;
  BusinessRule const* _failedRule;
  type::SabreTaxCode _sabreTaxCode; // guaranteed to be 3 or less characters
  bool _taxInclInd;
  boost::optional<TaxInclIndProcessor> _taxInclIndProcessor;
  TaxAndRounding _taxAndRounding;
  bool _isDuplicated;
  unsigned int _quantity;
};
} // namespace tax
