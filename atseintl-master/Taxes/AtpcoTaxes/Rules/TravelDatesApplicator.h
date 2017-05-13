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

#include "Rules/BusinessRuleApplicator.h"
#include "Common/Timestamp.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Geo.h"
#include "Rules/PaymentDetail.h"

namespace tax
{

class BusinessRule;
class PaymentDetail;

class TravelDatesJourneyApplicator : public BusinessRuleApplicator
{
public:
  TravelDatesJourneyApplicator(const BusinessRule* parent,
                               type::Date firstTravelDate,
                               type::Date lastTravelDate,
                               type::Date travelOriginDate)
    : BusinessRuleApplicator(parent),
      _value(travelOriginDate.is_between(firstTravelDate, lastTravelDate))
  {
  }
  bool apply(PaymentDetail& /*paymentDetail*/) const { return _value; }

private:
  bool _value;
};

class TravelDatesTaxPointApplicator : public BusinessRuleApplicator
{
public:
  TravelDatesTaxPointApplicator(const BusinessRule* parent,
                                type::Date saleDate,
                                type::Date firstTravelDate,
                                type::Date lastTravelDate,
                                const Itin& itin)
    : BusinessRuleApplicator(parent),
      _itin(itin),
      _saleDate(saleDate),
      _firstDate(firstTravelDate),
      _lastDate(lastTravelDate)
  {
  }
  bool apply(PaymentDetail& paymentDetail) const;

private:
  const Itin& _itin;
  const type::Date _saleDate;
  const type::Date _firstDate;
  const type::Date _lastDate;
};
}
