//-------------------------------------------------------------------
//  Copyright Sabre 2011
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

#include "Pricing/Combinations.h"
#include "Pricing/CombinationsSubCat109.h"

#include "DBAccess/OpenJawRestriction.h"

namespace tse
{
bool
CombinationsSubCat109::match()
{
  if (UNLIKELY(_prU.puType() != PricingUnit::Type::OPENJAW))
  {
    _components[0].getSubCat(Combinations::m109) = Combinations::MATCH;
    return true;
  }

  _openJawSetVector = &_trx.dataHandle().getOpenJawRestriction(_vendor, _itemNo);

  if (UNLIKELY(_openJawSetVector->empty()))
  {
    displayDiagError();
    return false;
  }

  DiagCollectorGuard dcg1(_diag, _diagnostic);
  if (UNLIKELY(_diag.isActive()))
    displayDiagItem();

  if (UNLIKELY(_components[0]._passMinor))
    return true;

  if (UNLIKELY(_diag.isActive()))
    displayDiag();

  switch (_prU.puSubType())
  {
  case PricingUnit::DEST_OPENJAW:
    matchDestinationPart();
    return true;

  case PricingUnit::ORIG_OPENJAW:
    matchOriginPart();
    return true;

  case PricingUnit::DOUBLE_OPENJAW:
    matchOriginPart();

    if (_components[0].getSubCat(Combinations::m109) == Combinations::MATCH && !_negativeApplication)
      matchDestinationPart();

    return true;

  case PricingUnit::UNKNOWN_SUBTYPE:
  default:
    ;
  }

  return true;
}

void
CombinationsSubCat109::matchOriginPart()
{
  matchGenericPart(_prU.fareUsage().front()->paxTypeFare()->fareMarket()->origin(),
                   _prU.fareUsage().back()->paxTypeFare()->fareMarket()->destination(),
                   _prU.fareUsage().front()->paxTypeFare()->fareMarket()->boardMultiCity(),
                   _prU.fareUsage().back()->paxTypeFare()->fareMarket()->offMultiCity());
}

void
CombinationsSubCat109::matchDestinationPart()
{
  const FareMarket* ojFrom;
  const FareMarket* ojTo;
  if (LIKELY(getDestinationOpenSegment(ojFrom, ojTo)))
  {
    matchGenericPart(
        ojFrom->destination(), ojTo->origin(), ojFrom->offMultiCity(), ojTo->boardMultiCity());
    return;
  }

  // carefully, if somehow (impossible?) we reach this, for doubleOJ we can have MATCH, reset to
  // NO_MATCH then!
  if (_components[0].getSubCat(Combinations::m109) == Combinations::MATCH)
    _components[0].getSubCat(Combinations::m109) = Combinations::NO_MATCH;
}

void
CombinationsSubCat109::matchGenericPart(const Loc* ojFrom,
                                        const Loc* ojTo,
                                        const LocCode& ojFromCity,
                                        const LocCode& ojToCity)
{
  if (UNLIKELY(!ojFrom || !ojTo))
    return;

  // To get accurrate match on City (avoid problem on multiCity transport)
  // copy ojFrom, ojTo to local memory ojFromTmp, ojToTmp and replace the city().
  // Using local memory as they are not only used by function called here
  const Loc& ojFromCorrect = (ojFrom->city() == ojFromCity) ? *ojFrom : Loc(*ojFrom, ojFromCity);
  const Loc& ojToCorrect = (ojTo->city() == ojToCity) ? *ojTo : Loc(*ojTo, ojToCity);

  const SetNumber passedSetNum = passOJSetRestriction(ojFromCorrect, ojToCorrect);

  if (passedSetNum != NO_MATCH_SET)
  {
    if (UNLIKELY(_diag.isActive()))
      _diag << "      MATCH SET " << passedSetNum << " - " << ojFromCity << " - " << ojToCity
            << std::endl;

    _components[0].getSubCat(Combinations::m109) = Combinations::MATCH;
  }

  else
  {
    if (UNLIKELY(_diag.isActive()))
      _diag << "      NO MATCH SET - " << ojFromCity << " - " << ojToCity << std::endl;

    _components[0].getSubCat(Combinations::m109) = Combinations::NO_MATCH;
  }
}

CombinationsSubCat109::SetNumber
CombinationsSubCat109::passOJSetRestriction(const Loc& ojFrom, const Loc& ojTo)
{
  ojrIt seqI = _openJawSetVector->begin();
  const ojrIt seqIend = _openJawSetVector->end();
  for (; seqI != seqIend; ++seqI)
  {
    if (matchNegativeSequence(
            ojFrom, ojTo, (**seqI).set1ApplInd(), (**seqI).set1Loc1(), (**seqI).set1Loc2()))
    {
      if (matchPositiveSet(SECOND_SET, ojFrom, ojTo))
        return SECOND_SET;

      _negativeApplication = true;
      return FIRST_SET;
    }

    if (UNLIKELY(matchNegativeSequence(
            ojFrom, ojTo, (**seqI).set2ApplInd(), (**seqI).set2Loc1(), (**seqI).set2Loc2())))
    {
      if (matchPositiveSet(FIRST_SET, ojFrom, ojTo))
        return FIRST_SET;

      _negativeApplication = true;
      return SECOND_SET;
    }
  }

  if (matchPositiveSet(FIRST_SET, ojFrom, ojTo))
    return FIRST_SET;

  if (matchPositiveSet(SECOND_SET, ojFrom, ojTo))
    return SECOND_SET;

  return NO_MATCH_SET;
}

bool
CombinationsSubCat109::matchPositiveSet(const SetNumber& setNumber,
                                        const Loc& ojFrom,
                                        const Loc& ojTo) const
{
  ojrIt seqI = _openJawSetVector->begin();
  const ojrIt seqIend = _openJawSetVector->end();

  if (setNumber == FIRST_SET)
  {
    for (; seqI != seqIend; ++seqI)
      if (matchPositiveSequence(
              ojFrom, ojTo, (**seqI).set1ApplInd(), (**seqI).set1Loc1(), (**seqI).set1Loc2()))
        return true;

    return matchPositiveBetweenSet(FIRST_SET, ojFrom, ojTo);
  }

  // SECOND_SET
  for (; seqI != seqIend; ++seqI)
    if (matchPositiveSequence(
            ojFrom, ojTo, (**seqI).set2ApplInd(), (**seqI).set2Loc1(), (**seqI).set2Loc2()))
      return true;

  return matchPositiveBetweenSet(SECOND_SET, ojFrom, ojTo);
}

bool
CombinationsSubCat109::matchPositiveBetweenSet(const SetNumber& setNumber,
                                               const Loc& ojFrom,
                                               const Loc& ojTo) const
{
  ojrIt seqI = _openJawSetVector->begin();
  const ojrIt seqIend = _openJawSetVector->end();
  std::vector<const LocKey*> setBetweenLocs;

  if (setNumber == FIRST_SET)
  {
    for (; seqI != seqIend; ++seqI)
      if ((**seqI).set1ApplInd() == OJ_BETWEEN_POINTS)
        setBetweenLocs.push_back(&((**seqI).set1Loc1()));
  }
  else // SECOND_SET
  {
    for (; seqI != seqIend; ++seqI)
      if ((**seqI).set2ApplInd() == OJ_BETWEEN_POINTS)
        setBetweenLocs.push_back(&((**seqI).set2Loc1()));
  }

  return setBetweenLocs.size() > 1 && isInLoc(ojFrom, setBetweenLocs) &&
         isInLoc(ojTo, setBetweenLocs);
}

bool
CombinationsSubCat109::matchNegativeSequence(const Loc& ojFrom,
                                             const Loc& ojTo,
                                             Indicator setApplInd,
                                             const LocKey& setLoc1,
                                             const LocKey& setLoc2) const
{
  if (setApplInd != OJ_NEGATIVE_LOC)
    return false;

  return setLoc2.isNull() ? (isInLoc(ojFrom, setLoc1) || isInLoc(ojTo, setLoc1))
                          : (isInLoc(ojFrom, setLoc1) && isInLoc(ojTo, setLoc2)) ||
                                (isInLoc(ojFrom, setLoc2) && isInLoc(ojTo, setLoc1));
}

bool
CombinationsSubCat109::matchPositiveSequence(const Loc& ojFrom,
                                             const Loc& ojTo,
                                             Indicator setApplInd,
                                             const LocKey& setLoc1,
                                             const LocKey& setLoc2) const
{
  if (setApplInd == OJ_BETWEEN_AND)
    return isBetween(ojFrom, ojTo, setLoc1, setLoc2);

  else if (setApplInd == OJ_SAME_COUNTRY)
  {
    if (!sameCountrySegment(ojFrom, ojTo))
      return false;

    return setLoc1.isNull() || (isInLoc(ojFrom, setLoc1) && isInLoc(ojTo, setLoc1));
  }

  return false; // NULL or not supported applInd
}

bool
CombinationsSubCat109::sameCountrySegment(const Loc& ojFrom, const Loc& ojTo) const
{
  if ((_prU.geoTravelType() != GeoTravelType::International && _prU.geoTravelType() != GeoTravelType::Transborder) ||
      (ojFrom.nation() == ojTo.nation()))
    return true;

  return LocUtil::isRussianGroup(ojFrom) && LocUtil::isRussianGroup(ojTo);
}

bool
CombinationsSubCat109::getDestinationOpenSegment(const FareMarket*& ojFrom, const FareMarket*& ojTo)
    const
{
  size_t numOfFU = _prU.fareUsage().size();
  for (size_t fuCount = 0; fuCount < numOfFU - 1; ++fuCount)
    if (_prU.fareUsage()[fuCount]->paxTypeFare()->fareMarket()->offMultiCity() !=
        _prU.fareUsage()[fuCount + 1]->paxTypeFare()->fareMarket()->boardMultiCity())
    {
      ojFrom = _prU.fareUsage()[fuCount]->paxTypeFare()->fareMarket();
      ojTo = _prU.fareUsage()[fuCount + 1]->paxTypeFare()->fareMarket();
      return true;
    }

  return false;
}

bool
CombinationsSubCat109::isBetween(const Loc& ojFrom,
                                 const Loc& ojTo,
                                 const LocKey& setLoc1,
                                 const LocKey& setLoc2) const
{
  return LocUtil::isBetween(
      setLoc1, setLoc2, ojFrom, ojTo, _vendor, RESERVED, LocUtil::CHK_CITY_INLOC);
}

bool
CombinationsSubCat109::isInLoc(const Loc& loc, const LocKey& locKey) const
{
  return LocUtil::isInLoc(loc,
                          locKey.locType(),
                          locKey.loc(),
                          _vendor,
                          RESERVED,
                          LocUtil::CHK_CITY_INLOC,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          _trx.getRequest()->ticketingDT());
}

bool
CombinationsSubCat109::isInLoc(const Loc& loc, std::vector<const LocKey*>& locVec) const
{
  std::vector<const LocKey*>::iterator locIter = locVec.begin();
  const std::vector<const LocKey*>::const_iterator locIterEnd = locVec.end();
  for (; locIter != locIterEnd; ++locIter)
    if (isInLoc(loc, **locIter))
    {
      locVec.erase(locIter);
      return true;
    }

  return false;
}

void
CombinationsSubCat109::displayDiag()
{
  if (!_diag.isActive())
  {
    return;
  }

  ojrIt ojRestIter = _openJawSetVector->begin();
  const ojrIt ojRestIterEnd = _openJawSetVector->end();
  for (int16_t occur = 1; ojRestIter != ojRestIterEnd; ojRestIter++, occur++)
  {
    const OpenJawRestriction& openJawRestriction = **ojRestIter;

    _diag << "  OCCURRENCE : " << occur << std::endl << "    SET 1 : APPL IND - "
          << openJawRestriction.set1ApplInd()
          << "  LOC 1 : " << openJawRestriction.set1Loc1().locType() << " - "
          << openJawRestriction.set1Loc1().loc()
          << "  LOC 2 : " << openJawRestriction.set1Loc2().locType() << " - "
          << openJawRestriction.set1Loc2().loc() << std::endl << "    SET 2 : APPL IND - "
          << openJawRestriction.set2ApplInd()
          << "  LOC 1 : " << openJawRestriction.set2Loc1().locType() << " - "
          << openJawRestriction.set2Loc1().loc()
          << "  LOC 2 : " << openJawRestriction.set2Loc2().locType() << " - "
          << openJawRestriction.set2Loc2().loc() << std::endl;
  }
}
}
