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
#include "RexPricing/SequenceStopByteByTag.h"

#include "Common/TsePrimitiveTypes.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/RexPricingTrx.h"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

namespace tse
{
SequenceStopByteByTag::SequenceStopByteByTag() {}

bool
SequenceStopByteByTag::hasStopByte(const ProcessTagInfo* processTagInfo)
{
  static const Indicator StopByte = 'X';

  return processTagInfo->reissueSequence()->orig() &&
         processTagInfo->reissueSequence()->stopInd() == StopByte;
}
namespace
{
struct CompareSequencePermutationPairBySequenceGreater
{
  int _sequence;
  CompareSequencePermutationPairBySequenceGreater(int sequence) : _sequence(sequence) {}
  bool operator()(std::pair</*T988 Seq*/ int, /*Permutation Number*/ int> sequencePermutation)
  {
    return _sequence > sequencePermutation.first;
  }
};
struct CompareSequencePermutationPairBySequenceEqual
{
  int _sequence;
  CompareSequencePermutationPairBySequenceEqual(int sequence) : _sequence(sequence) {}
  bool operator()(std::pair</*T988 Seq*/ int, /*Permutation Number*/ int> sequencePermutation)
  {
    return _sequence == sequencePermutation.first;
  }
};
};
int
SequenceStopByteByTag::isSequenceGreater(
    std::set<std::pair</*T988 Seq*/ int, /*Perm Number*/ int> >& sequences, int sequence) const
{
  int seq(0);
  CompareSequencePermutationPairBySequenceGreater compareSequenceIsGreaterThan(sequence);
  std::set<std::pair<int, int> >::iterator search(
      find_if(sequences.begin(), sequences.end(), compareSequenceIsGreaterThan));
  if (search != sequences.end())
    seq = search->first;
  return seq;
}

bool
SequenceStopByteByTag::findSequenceGreater(std::set<std::pair<int, int> >& stopByteSequences,
                                           int sequence,
                                           std::pair<int, int>& sequencePermutation) const
{
  bool rc(false);
  CompareSequencePermutationPairBySequenceGreater compareSequenceIsGreaterThan(sequence);
  std::set<std::pair<int, int> >::iterator search(
      find_if(stopByteSequences.begin(), stopByteSequences.end(), compareSequenceIsGreaterThan));
  if (search != stopByteSequences.end())
  {
    rc = true;
    sequencePermutation = *search;
  }
  return rc;
}

bool
SequenceStopByteByTag::hasSequence(std::set<std::pair<int, int> >& sequences, int sequence) const
{
  bool rc = false;
  CompareSequencePermutationPairBySequenceEqual compareSequenceIsEqual(sequence);
  std::set<std::pair<int, int> >::iterator search(
      find_if(sequences.begin(), sequences.end(), compareSequenceIsEqual));
  if (search != sequences.end())
    rc = true;
  return rc;
}

bool
SequenceStopByteByTag::skipByStopByte(const ProcessTagPermutation& permutation)
{
  i = permutation.processTags().begin();
  e = permutation.processTags().end();
  bool skip(false);
  for (; i != e; i++)
  {
    ProcessTagInfo* processTagInfo(*i);

    if (!processTagInfo->reissueSequence()->orig())
      continue;

    FareCompInfo* fareCompInfo(processTagInfo->fareCompInfo());
    ProcessTag tag((ProcessTag)processTagInfo->processTag());
    int sequence((ProcessTag)processTagInfo->seqNo());
    std::pair<ProcessTag, FareCompInfo*> key(tag, fareCompInfo);

    int seqFound(isSequenceGreater(_tagFcWithStopBytes[key], sequence));
    // This works because there is no tag 0.
    if (seqFound)
    {
      skip = true;
      break;
    }
  }
  return skip;
}

void
SequenceStopByteByTag::saveStopByteInfo(const ProcessTagPermutation& permutation)
{
  i = permutation.processTags().begin();
  e = permutation.processTags().end();

  for (; i != e; i++)
  {
    ProcessTagInfo* processTagInfo(*i);

    if (!processTagInfo->reissueSequence()->orig())
      continue;

    FareCompInfo* fareCompInfo(processTagInfo->fareCompInfo());
    ProcessTag tag((ProcessTag)processTagInfo->processTag());
    int sequence((ProcessTag)processTagInfo->seqNo());
    std::pair<ProcessTag, FareCompInfo*> key(tag, fareCompInfo);
    if (SequenceStopByteByTag::hasStopByte(processTagInfo))
    {
      if (!hasSequence(_tagFcWithStopBytes[key], sequence))
        _tagFcWithStopBytes[key].insert(std::make_pair(sequence, permutation.number()));
    }
  }
  _maybeSkipPermutations.push_back(&permutation);
}
}
