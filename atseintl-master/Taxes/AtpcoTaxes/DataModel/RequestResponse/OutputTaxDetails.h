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

#include <vector>
#include <boost/optional.hpp>

#include "DataModel/RequestResponse/OutputCalcDetails.h"
#include "DataModel/RequestResponse/OutputExchangeReissueDetails.h"
#include "DataModel/RequestResponse/OutputGeoDetails.h"
#include "DataModel/RequestResponse/OutputOptionalServiceDetails.h"
#include "DataModel/RequestResponse/OutputTaxDetailsRef.h"
#include "DataModel/Common/Types.h"

namespace tax
{
struct OutputTotalAmtDetails
{
  OutputTotalAmtDetails() : totalAmt(0) {}

  type::MoneyAmount totalAmt;
};

struct OutputFaresDetails : OutputTotalAmtDetails
{
};
struct OutputYQYRDetails : OutputTotalAmtDetails
{
};
struct OutputOCDetails : OutputTotalAmtDetails
{
};
struct OutputBaggageDetails : OutputTotalAmtDetails
{
};
struct OutputOBDetails : OutputTotalAmtDetails
{
};

struct OutputTaxOnTaxDetails : OutputTotalAmtDetails
{
  std::vector<OutputTaxDetailsRef> taxDetailsRef;
};

class OutputTaxDetails
{
public:
  OutputTaxDetails()
    : _seqNo(0),
      _paymentAmt(0),
      _paymentWithMarkupAmt(0),
      _exemptAmt(0),
      _publishedAmt(0),
      _exempt(false),
      _gst(false),
      _commandExempt(type::CalcRestriction::Blank),
      _percentFlatTag(type::PercentFlatTag::Flat)
  {
  }

  bool operator==(const OutputTaxDetails& r) const
  {
    // if id exists check them else check all atribbutes
    if (_id && r._id)
    {
      return *_id == *r._id;
    }
    else
      return (
          (_seqNo == r._seqNo) && (_paymentAmt == r._paymentAmt) &&
          (_paymentWithMarkupAmt == r._paymentWithMarkupAmt) && (_exemptAmt == r._exemptAmt) &&
          (_publishedAmt == r._publishedAmt) && (_publishedCur == r._publishedCur) &&
          (_taxPointLocBegin == r._taxPointLocBegin) && (_taxPointLocEnd == r._taxPointLocEnd) &&
          (_name == r._name) && (_carrier == r._carrier) && (_gst == r._gst) && (_exempt == r._exempt) &&
          (_commandExempt == r._commandExempt) && (_nation == r._nation) && (_type == r._type) &&
          (_code == r._code) && (_totalFareAmount == r._totalFareAmount) &&
          (_percentFlatTag == r._percentFlatTag) && (_geoDetails == r._geoDetails) &&
          (_calcDetails == r._calcDetails) && (_exchangeDetails == r._exchangeDetails) &&
          (_optionalServiceDetails == r._optionalServiceDetails) &&
          (_optionalServiceId == r._optionalServiceId) && (_taxableUnitTags == r._taxableUnitTags));
  }

  const boost::optional<OutputCalcDetails>& calcDetails() const { return _calcDetails; }
  boost::optional<OutputCalcDetails>& calcDetails() { return _calcDetails; }

  const boost::optional<OutputExchangeReissueDetails>& exchangeDetails() const
  {
    return _exchangeDetails;
  }
  boost::optional<OutputExchangeReissueDetails>& exchangeDetails() { return _exchangeDetails; }

  const boost::optional<OutputGeoDetails>& geoDetails() const { return _geoDetails; }
  boost::optional<OutputGeoDetails>& geoDetails() { return _geoDetails; }

  const boost::optional<OutputOptionalServiceDetails>& optionalServiceDetails() const
  {
    return _optionalServiceDetails;
  }
  boost::optional<OutputOptionalServiceDetails>& optionalServiceDetails()
  {
    return _optionalServiceDetails;
  }

  const boost::optional<type::Index>& id() const { return _id; }
  boost::optional<type::Index>& id() { return _id; }

  const type::SeqNo& seqNo() const { return _seqNo; }
  type::SeqNo& seqNo() { return _seqNo; }

  const type::MoneyAmount& paymentAmt() const { return _paymentAmt; }
  type::MoneyAmount& paymentAmt() { return _paymentAmt; }

  const type::MoneyAmount& paymentWithMarkupAmt() const { return _paymentWithMarkupAmt; }
  type::MoneyAmount& paymentWithMarkupAmt() { return _paymentWithMarkupAmt; }

  const type::MoneyAmount& exemptAmt() const { return _exemptAmt; }
  type::MoneyAmount& exemptAmt() { return _exemptAmt; }

  const type::MoneyAmount& publishedAmt() const { return _publishedAmt; }
  type::MoneyAmount& publishedAmt() { return _publishedAmt; }

  const type::CurrencyCode& publishedCur() const { return _publishedCur; }
  type::CurrencyCode& publishedCur() { return _publishedCur; }

  const type::AirportCode& taxPointLocBegin() const { return _taxPointLocBegin; }
  type::AirportCode& taxPointLocBegin() { return _taxPointLocBegin; }

  const type::AirportCode& taxPointLocEnd() const { return _taxPointLocEnd; }
  type::AirportCode& taxPointLocEnd() { return _taxPointLocEnd; }

  const type::TaxLabel& name() const { return _name; }
  type::TaxLabel& name() { return _name; }

  const type::CarrierCode& carrier() const { return _carrier; }
  type::CarrierCode& carrier() { return _carrier; }

  const type::Nation& nation() const { return _nation; }
  type::Nation& nation() { return _nation; }

  const type::TaxCode& code() const { return _code; }
  type::TaxCode& code() { return _code; }

  const type::SabreTaxCode& sabreCode() const { return _sabreCode; }
  type::SabreTaxCode& sabreCode() { return _sabreCode; }

  const type::TaxType& type() const { return _type; }
  type::TaxType& type() { return _type; }

  const bool& exempt() const { return _exempt; }
  bool& exempt() { return _exempt; }

  const bool& gst() const { return _gst; }
  bool& gst() { return _gst; }

  const type::CalcRestriction& commandExempt() const { return _commandExempt; }
  type::CalcRestriction& commandExempt() { return _commandExempt; }

  const std::vector<type::TaxableUnit>& taxableUnitTags() const { return _taxableUnitTags; }
  std::vector<type::TaxableUnit>& taxableUnitTags() { return _taxableUnitTags; }

  const boost::optional<tax::type::Index>& optionalServiceId() const { return _optionalServiceId; }
  boost::optional<tax::type::Index>& optionalServiceId() { return _optionalServiceId; }

  type::MoneyAmount& totalFareAmount() { return _totalFareAmount; }
  type::MoneyAmount const& totalFareAmount() const { return _totalFareAmount; }

  type::PercentFlatTag& percentFlatTag() { return _percentFlatTag; }
  type::PercentFlatTag const& percentFlatTag() const { return _percentFlatTag; }

private:
  OutputTaxDetails& operator=(const OutputTaxDetails&);

  boost::optional<OutputCalcDetails> _calcDetails;
  boost::optional<OutputExchangeReissueDetails> _exchangeDetails;
  boost::optional<OutputGeoDetails> _geoDetails;
  boost::optional<OutputOptionalServiceDetails> _optionalServiceDetails;
  boost::optional<type::Index> _id;
  type::SeqNo _seqNo;
  type::MoneyAmount _paymentAmt;
  type::MoneyAmount _paymentWithMarkupAmt;
  type::MoneyAmount _exemptAmt;
  type::MoneyAmount _publishedAmt;
  type::CurrencyCode _publishedCur;
  type::AirportCode _taxPointLocBegin;
  type::AirportCode _taxPointLocEnd;
  type::TaxLabel _name;
  type::CarrierCode _carrier;
  type::Nation _nation;
  type::TaxCode _code;
  type::SabreTaxCode _sabreCode;
  type::TaxType _type;
  bool _exempt;
  bool _gst;
  type::CalcRestriction _commandExempt;
  type::MoneyAmount _totalFareAmount;
  type::PercentFlatTag _percentFlatTag;
  std::vector<type::TaxableUnit> _taxableUnitTags;
  boost::optional<tax::type::Index> _optionalServiceId;

public:
  boost::optional<OutputFaresDetails> taxOnFaresDetails;
  boost::optional<OutputTaxOnTaxDetails> taxOnTaxDetails;
  boost::optional<OutputYQYRDetails> taxOnYqYrDetails;
  boost::optional<OutputOCDetails> taxOnOcDetails;
  boost::optional<OutputBaggageDetails> taxOnBaggageDetails;
  boost::optional<OutputOBDetails> obDetails;
};

} // namespace tax
