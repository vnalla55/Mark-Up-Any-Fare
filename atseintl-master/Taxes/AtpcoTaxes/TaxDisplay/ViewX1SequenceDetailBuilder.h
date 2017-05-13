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
#include "TaxDisplay/ViewX1TaxDetail.h"
#include "TaxDisplay/ViewX1SequenceDetail.h"

namespace tax
{
class Services;

namespace display
{
class TaxDisplayRequest;

class ViewX1SequenceDetailBuilder
{
public:
  ViewX1SequenceDetailBuilder(const TaxDisplayRequest& request,
                              const Services& services) :
                                _request(request),
                                _services(services) {}

  ~ViewX1SequenceDetailBuilder() = default;

  std::unique_ptr<ViewX1SequenceDetail> build(const ViewX1TaxDetail::DataSet& taxDetailData,
                                              ResponseFormatter& formatter,
                                              DetailEntryNo entryNo);

  std::unique_ptr<ViewX1SequenceDetail> build(const RulesRecord& rulesRecord,
                                              ResponseFormatter& formatter);

private:
  void setCategories(const RulesRecord& rulesRecord, ViewX1SequenceDetail& view);

  const TaxDisplayRequest& _request;
  const Services& _services;
};

} // namespace display
} // namespace tax
