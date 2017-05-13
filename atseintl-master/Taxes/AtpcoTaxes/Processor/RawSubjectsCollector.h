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

#include "Rules/PaymentDetail.h"

#include <boost/function.hpp>

namespace tax
{
class Geo;
class OptionalService;
class Request;
class Service;

struct RawSubjects
{
  // this should be changed to TaxableItinerary once we get rid of TaxableData from
  // PaymentDetailBase
  TaxSubjectData _itinerary;
  TaxableYqYrs _yqyrs;
};

class RawSubjectsCollector
{
  typedef boost::function<bool(const OptionalService&)> OCTypeChecker;

public:
  RawSubjectsCollector() = default;

  RawSubjectsCollector(const type::ProcessingGroup& processingGroup,
                       const Request& request,
                       const type::Index itinId,
                       const Geo& taxPoint,
                       const Geo& nextPrevTaxPoint);

  const RawSubjects& getSubjects() const { return _subjects; }

  void addChangeFee(const Request& request, const type::Index& itinId);

  void addYqYrs(const Request&, const type::Index, const Geo&, const Geo&, bool onAllBaseFare = false);

  void addOptionalServices(const Request& request,
                           const type::Index itinId,
                           const type::TaxPointTag& tag,
                           const type::ProcessingGroup& processingGroup,
                           const type::Index taxPointBeginId);

  void addTicketingFee(const Request& request, const type::Index& itinId);

private:
  RawSubjects _subjects;
};
}

