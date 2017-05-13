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

#include "Common/FilterPredicates.h"
#include "Common/GroupedContainers.h"
#include "Common/TaxName.h"
#include "Common/Timestamp.h"
#include "DataModel/Common/Codes.h"
#include "DataModel/Common/SafeEnums.h"
#include "Rules/BusinessRulesContainer.h"
#include "Util/BranchPrediction.h"

#include <functional>
#include <memory>
#include <vector>

namespace tax
{

class TaxData : public GroupedContainers<std::vector<std::shared_ptr<BusinessRulesContainer>> >
{
public:
  TaxData(const TaxName& taxName, const type::Vendor& vendor) :
    _taxName(taxName),
    _vendor(vendor) {}

  const TaxName& getTaxName() const { return _taxName; }
  const type::Vendor& getVendor() const { return _vendor; }

  std::vector<std::shared_ptr<BusinessRulesContainer>>
  getDateFilteredCopy(const Key& key, type::Timestamp ticketingDate) const
  {
    auto predicate = std::bind(validBusinessRuleDatePredicate, std::placeholders::_1, ticketingDate);
    return getFilteredCopy(key, predicate);
  }

  std::vector<std::shared_ptr<BusinessRulesContainer>>::difference_type
  getDateFilteredSize(const Key& key, type::Timestamp ticketingDate) const
  {
    auto predicate = std::bind(validBusinessRuleDatePredicate, std::placeholders::_1, ticketingDate);
    return getFilteredSize(key, predicate);
  }

private:
  const TaxName _taxName;
  const type::Vendor _vendor;
};
}
