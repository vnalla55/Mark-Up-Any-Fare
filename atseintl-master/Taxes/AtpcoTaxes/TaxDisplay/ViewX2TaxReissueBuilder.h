// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "TaxDisplay/Common/Types.h"
#include "TaxDisplay/ViewX2TaxReissue.h"

#include <set>

namespace tax
{
class Services;

namespace display
{
class ResponseFormatter;
class TaxDisplayRequest;

class ViewX2TaxReissueBuilder
{
public:
  ViewX2TaxReissueBuilder(const Services& services) : _services(services) {}
  ~ViewX2TaxReissueBuilder() = default;

  std::map<type::Nation, ViewX2TaxReissue::ReissueViewData>
  getData(const TaxDisplayRequest& request,
          const ReportingRecordsDataSet& reportingRecords);

  bool hasGotAnyReissues() const { return _hasGotAnyReissues; }

private:
  const Services& _services;
  bool _hasGotAnyReissues{false};
};

} // namespace display
} // namespace tax
