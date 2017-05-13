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
#include "Rules/BusinessRule.h"

namespace tax
{
class Request;
class Services;
class TravelDatesJourneyApplicator;
class TravelDatesTaxPointApplicator;

class TravelDatesJourneyRule : public BusinessRule
{
public:
  typedef TravelDatesJourneyApplicator ApplicatorType;

  TravelDatesJourneyRule(type::Date firstTravelDate, type::Date lastTravelDate)
    : _firstTravelDate(firstTravelDate), _lastTravelDate(lastTravelDate) {};
  virtual ~TravelDatesJourneyRule() {};

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& itinPayments) const;

  const type::Date& firstTravelDate() const { return _firstTravelDate; }
  const type::Date& lastTravelDate() const { return _lastTravelDate; }

private:
  type::Date _firstTravelDate;
  type::Date _lastTravelDate;
};

class TravelDatesTaxPointRule : public BusinessRule
{
public:
  typedef TravelDatesTaxPointApplicator ApplicatorType;
  TravelDatesTaxPointRule(type::Date firstTravelDate, type::Date lastTravelDate)
    : _firstTravelDate(firstTravelDate), _lastTravelDate(lastTravelDate) {};
  virtual ~TravelDatesTaxPointRule() {};

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& itinPayments) const;

  const type::Date& firstTravelDate() const { return _firstTravelDate; }
  const type::Date& lastTravelDate() const { return _lastTravelDate; }

private:
  type::Date _firstTravelDate;
  type::Date _lastTravelDate;
};
}
