// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Pricing/Shopping/PQ/SoloFlightOnlySolutionsDataTypes.h"

#include "Pricing/Shopping/PQ/SoloFlightOnlySolutionsValidators.h"

namespace tse
{
namespace fos
{

bool operator<(const fos::SOPWrapper& lhs, const fos::SOPWrapper& rhs)
{
  return lhs._sopId < rhs._sopId;
}

void
GroupedSolutions::addFOS(const SolutionsContainer::value_type& item)
{
  _solutions.push_back(item);
}

std::string
SOPDetails::str() const
{
  std::ostringstream stream;
  stream << _cxr[0] << " " << _tvlSegPortions[0].front()->origAirport() << " "
         << _tvlSegPortions[0].back()->destAirport() << std::endl;
  if (!_tvlSegPortions[1].empty())
    stream << _cxr[1] << " " << _tvlSegPortions[1].front()->origAirport() << " "
           << _tvlSegPortions[1].back()->destAirport();
  return stream.str();
}

std::pair<Sops, Sops>
SOPCollections::getSopsByDatePair(const DatePair& datePair)
{
  if ((_sopsByDate.size() == 2) && _sopsByDate[0].count(datePair.first) &&
      _sopsByDate[1].count(datePair.second))
    return std::make_pair(_sopsByDate[0][datePair.first], _sopsByDate[1][datePair.second]);

  return std::make_pair(Sops(), Sops());
}

} // namespace fos
} // namespace tse
