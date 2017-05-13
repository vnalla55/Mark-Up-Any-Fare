
//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/Assert.h"

#include <ostream>

namespace tse
{

namespace swp
{

// Basic score given by a single appraiser
// to a particular item
//
// CATEGORY values have following meanings:
//     MUST_HAVE: an item is in appraiser's interest and its collecting will
//                increase appraiser's progress (e.g. appraiser needs
//                to collect 20 items of this type and there are currently
//                5 collected)
//     NICE_TO_HAVE: an item is in appraiser's interest but its collecting
//                   will not increase appraiser's progress (e.g. appraiser needs
//                   to collect 20 items of this type and there are currently
//                   23 collected). If some elements will be removed, use of this
//                   increases chance that only completely unuseful items
//                   will be discarded since we may treat elements 'nice
//                   to have' as 'better' than ignored ones.
//     IGNORE: not in appraiser's interest
//     WANT_TO_REMOVE: appraiser does not want this item to be in the
//                     result set, e.g. because it exceedes the maximum
//                     desired amount of such items (for example, appraiser
//                     does not want to be more than 10 elements of this kind
//                     and this is the 11-th)
//
// Minor rank, which is an int, is an additional sort key
// which may be used as a help for an appraiser that wants
// to further distinguish between items in the same category.
// E.g. an appraiser collects numbers and wants all even numbers
// to stay in the result set (must have). But also is more interested
// in big numbers than in small ones. So it can use the number
// value as minor rank.
class BasicAppraiserScore
{
public:
  enum CATEGORY
  {
    WANT_TO_REMOVE = 0,
    IGNORE,
    NICE_TO_HAVE,
    MUST_HAVE
  };

  static const int DEFAULT_MINOR_RANK = 0;

  // Assigns category and rank, as described above.
  explicit BasicAppraiserScore(CATEGORY category = IGNORE, int minorRank = DEFAULT_MINOR_RANK)
    : _category(category), _minorRank(minorRank)
  {
  }

  bool operator==(const BasicAppraiserScore& right) const
  {
    return (_category == right._category) && (_minorRank == right._minorRank);
  }

  bool operator!=(const BasicAppraiserScore& right) const { return !((*this) == right); }

  CATEGORY getCategory() const { return _category; }

  void setCategory(CATEGORY c)
  {
    TSE_ASSERT((c >= WANT_TO_REMOVE) && (c <= MUST_HAVE));
    _category = c;
  }

  int getMinorRank() const { return _minorRank; }

  void setMinorRank(int minorRank) { _minorRank = minorRank; }

private:
  CATEGORY _category = CATEGORY::IGNORE;
  int _minorRank = DEFAULT_MINOR_RANK;
};

std::ostream& operator<<(std::ostream& out, BasicAppraiserScore::CATEGORY c);

void
formatBasicAppraiserScore(std::ostream& out, const BasicAppraiserScore& brs);

void
formatShort(std::ostream& out, const BasicAppraiserScore& brs);

void
printBasShortFormatDescription(std::ostream& out);

std::ostream& operator<<(std::ostream& out, const BasicAppraiserScore& brs);

} // namespace swp
} // namespace tse
