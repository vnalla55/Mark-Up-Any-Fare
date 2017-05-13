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

#include "DataModel/Common/Codes.h"
#include "DataModel/Common/Types.h"
#include "DataModel/Services/TaxReissue.h"
#include "TaxDisplay/View.h"

#include <map>
#include <vector>

namespace tax
{
namespace display
{
class ResponseFormatter;

class ViewX2TaxReissue : public View
{
public:
  struct ReissueTaxData
  {
    std::shared_ptr<const tax::TaxReissue> taxReissue;
    type::TaxLabel taxLabel;
  };

  struct ReissueViewData
  {
    std::vector<ReissueTaxData> reissueTaxDataVec;
    type::NationName nationName;
  };

  ViewX2TaxReissue(const std::map<type::Nation, ReissueViewData>& data,
                   ResponseFormatter& responseFormatter)
    : View(responseFormatter),
      _data(data) {}

  bool body() override;

private:
  const std::map<type::Nation, ReissueViewData>& _data;
};

} // namespace display
} // namespace tax
