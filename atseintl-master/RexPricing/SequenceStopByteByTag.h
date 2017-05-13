#pragma once

//-------------------------------------------------------------------
//
//   The copyright to the computer program(s) herein
//   is the property of Sabre.
//   The program(s) may be used and/or copied only with
//   the written permission of Sabre or in accordance
//   with the terms and conditions stipulated in the
//   agreement/contract under which the program(s)
//   have been supplied.
//
//----------------------------------------------------------------------------

#include "Diagnostic/Diag689Collector.h"

#include <log4cxx/helpers/objectptr.h>

namespace tse
{
class ProcessTagInfo;
class ProcessTagPermutation;
class FareCompInfo;

typedef std::map<std::pair</*ProcessTag*/ int, FareCompInfo*>,
                 std::set<std::pair</*T988 Seq*/ int, /*Permutation Number*/ int> > >
TagFcWithStopBytes;
typedef std::vector<const ProcessTagPermutation*> MaybeSkipPermutations;

class SequenceStopByteByTag
{

  friend class SequenceStopByteByTagTest;

private:
  TagFcWithStopBytes _tagFcWithStopBytes;
  MaybeSkipPermutations _maybeSkipPermutations;

  std::vector<ProcessTagInfo*>::const_iterator i;
  std::vector<ProcessTagInfo*>::const_iterator e;

public:
  static bool hasStopByte(const ProcessTagInfo* processTagInfo);
  SequenceStopByteByTag();
  virtual ~SequenceStopByteByTag() {}
  int isSequenceGreater(std::set<std::pair<int, int> >& sequences, int sequence) const;
  bool findSequenceGreater(std::set<std::pair<int, int> >& stopByteSequences,
                           int sequence,
                           std::pair<int, int>& sequencePermutation) const;
  bool hasSequence(std::set<std::pair<int, int> >& sequences, int sequence) const;
  bool skipByStopByte(const ProcessTagPermutation& permutation);
  void saveStopByteInfo(const ProcessTagPermutation& permutation);

  TagFcWithStopBytes& tagFcWithStopBytes() { return _tagFcWithStopBytes; }
  MaybeSkipPermutations& maybeSkipPermutations() { return _maybeSkipPermutations; }
};
}

// local variables:
// mode: c++
// end:
