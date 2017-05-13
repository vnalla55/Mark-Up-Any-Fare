#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/BookingCodeExceptionSequenceList.h"

#include <tuple>
#include <boost/noncopyable.hpp>

#include <memory>

namespace tse
{

class BookingCodeExceptionIndex;
typedef std::shared_ptr<const BookingCodeExceptionIndex> BookingCodeExceptionIndexConstPtr;

//=============================================================================
class BookingCodeExceptionChains
{
  friend class BookingCodeExceptionIndex;

public:
  typedef std::tuple<BookingCodeExceptionSeqVec::const_iterator,
                       BookingCodeExceptionSeqVec::const_iterator> HeapItem;
  typedef std::vector<HeapItem> Heap;

  typedef std::shared_ptr<BookingCodeExceptionChains> Ptr;
  typedef std::shared_ptr<const BookingCodeExceptionChains> ConstPtr;

  // Returns 0 if there is no more sequences
  BookingCodeExceptionSequence* nextSequence();

private:
  // Takes ownership of heap data
  BookingCodeExceptionChains(const BookingCodeExceptionIndexConstPtr& owner);

private:
  const BookingCodeExceptionIndexConstPtr _owner;
  Heap _heap;
  int _lastSeqNo;
};

//=============================================================================
class BookingCodeExceptionIndex : public std::enable_shared_from_this<BookingCodeExceptionIndex>
{
public:
  typedef std::shared_ptr<BookingCodeExceptionIndex> Ptr;
  typedef std::shared_ptr<const BookingCodeExceptionIndex> ConstPtr;
  typedef std::vector<BookingCodeExceptionSeqVec> LetterToChainMap;
  typedef std::map<CarrierCode, LetterToChainMap> CarrierToChainsMap;

  static Ptr create(const BookingCodeExceptionSequenceList& bceSequences, bool chart2);

  BookingCodeExceptionChains::Ptr lookupChains(const std::set<CarrierCode>& carriers,
                                               const FareType& fareType,
                                               const FareClassCode& fareClass) const;

  friend std::ostream& operator<<(std::ostream& stream, const BookingCodeExceptionIndex& index);

private:
  typedef std::set<char> CharSet;

  enum Status
  { VALID,
    INVALID,
    UNRECOGNIZED };

private:
  BookingCodeExceptionIndex(const BookingCodeExceptionSequenceList& bceSequences, bool chart2);

  void updateChainsData(BookingCodeExceptionChains::Heap& heap,
                        const std::set<CarrierCode>& uniqueCarriers,
                        const CarrierToChainsMap& map,
                        char letter) const;

  int letterToIndex(char c) const;
  void buildIndex(const BookingCodeExceptionSequenceList& bceSequences);
  CarrierCode findViaCarrier(const BookingCodeExceptionSequence& sequence);
  bool collectAllowedChars(CharSet& allowedChar,
                           BookingCodeExceptionSequence* sequence,
                           BookingCodeExceptionSegmentVector& segmentVector,
                           bool& fareType,
                           bool canSkipFareClassOrTypeMatching);
  bool collectAllowedCharsFromSegment(BookingCodeExceptionSequence* sequence,
                                      CharSet& allowedChar,
                                      const BookingCodeExceptionSegment* segment,
                                      bool& fareType,
                                      bool canSkipFareClassOrTypeMatching);
  void updateCarrierMaps(LetterToChainMap& fareTypeMap,
                         LetterToChainMap& fareClassMap,
                         BookingCodeExceptionSequence* sequence,
                         const CharSet& allowedChar,
                         bool fareType);
  bool updateAllowedFareTypesUsingWildcard(CharSet& allowed,
                                           const FareClassCode& wildcard,
                                           BookingCodeExceptionSequence* sequence,
                                           bool canSkipFareClassOrTypeMatching);
  bool updateAllowedFareClassesUsingSingleFareClass(CharSet& allowed, const FareClassCode& cls);
  bool updateAllowedFareClassesUsingWildcard(CharSet& allowed,
                                             const FareClassCode& wildcard,
                                             BookingCodeExceptionSequence* sequence,
                                             bool canSkipFareClassOrTypeMatching);
  void pushBackToLetters(LetterToChainMap& map,
                         const CharSet& letters,
                         BookingCodeExceptionSequence* sequence);
  void initializeSkipMatchingFlag(BookingCodeExceptionSequence* sequence);
  bool thenSegmentsWithTheSameCarrier(const BookingCodeExceptionSegmentVector& segments);
  bool differentFareClass(const BookingCodeExceptionSegmentVector& segments);
  bool isSpecialTwoSegSwitchedCase(const BookingCodeExceptionSegmentVector& segments);
  bool isSpecialDoubledCase(const BookingCodeExceptionSegmentVector& segments);
  bool isSpecialMultiMTCase(const BookingCodeExceptionSegmentVector& segments, bool& emptyFT);

private:
  bool _chart2;
  CarrierToChainsMap _fareTypeChains;
  CarrierToChainsMap _fareClassChains;
  BookingCodeExceptionSeqVec _exceptionsChain;
};

} // namespace tse

