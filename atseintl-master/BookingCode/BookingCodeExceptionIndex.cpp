#include "BookingCode/BookingCodeExceptionIndex.h"

#include "BookingCode/BookingCodeExceptionValidator.h"
#include "Common/TseConsts.h"
#include "DBAccess/BookingCodeExceptionSequence.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <tuple>

namespace tse
{

struct GreaterSeqNo
{
  bool operator()(const BookingCodeExceptionChains::HeapItem& a,
                  const BookingCodeExceptionChains::HeapItem& b) const
  {
    if (UNLIKELY((*std::get<0>(a))->itemNo() != (*std::get<0>(b))->itemNo()))
      return (*std::get<0>(a))->itemNo() > (*std::get<0>(b))->itemNo();

    return (*std::get<0>(a))->seqNo() > (*std::get<0>(b))->seqNo();
  }
};
// GreaterSeqNo

BookingCodeExceptionSequence*
BookingCodeExceptionChains::nextSequence()
{
  GreaterSeqNo greaterSeqNo;
  while (!_heap.empty())
  {
    std::pop_heap(_heap.begin(), _heap.end(), greaterSeqNo);

    BookingCodeExceptionChains::HeapItem& item = _heap.back();
    BookingCodeExceptionSeqVec::const_iterator it = std::get<0>(item);
    BookingCodeExceptionSeqVec::const_iterator end = std::get<1>(item);

    BookingCodeExceptionSequence* result(*it);
    it++;

    if (it == end)
      _heap.pop_back();
    else
    {
      _heap.back() = std::make_tuple(it, end);
      std::push_heap(_heap.begin(), _heap.end(), greaterSeqNo);
    }

    // There may be duplicates in the heap
    // so return only if we got higher sequence number
    if (LIKELY((int)result->seqNo() > _lastSeqNo))
    {
      _lastSeqNo = result->seqNo();

      return result;
    }
  }
  return nullptr;
}

BookingCodeExceptionChains::BookingCodeExceptionChains(
    const BookingCodeExceptionIndexConstPtr& owner)
  : _owner(owner), _lastSeqNo(-1)
{
}

BookingCodeExceptionIndex::Ptr
BookingCodeExceptionIndex::create(const BookingCodeExceptionSequenceList& bceSequences, bool chart2)
{
  return Ptr(new BookingCodeExceptionIndex(bceSequences, chart2));
}

BookingCodeExceptionChains::Ptr
BookingCodeExceptionIndex::lookupChains(const std::set<CarrierCode>& carriers,
                                        const FareType& fareType,
                                        const FareClassCode& fareClass) const
{
  BookingCodeExceptionChains::Ptr result(new BookingCodeExceptionChains(shared_from_this()));

  if (!_exceptionsChain.empty())
    result->_heap.push_back(std::make_tuple(_exceptionsChain.begin(), _exceptionsChain.end()));

  if (LIKELY(!fareType.empty()))
    updateChainsData(result->_heap, carriers, _fareTypeChains, fareType[0]);

  if (LIKELY(!fareClass.empty()))
    updateChainsData(result->_heap, carriers, _fareClassChains, fareClass[0]);

  std::make_heap(result->_heap.begin(), result->_heap.end(), GreaterSeqNo()); // top of the heap is
                                                                              // smallest sequence

  return result;
}

BookingCodeExceptionIndex::BookingCodeExceptionIndex(
    const BookingCodeExceptionSequenceList& bceSequences, bool chart2)
  : _chart2(chart2)
{
  buildIndex(bceSequences);
}

void
BookingCodeExceptionIndex::updateChainsData(BookingCodeExceptionChains::Heap& heap,
                                            const std::set<CarrierCode>& uniqueCarriers,
                                            const CarrierToChainsMap& map,
                                            char letter) const
{
  int idx = letterToIndex(letter);
  if (UNLIKELY(idx < 0))
    return;

  for (const CarrierCode& carrier : uniqueCarriers)
  {
    CarrierToChainsMap::const_iterator it = map.find(carrier);
    if (it == map.end())
      continue;

    const LetterToChainMap& l2c(it->second);

    if (idx >= (int)l2c.size())
      continue;

    const BookingCodeExceptionSeqVec& chain = l2c[idx];

    if (!chain.empty())
      heap.push_back(std::make_tuple(chain.begin(), chain.end()));

    const BookingCodeExceptionSeqVec& chainAll = l2c[letterToIndex(ALL_TYPE)];

    if (!chainAll.empty())
      heap.push_back(std::make_tuple(chainAll.begin(), chainAll.end()));
  }
}

int
BookingCodeExceptionIndex::letterToIndex(char c) const
{
  if (c == ALL_TYPE)
    return 'Z' - 'A' + 1;

  int index = c - 'A';
  if (UNLIKELY(index < 0 || index > ('Z' - 'A')))
    throw std::invalid_argument("letter A to Z was expected");
  return index;
}

void
BookingCodeExceptionIndex::buildIndex(const BookingCodeExceptionSequenceList& bceSequences)
{
  for (BookingCodeExceptionSequence* sequence : bceSequences.getSequences())
  {
    CarrierCode carrier;

    if (sequence->ifTag() == BookingCodeExceptionValidator::BCE_IF_ANY_TVLSEG)
    {
      if (_chart2)
        carrier = DOLLAR_CARRIER;
      else
        carrier = findViaCarrier(*sequence);
    }
    else
      carrier = findViaCarrier(*sequence);

    if (UNLIKELY(carrier.empty()))
    {
      _exceptionsChain.push_back(sequence);
      continue;
    }

    // There is only one chain for all carrier wildcards
    if (carrier == XDOLLAR_CARRIER)
    {
      carrier = DOLLAR_CARRIER;
    }

    BookingCodeExceptionSegmentVector segmentVector;

    bool emptyFT = false;
    if (isSpecialMultiMTCase(sequence->segmentVector(), emptyFT))
    {
      if (!emptyFT)
      {
        for (BookingCodeExceptionSegment* seg : sequence->segmentVector())
        {
          segmentVector.push_back(seg);
        }
      }
    }
    else if (sequence->ifTag() == BookingCodeExceptionValidator::BCE_IF_FARECOMPONENT)
    {
      if (_chart2 && isSpecialDoubledCase(sequence->segmentVector()))
      {
        segmentVector.push_back(sequence->segmentVector()[0]);
        segmentVector.push_back(sequence->segmentVector()[2]);
      }
      else if (isSpecialTwoSegSwitchedCase(sequence->segmentVector()))
      {
        segmentVector.push_back(sequence->segmentVector()[1]);
      }
    }

    bool canSkipFareClassOrTypeMatching = true;
    if (segmentVector.empty())
    {
      bool diffFareClass = differentFareClass(sequence->segmentVector());
      if (diffFareClass && sequence->ifTag() == BookingCodeExceptionValidator::BCE_CHAR_BLANK)
      {
        _exceptionsChain.push_back(sequence);
        continue;
      }
      canSkipFareClassOrTypeMatching = !diffFareClass;
    }

    CharSet allowedChar;
    bool fareType = false;

    if (segmentVector.size() == 0)
      segmentVector.push_back(sequence->segmentVector()[0]);

    if (UNLIKELY(!collectAllowedChars(
            allowedChar, sequence, segmentVector, fareType, canSkipFareClassOrTypeMatching)))
    {
      _exceptionsChain.push_back(sequence);
      continue;
    }

    updateCarrierMaps(
        _fareTypeChains[carrier], _fareClassChains[carrier], sequence, allowedChar, fareType);
  }
}

bool
BookingCodeExceptionIndex::collectAllowedChars(CharSet& allowedChar,
                                               BookingCodeExceptionSequence* sequence,
                                               BookingCodeExceptionSegmentVector& segmentVector,
                                               bool& fareType,
                                               bool canSkipFareClassOrTypeMatching)
{
  if (segmentVector.size() > 1)
  {
    for (BookingCodeExceptionSegment* seg : segmentVector)
    {
      if (!collectAllowedCharsFromSegment(
              sequence, allowedChar, seg, fareType, canSkipFareClassOrTypeMatching))
        return false;
    }
  }
  else
  {
    if (segmentVector[0]->fareclass().empty())
    {
      allowedChar.insert(ALL_TYPE);
      sequence->setSkipFareClassOrTypeMatching(true);
    }
    else
      return collectAllowedCharsFromSegment(
          sequence, allowedChar, segmentVector[0], fareType, canSkipFareClassOrTypeMatching);
  }

  return true;
}

bool
BookingCodeExceptionIndex::collectAllowedCharsFromSegment(
    BookingCodeExceptionSequence* sequence,
    CharSet& allowedChar,
    const BookingCodeExceptionSegment* segment,
    bool& fareType,
    bool canSkipFareClassOrTypeMatching)
{
  bool status = false;
  switch (segment->fareclassType())
  {
  case 'T':
    status = updateAllowedFareTypesUsingWildcard(
        allowedChar, segment->fareclass(), sequence, canSkipFareClassOrTypeMatching);
    fareType = true;
    break;

  case 'F':
    status = updateAllowedFareClassesUsingSingleFareClass(allowedChar, segment->fareclass());
    break;

  case 'M':
    status = updateAllowedFareClassesUsingWildcard(
        allowedChar, segment->fareclass(), sequence, canSkipFareClassOrTypeMatching);
    break;

  default:
    // We dont recognize fare class/type indicator, so just mark
    // this sequence as exception
    break;
  }

  return status;
}

void
BookingCodeExceptionIndex::updateCarrierMaps(LetterToChainMap& fareTypeMap,
                                             LetterToChainMap& fareClassMap,
                                             BookingCodeExceptionSequence* sequence,
                                             const CharSet& allowedChar,
                                             bool fareType)
{
  if (fareType)
    pushBackToLetters(fareTypeMap, allowedChar, sequence);
  else
    pushBackToLetters(fareClassMap, allowedChar, sequence);

  initializeSkipMatchingFlag(sequence);
}

bool
BookingCodeExceptionIndex::isSpecialMultiMTCase(const BookingCodeExceptionSegmentVector& segments,
                                                bool& emptyFT)
{
  if (segments.size() < 2)
    return false;

  CarrierCode c0 = segments[0]->viaCarrier();
  char ft = segments[0]->fareclassType();
  emptyFT = (ft == ' ');

  for (BookingCodeExceptionSegment* seg : segments)
  {
    if (seg->fareclassType() != ft || seg->viaCarrier() != c0)
      return false;
  }

  return true;
}

bool
BookingCodeExceptionIndex::isSpecialDoubledCase(const BookingCodeExceptionSegmentVector& segments)
{
  if (segments.size() != 4)
    return false;

  char fct0 = segments[0]->fareclassType();
  char fct1 = segments[1]->fareclassType();
  CarrierCode c0 = segments[0]->viaCarrier();
  CarrierCode c1 = segments[1]->viaCarrier();

  return segments[2]->viaCarrier() == c0 && segments[3]->viaCarrier() == c1 && fct1 == ' ' &&
         segments[2]->fareclassType() == fct0 && segments[3]->fareclassType() == fct1;
}

bool
BookingCodeExceptionIndex::isSpecialTwoSegSwitchedCase(
    const BookingCodeExceptionSegmentVector& segments)
{
  char fct = segments[0]->fareclassType();
  FareClassCode& fcc = segments[0]->fareclass();

  return segments.size() == 2 && fct == ' ' && fcc.empty() && segments[1]->fareclassType() != ' ' &&
         !segments[1]->fareclass().empty();
}

bool
BookingCodeExceptionIndex::differentFareClass(const BookingCodeExceptionSegmentVector& segments)
{
  if (segments.size() == 1)
    return false;

  char fct = segments[0]->fareclassType();
  FareClassCode& fcc = segments[0]->fareclass();

  for (size_t x = 1; x < segments.size(); x++)
  {
    if (segments[x]->fareclassType() != ' ' && segments[x]->fareclassType() != fct)
      return true;
    if (UNLIKELY(!segments[x]->fareclass().empty() && segments[x]->fareclass() != fcc))
      return true;
  }

  return false;
}

CarrierCode
BookingCodeExceptionIndex::findViaCarrier(const BookingCodeExceptionSequence& sequence)
{
  const BookingCodeExceptionSegmentVector& segments = sequence.segmentVector();
  if (_chart2)
  {
    if (sequence.ifTag() ==
        BookingCodeExceptionValidator::BCE_CHAR_BLANK) // IF-tag is blank: no conditional segment
      return DOLLAR_CARRIER;
    else
      return segments[0]->viaCarrier(); // ViaCarrier of the conditional segment
  }
  else
  {
    // chart 1: return the ViaCarrier of THEN segments if all THEN segments have the same carrier
    if (segments.size() == 1)
      return segments[0]->viaCarrier(); // single-segment case
    else // multiple segments
    {
      if (thenSegmentsWithTheSameCarrier(segments))
        return segments[1]->viaCarrier(); // ViaCarrier of the second segment, which is THEN segment
      else
        return ""; // empty carrier: go to exception chain!!!
    }
  }

  return "";
}

bool
BookingCodeExceptionIndex::thenSegmentsWithTheSameCarrier(
    const BookingCodeExceptionSegmentVector& segments)
{
  if (segments.size() <= 2)
    return true;

  CarrierCode crx = segments[1]->viaCarrier();
  for (size_t i = 2; i < segments.size(); ++i)
  {
    if (crx != segments[i]->viaCarrier())
      return false;
  }

  return true;
}

bool
BookingCodeExceptionIndex::updateAllowedFareTypesUsingWildcard(
    CharSet& allowed,
    const FareClassCode& wildcard,
    BookingCodeExceptionSequence* sequence,
    bool canSkipFareClassOrTypeMatching)
{
  if (UNLIKELY(wildcard.empty()))
    return false;

  if (wildcard[0] != ALL_TYPE)
  {
    if (wildcard[0] < 'A' || 'Z' < wildcard[0]) // Fare types must start with alpha character
      return false;

    allowed.insert(wildcard[0]);
    return true;
  }

  switch (wildcard[1])
  {
  case ALL_TYPE:
    // Only in V2 - means any fare type
    allowed.insert(ALL_TYPE);
    if (canSkipFareClassOrTypeMatching)
      sequence->setSkipFareClassOrTypeMatching(true);
    break;

  case 'Y':
    allowed.insert('A');
    allowed.insert('E');
    allowed.insert('P');
    allowed.insert('S');
    allowed.insert('X');

    // don't need to match fare type during validation
    if (canSkipFareClassOrTypeMatching)
      sequence->setSkipFareClassOrTypeMatching(true);
    break;

  default:
    if (UNLIKELY(wildcard[1] < 'A' || 'Z' < wildcard[1]))
      return false;

    allowed.insert(wildcard[1]);

    // don't need to match fare type during validation
    if (canSkipFareClassOrTypeMatching &&
        (wildcard[1] == R_TYPE || wildcard[1] == F_TYPE || wildcard[1] == B_TYPE ||
         wildcard[1] == E_TYPE || wildcard[1] == W_TYPE || wildcard[1] == X_TYPE ||
         wildcard[1] == S_TYPE || wildcard[1] == P_TYPE || wildcard[1] == A_TYPE ||
         wildcard[1] == Z_TYPE || wildcard[1] == J_TYPE ))
      sequence->setSkipFareClassOrTypeMatching(true);

    break;
  }
  return true;
}

bool
BookingCodeExceptionIndex::updateAllowedFareClassesUsingSingleFareClass(CharSet& allowed,
                                                                        const FareClassCode& cls)
{
  if (cls.empty())
    return false;

  allowed.insert(cls[0]);

  return true;
}

bool
BookingCodeExceptionIndex::updateAllowedFareClassesUsingWildcard(
    CharSet& allowed,
    const FareClassCode& wildcard,
    BookingCodeExceptionSequence* sequence,
    bool canSkipFareClassOrTypeMatching)
{
  if (UNLIKELY(wildcard.empty()))
    return false;

  if (wildcard[0] < 'A' || 'Z' < wildcard[0])
  // This may be either wildcard '-', or digit.
  // We do not support indexing it.
  {
    if (wildcard[0] == '-')
    {
      allowed.insert(ALL_TYPE);
      return true;
    }

    return false;
  }

  allowed.insert(wildcard[0]);

  if (canSkipFareClassOrTypeMatching && wildcard.length() == 2 && wildcard[1] == '-')
  {
    // don't need to match fare class during validation
    sequence->setSkipFareClassOrTypeMatching(true);
  }

  return true;
}

void
BookingCodeExceptionIndex::pushBackToLetters(LetterToChainMap& map,
                                             const CharSet& letters,
                                             BookingCodeExceptionSequence* sequence)
{
  map.resize(letterToIndex(ALL_TYPE) + 1);
  for (char letter : letters)
  {
    map[letterToIndex(letter)].push_back(sequence);
  }
}

void
BookingCodeExceptionIndex::initializeSkipMatchingFlag(BookingCodeExceptionSequence* sequence)
{
  for (const BookingCodeExceptionSegment* s : sequence->segmentVector())
  {
    if (s->primarySecondary() != BookingCodeExceptionValidator::BCE_CHAR_BLANK)
      return;

    if (s->flight1() > 0)
      return;

    if (s->equipType() != BookingCodeExceptionValidator::BCE_EQUIPBLANK)
      return;

    if (!s->tvlPortion().empty() &&
        s->tvlPortion()[0] != BookingCodeExceptionValidator::BCE_CHAR_BLANK)
      return;

    if (s->tsi() != BookingCodeExceptionValidator::BCE_NO_TSI)
      return;

    if (!s->loc1().empty() || !s->loc2().empty())
      return;

    if (!s->posLoc().empty() && s->posLocType() != '\0')
      return;

    if (UNLIKELY(s->soldInOutInd() != BookingCodeExceptionValidator::BCE_CHAR_BLANK &&
        s->soldInOutInd() != '\0'))
      return;

    if (!s->daysOfWeek().empty() || s->tvlEffYear() != 0 || s->tvlEffMonth() != 0 ||
        s->tvlEffDay() != 0 || s->tvlDiscYear() != 0 || s->tvlDiscMonth() != 0 || s->tvlDiscDay())
      return;
  }
  sequence->setSkipMatching(true);
}

std::ostream& operator<<(std::ostream& stream, const BookingCodeExceptionSegment& seg)
{
  stream << "fareclass: " << seg.fareclass() << " fareclass type: " << seg.fareclassType()
         << " via carrier: " << seg.viaCarrier() << std::endl;
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const BookingCodeExceptionSequence& seq)
{
  stream << "itemNo: " << seq.itemNo() << " seqNo: " << seq.seqNo() << " ifTag: " << seq.ifTag()
         << std::endl;
  for (const BookingCodeExceptionSegment* seg : seq.segmentVector())
  {
    stream << *seg;
  }
  return stream;
}

std::ostream&
operator<<(std::ostream& stream, const BookingCodeExceptionIndex::CarrierToChainsMap& cxToMap)
{
  for (const BookingCodeExceptionIndex::CarrierToChainsMap::value_type& cxSeqVec : cxToMap)
  {
    if (cxSeqVec.second.size())
    {
      stream << "Carrier: " << cxSeqVec.first << std::endl;
      char idx = 0;
      for (const BookingCodeExceptionSeqVec& seqVec : cxSeqVec.second)
      {
        if (seqVec.size())
          stream << char(idx + 'A') << " ";
        idx++;
        for (BookingCodeExceptionSequence* seq : seqVec)
        {
          stream << *seq;
        }
      }
    }
  }
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const BookingCodeExceptionIndex& index)
{
  stream << "Chart " << (index._chart2 ? "2" : "1") << std::endl;

  stream << "Exceptions: " << std::endl;
  for (BookingCodeExceptionSequence* seq : index._exceptionsChain)
  {
    stream << *seq;
  }

  stream << "FareTypeChains: " << std::endl;
  stream << index._fareTypeChains;
  stream << "FareClassChains: " << std::endl;
  stream << index._fareClassChains;

  return stream;
}

} // namespace tse
