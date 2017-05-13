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

namespace tax
{

class TaxName
{
public:
  TaxName()
    : _taxPointTag(type::TaxPointTag::Sale),
      _percentFlatTag(type::PercentFlatTag::Flat),
      _taxRemittanceId(type::TaxRemittanceId::Sale)
  {
  }
  ~TaxName() {}

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

  type::CarrierCode& taxCarrier() { return _taxCarrier; }

  type::CarrierCode const& taxCarrier() const { return _taxCarrier; }

  bool operator<(TaxName const& other) const;
  bool operator==(TaxName const& other) const;
  bool operator!=(TaxName const& other) const { return !operator==(other); }

private:
  type::Nation _nation;
  type::TaxCode _taxCode;
  type::TaxType _taxType;
  type::TaxPointTag _taxPointTag;
  type::PercentFlatTag _percentFlatTag;
  type::TaxRemittanceId _taxRemittanceId;
  type::CarrierCode _taxCarrier;
};

} // tax

