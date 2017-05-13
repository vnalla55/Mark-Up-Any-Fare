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

#include "Common/TaxableUnitTagSet.h"
#include "DataModel/Common/Codes.h"
#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Common/Types.h"
#include "DataModel/Services/RulesRecord.h"
#include "TaxDisplay/View.h"

#include <functional>
#include <memory>
#include <set>

namespace tax
{
namespace display
{
class ViewX1SequenceDetailBuilder;

class ViewX1TaxDetail : public View
{
  using Comparator = bool(*)(const std::shared_ptr<const RulesRecord>&,
                             const std::shared_ptr<const RulesRecord>&);

public:
  using DataSet = std::set<std::shared_ptr<const RulesRecord>, Comparator>;

  ViewX1TaxDetail(const DataSet& rulesRecords,
                  const type::NationName& nationName,
                  const type::TaxLabel* taxLabel,
                  ResponseFormatter& responseFormatter)
    : View(responseFormatter),
      _nationName(nationName),
      _rulesRecords(rulesRecords),
      _taxLabel(taxLabel) {}

  bool header() override;
  bool body() override;
  bool footer() override;

  static bool bySequenceNbCompare(const std::shared_ptr<const RulesRecord>& lhs,
                                  const std::shared_ptr<const RulesRecord>& rhs)
  {
    return lhs->seqNo < rhs->seqNo;
  }

  void setCombinedView(const std::shared_ptr<ViewX1SequenceDetailBuilder>& sequenceDetailBuilder)
  {
    _sequenceDetailBuilder = sequenceDetailBuilder;
  }

private:
  const type::NationName& _nationName;
  const DataSet& _rulesRecords;
  const type::TaxLabel* _taxLabel;

  std::shared_ptr<ViewX1SequenceDetailBuilder> _sequenceDetailBuilder;
};

} // namespace display
} // namespace tax
