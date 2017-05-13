// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "DataModel/Common/Types.h"

#include <tuple>

namespace tax
{

class IoTaxName
{
public:
  IoTaxName()
    : _taxPointTag(type::TaxPointTag::Sale),
      _percentFlatTag(type::PercentFlatTag::Flat),
      _taxRemittanceId(type::TaxRemittanceId::Sale)
  {
  }
  ~IoTaxName() {}

  type::Nation& nation() { return _nation; }

  type::Nation const& nation() const { return _nation; }

  type::TaxCode& taxCode() { return _taxCode; }

  type::TaxCode const& taxCode() const { return _taxCode; }

  type::TaxType& taxType() { return _taxType; }

  type::TaxType const& taxType() const { return _taxType; }

  type::TaxPointTag& taxPointTag() { return _taxPointTag; }

  type::TaxPointTag const& taxPointTag() const { return _taxPointTag; }

  type::PercentFlatTag& percentFlatTag() { return _percentFlatTag; }

  type::PercentFlatTag const& percentFlatTag() const { return _percentFlatTag; }

  type::TaxRemittanceId& taxRemittanceId() { return _taxRemittanceId; }

  type::TaxRemittanceId const& taxRemittanceId() const { return _taxRemittanceId; }

  // Tax remittance id not included in this comparison as it should be constant for a given
  // (taxCode, taxType) pair.
  bool operator<(IoTaxName const& other) const
  {
    return std::make_tuple(_nation, _taxCode, _taxType, _taxPointTag, _percentFlatTag) <
           std::make_tuple(other._nation,
                             other._taxCode,
                             other._taxType,
                             other._taxPointTag,
                             other._percentFlatTag);
  }

  bool operator==(IoTaxName const& other) const
  {
    return std::make_tuple(_nation, _taxCode, _taxType, _taxPointTag, _percentFlatTag) ==
           std::make_tuple(other._nation,
                             other._taxCode,
                             other._taxType,
                             other._taxPointTag,
                             other._percentFlatTag);
  }

private:
  type::Nation _nation;
  type::TaxCode _taxCode;
  type::TaxType _taxType;
  type::TaxPointTag _taxPointTag;
  type::PercentFlatTag _percentFlatTag;
  type::TaxRemittanceId _taxRemittanceId;
};

} // namespace tax
