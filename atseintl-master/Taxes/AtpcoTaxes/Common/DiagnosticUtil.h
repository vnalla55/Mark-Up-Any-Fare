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
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>

#include <DataModel/Common/Types.h>

#include "Util/BranchPrediction.h"

namespace tax
{

struct DiagnosticUtil
{
  static void
  splitSeqValues(const std::string& line, type::SeqNo& seq, type::SeqNo& seqLimit, bool& isSeqRange)
  {
    std::vector<std::string> values;
    std::string delimiter = "-";

    boost::split(values, line, boost::is_any_of(delimiter));

    seq = type::SeqNo();
    if ((values.size() >= 1) && !values[0].empty())
    {
      try { seq = boost::lexical_cast<type::SeqNo>(values[0]); }
      catch (boost::bad_lexical_cast&) {}
    }

    seqLimit = type::SeqNo();
    if ((values.size() >= 2) && !values[1].empty())
    {
      try { seqLimit = boost::lexical_cast<type::SeqNo>(values[1]); }
      catch (boost::bad_lexical_cast&) {}
    }

    isSeqRange = (seq < seqLimit) ||
                 (!line.empty() && (line.substr(values[0].size(), delimiter.size()) == delimiter));
  }

  static bool
  isSeqNoMatching(const type::SeqNo& seqNo,
                  const type::SeqNo& seq,
                  const type::SeqNo& seqLimit,
                  const bool& isSeqRange)
  {
    bool isSeqMatched = (seq == seqNo) || ((seq == 0) && (seqLimit == 0));

    if (UNLIKELY(isSeqRange))
    {
      if (seqLimit == 0)
      {
        isSeqMatched = (seq <= seqNo);
      }
      else
      {
        isSeqMatched = (seq <= seqNo) && (seqNo <= seqLimit);
      }
    }

    return isSeqMatched;
  }
};
}
