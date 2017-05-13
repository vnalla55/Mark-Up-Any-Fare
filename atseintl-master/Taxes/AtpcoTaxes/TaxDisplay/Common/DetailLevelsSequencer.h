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

#include <functional>
#include <vector>

namespace tax
{
namespace display
{

class DetailLevelsSequencer
{
  friend class DetailLevelsSequencerTest;

public:
  DetailLevelsSequencer() = default;
  explicit DetailLevelsSequencer(const DetailLevels& detailLevels);

  void pushCallback(std::function<bool(DetailEntryNo, bool)> callback)
  {
    _callbacks.push_back(callback);
  }

  const std::vector<DetailEntryNo>& getDetailEntries() const { return _detailEntries; }
  bool hasDetailEntries() const { return !_detailEntries.empty(); }
  bool run() const;

private:
  std::vector<DetailEntryNo> _detailEntries;
  std::vector<std::function<bool(DetailEntryNo, bool)>> _callbacks;
};

} // namespace display
} // namespace tax
