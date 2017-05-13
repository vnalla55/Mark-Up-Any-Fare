//----------------------------------------------------------------------------
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include "ItinAnalyzer/FamilyLogicSplitter.h"

#include "Common/Assert.h"
#include "Common/ClassOfService.h"
#include "Common/ItinUtil.h"
#include "Common/TrxUtil.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/Itin.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag982Collector.h"
#include "DBAccess/Cabin.h"
#include "ItinAnalyzer/FamilyLogicUtils.h"
#include "ItinAnalyzer/ItinAnalyzerUtils.h"
#include "Util/BranchPrediction.h"

namespace tse
{
static Logger
logger("atseintl.ItinAnalyzer.FamilyLogicSplitter");

static const uint16_t NUMBER_OF_CLASSES_FOR_AVAIL_COMPARE = 9;
namespace
{
uint16_t
getNonArunkSegCount(const PricingTrx::ClassOfServiceKey& key)
{
  return std::count_if(key.cbegin(),
                       key.cend(),
                       [](const TravelSeg* tvlSeg)
                       { return tvlSeg->segmentType() != Arunk; });
}

void
displayDiag982String(PricingTrx& trx, const char* c)
{
  DCFactory* factory = DCFactory::instance();
  Diag982Collector* diag = dynamic_cast<Diag982Collector*>(factory->create(trx));
  diag->enable(Diagnostic982);

  *diag << c << "\n";

  diag->flushMsg();
}

size_t
getNoOfSameSeg(Itin* excItin, Itin* itin, std::vector<bool>& sameSegments)
{
  sameSegments.resize(itin->travelSeg().size(), false);
  for (TravelSeg* excSeg : excItin->travelSeg())
  {
    if (!excSeg->unflown())
    {
      continue;
    }

    size_t index = 0;
    for (TravelSeg* seg : itin->travelSeg())
    {
      if (!seg->unflown())
      {
        sameSegments[index] = true; // flown seg has to be equal
        index++;
        continue;
      }

      if (excSeg->origAirport() != seg->origAirport() ||
          excSeg->destAirport() != seg->destAirport() ||
          excSeg->segmentType() != seg->segmentType())
      {
        index++;
        continue;
      }

      AirSeg* airExcSeg = dynamic_cast<AirSeg*>(excSeg);
      AirSeg* airSeg = dynamic_cast<AirSeg*>(seg);
      if (airExcSeg && airSeg) //  not ArunkSeg or SurfaceSeg
      {
        if (airExcSeg->carrier() != airSeg->carrier() ||
            airExcSeg->departureDT() != airSeg->departureDT() ||
            airExcSeg->flightNumber() != airSeg->flightNumber())
        {
          index++;
          continue;
        }
      }

      sameSegments[index] = true;
      index++;
      break;
    }
  }

  return std::count_if(
      sameSegments.begin(), sameSegments.end(), std::bind2nd(std::equal_to<bool>(), true));
}
bool
sameAsExcItin(Itin* excItin, Itin* itin, size_t noOfSameSeg)
{
  return noOfSameSeg == excItin->travelSeg().size() &&
         excItin->travelSeg().size() == itin->travelSeg().size();
}
double
getCabinAvailScore(const FamilyLogicSplitter::KeyScoreVector& keyScoreVector)
{
  double availScore = 0.0;
  uint16_t allSegCount = 0;
  for (FamilyLogicSplitter::KeyScore keyScore : keyScoreVector)
  {
    availScore += keyScore.second;
    allSegCount += getNonArunkSegCount(keyScore.first);
  }

  return allSegCount ? (availScore / allSegCount) : 0;
}
}

struct HigherAvailFirst
{
  HigherAvailFirst(FamilyLogicSplitter::AvailScoreMap availScoreMap) : _availScoreMap(availScoreMap)
  {
  }

  bool operator()(Itin* l, Itin* r) { return _availScoreMap[l] > _availScoreMap[r]; }

  FamilyLogicSplitter::AvailScoreMap _availScoreMap;
};

class JourneyAvailScore
{
public:
  JourneyAvailScore(bool calculateLocalScore)
    : _numberOfSegments(0), _calculateLocalScore(calculateLocalScore), _segmentScore(0)
  {
  }

  void addBookingCode(const BookingCode& bookingCode, int16_t classOrder)
  {
    if (classOrder <= 0)
      return;

    if (_calculateLocalScore)
    {
      _bookingCodeLegCount[bookingCode] += 1;
      _bookingCodeAvailOrder[bookingCode] += classOrder;
    }
    else
      _segmentScore += classOrder;
  }

  void addSegment() { ++_numberOfSegments; }
  uint16_t getSegmentCount() { return _numberOfSegments; }

  uint16_t getLeastAvailScore()
  {
    uint16_t maxScore = 0;
    std::map<const BookingCode, uint16_t>::const_iterator i = _bookingCodeLegCount.begin();
    std::map<const BookingCode, uint16_t>::const_iterator iEnd = _bookingCodeLegCount.end();

    for (; i != iEnd; ++i)
      if ((i->second == _numberOfSegments) && (_bookingCodeAvailOrder[i->first] > maxScore))
        maxScore = _bookingCodeAvailOrder[i->first];

    return maxScore;
  }

  uint16_t getCabinAvailScore()
  {
    uint16_t score = 0;

    if (_calculateLocalScore)
    {
      std::map<const BookingCode, uint16_t>::const_iterator i = _bookingCodeLegCount.begin();
      std::map<const BookingCode, uint16_t>::const_iterator iEnd = _bookingCodeLegCount.end();

      for (; i != iEnd; ++i)
        if (i->second == _numberOfSegments)
          score += _bookingCodeAvailOrder[i->first];
    }
    else
      score = _segmentScore;

    return score;
  }

private:
  std::map<const BookingCode, uint16_t> _bookingCodeLegCount;
  std::map<const BookingCode, uint16_t> _bookingCodeAvailOrder;
  uint16_t _numberOfSegments;
  bool _calculateLocalScore;
  uint16_t _segmentScore;
};

void
FamilyLogicSplitter::splitItinFamilies(bool splitItinsForDomesticOnly)
{
  if (_trx.isBRAll()) //for BRALL requests the whole travel must be the same
    splitFamiliesBasedOnTravel();

  std::vector<Itin*> splittedItins;

  for (Itin*& itin : _trx.itin())
    checkItinFamily(itin, splittedItins, splitItinsForDomesticOnly, true);

  // Merge new families with the old once
  _trx.itin().insert(_trx.itin().end(), splittedItins.begin(), splittedItins.end());
  if (!_trx.flpVector().empty())
  {
    const uint16_t origItinSize = _trx.itin().size();
    bool diag982 = FamilyLogicUtils::diag982DDAllActive(_trx);
    if (diag982)
      displayDiag982String(_trx, "***** START FAMILY SPLITTING DIAGNOSTICS *****\n");

    ItinAvailKeys itinAvailKeys;
    AvailScoreMap cabinAvailScores;
    getItinCOSKeys(itinAvailKeys, cabinAvailScores);

    uint16_t numSplits = _trx.flpVector()[PricingTrx::NUM_FAMILY_BOOKED_SPLITS];
    if (numSplits > 0)
      splitFamiliesBasedOnBookedAvailibility(itinAvailKeys, false, numSplits);

    numSplits = _trx.flpVector()[PricingTrx::NUM_FAMILY_MOTHER_BOOKED_SPLITS];

    for (int i = 0; i < numSplits; ++i)
      splitFamiliesBasedOnBookedAvailibility(itinAvailKeys, true, 1);

    splitFamiliesBasedOnCabinAvailibility(cabinAvailScores);

    if (diag982)
    {
      std::stringstream ss;
      ss << "\nITIN SIZE BEFORE SPLITS: " << origItinSize << "\n"
         << "ITIN SIZE AFTER SPLITS: " << _trx.itin().size() << "\n"
         << "\n***** END FAMILY SPLITTING DIAGNOSTICS *****\n";

      displayDiag982String(_trx, ss.str().c_str());
    }
  }

  std::vector<Itin*> excSplittedItins; // new families
  std::vector<Itin*> excRemovedItins; // removed itin
  RexExchangeTrx* excTrx = dynamic_cast<RexExchangeTrx*>(&_trx);
  if (excTrx)
  {
    for (Itin* itin : _trx.itin())
    {
      if (!checkExcItinFamily(excTrx, itin, excSplittedItins))
      {
        excRemovedItins.push_back(itin);
      }
    }

    // Merge new families with the old once
    _trx.itin().insert(_trx.itin().end(), excSplittedItins.begin(), excSplittedItins.end());

    if (_trx.itin().size() > excRemovedItins.size())
      eraseItins(excRemovedItins);
  }

  if (!_trx.flpVector().empty())
    diag982(splittedItins,
            (excTrx ? excTrx->exchangeItin().front() : nullptr),
            excRemovedItins,
            excSplittedItins);
}

Itin*
FamilyLogicSplitter::createFamily(std::vector<SimilarItinData>& itinVec)
{
  if (itinVec.empty())
    return nullptr;

  Itin* mother = itinVec.front().itin;
  itinVec.erase(itinVec.begin());

  if (!itinVec.empty())
    mother->addSimilarItins(itinVec.begin(), itinVec.end());

  return mother;
}

namespace {
  std::string getItinTravelString(const Itin* itin)
  {
    std::string result;
    for (TravelSeg* tSeg: itin->travelSeg())
    {
      if (!tSeg->isAir())
      {
        result.append("#|");
        continue;
      }
      result.append(tSeg->toAirSeg()->origin()->loc());
      result.append("-");
      result.append(tSeg->toAirSeg()->marketingCarrierCode());
      result.append("-");
      result.append(tSeg->toAirSeg()->destination()->loc());
      result.append("|");
    }
    return result;
  }
}

void
FamilyLogicSplitter::splitFamiliesBasedOnTravel()
{
  std::vector<Itin*> newMothers, erasedMothers;
  std::stringstream diag;
  bool diag982 = FamilyLogicUtils::diag982DDAllActive(_trx);

  if (UNLIKELY(diag982))
    diag << "\n*** SPLITTING ITINS USING TRAVEL INFORMATION ***\n";

  for (Itin* itin : _trx.itin())
  {
    std::map<std::string, std::vector<Itin*>> travelBasedMap;
    travelBasedMap[getItinTravelString(itin)].push_back(itin);
    for (const SimilarItinData& similarItinData : itin->getSimilarItins())
      travelBasedMap[getItinTravelString(similarItinData.itin)].push_back(similarItinData.itin);

    if (travelBasedMap.size() < 2)
      continue; //no split

    if (UNLIKELY(diag982))
    {
      diag << "\nITIN " << itin->itinNum() << " WILL SPLIT INTO " << travelBasedMap.size() << "\n";
      diag << "BEFORE SPLIT:\n";
      FamilyLogicUtils::printFamily(itin, diag);
      diag << "AFTER SPLIT:\n";
    }

    erasedMothers.push_back(itin);
    itin->clearSimilarItins();

    for (const auto& itins: travelBasedMap)
    {
      std::vector<Itin*>::const_iterator i = itins.second.begin();
      Itin* newMother = *i;
      newMothers.push_back(newMother);
      ++i;
      for (; i != itins.second.end(); ++i)
        newMother->addSimilarItin(*i);
      if (UNLIKELY(diag982))
        FamilyLogicUtils::printFamily(newMother, diag);
    }
  }

  eraseItins(erasedMothers);
  _trx.itin().insert(_trx.itin().end(), newMothers.begin(), newMothers.end());

  if (UNLIKELY(diag982))
  {
    if (newMothers.empty())
      diag << "  NO SPLITS\n";
    displayDiag982String(_trx, diag.str().c_str());
  }
}

void
FamilyLogicSplitter::splitFamiliesBasedOnBookedAvailibility(ItinAvailKeys& itinAvailKeys,
                                                            bool useMotherAvail,
                                                            uint16_t numSplits)
{
  std::vector<Itin*> newMothers, erasedMothers;
  std::stringstream diag;
  bool diag982 = FamilyLogicUtils::diag982DDAllActive(_trx);

  if (diag982)
    diag << "\n*** SPLITTING ITINS USING " << (useMotherAvail ? "MOTHER'S" : "")
         << " BOOKED AVAILIBILITY ***\n";

  for (Itin* itin : _trx.itin())
  {
    if (!itin->getSimilarItins().empty())
    {
      std::vector<Itin*> validItins, invalidItins;

      const bool motherValid = processBookingCodes(itin, itinAvailKeys[itin], itin->origBooking());
      if (!motherValid)
      {
        if (useMotherAvail) // This means all children are already invalid from previous step
          continue;

        invalidItins.push_back(itin);
      }

      for (const SimilarItinData& similarItinData : itin->getSimilarItins())
      {
        Itin& similarItin = *similarItinData.itin;
        const bool childValid =
            processBookingCodes(&similarItin,
                                itinAvailKeys[&similarItin],
                                useMotherAvail ? itin->origBooking() : similarItin.origBooking());

        childValid ? validItins.push_back(&similarItin) : invalidItins.push_back(&similarItin);
      }

      if (invalidItins.empty() || (invalidItins.size() == itin->getSimilarItins().size() + 1))
        continue;

      if (diag982)
      {
        diag << "\nITIN " << itin->itinNum() << " WILL SPLIT INTO " << numSplits + 1 << "\n";
        diag << "BEFORE SPLIT:\n";
        FamilyLogicUtils::printFamily(itin, diag);
        diag << "AFTER SPLIT:\n";
      }

      if (motherValid)
      {
        cleanInvalidatedItins(itin, invalidItins);
        if (diag982)
          FamilyLogicUtils::printFamily(itin, diag);
      }
      else
      {
        erasedMothers.push_back(itin);
        itin->clearSimilarItins();

        std::vector<Itin*>::const_iterator i = validItins.begin();
        Itin* newMother = *i++;
        newMothers.push_back(newMother);
        for (; i != validItins.end(); ++i)
          newMother->addSimilarItin(*i);

        if (diag982)
          FamilyLogicUtils::printFamily(newMother, diag);
      }

      addNewMothers(invalidItins, newMothers, numSplits, nullptr, diag);
    }
  }

  eraseItins(erasedMothers);
  _trx.itin().insert(_trx.itin().end(), newMothers.begin(), newMothers.end());

  if (diag982)
  {
    if (newMothers.empty())
      diag << "  NO SPLITS\n";

    displayDiag982String(_trx, diag.str().c_str());
  }
}

void
FamilyLogicSplitter::eraseItins(const std::vector<Itin*>& erasedItins)
{
  for (Itin* rmItin : erasedItins)
  {
    std::vector<Itin*>::iterator found = std::find(_trx.itin().begin(), _trx.itin().end(), rmItin);
    if (found != _trx.itin().end())
      _trx.itin().erase(found);
    else
      LOG4CXX_ERROR(logger, "Removed itin was not found in_trx itins.");
  }
}

void
FamilyLogicSplitter::printUpdatedMother(Itin* updatedItin,
                                        AvailScoreMap& availScoreMap,
                                        std::stringstream& diag)
{
  diag << "UPDATED FAMILY:\n";

  diag << "MOTHER SCORE FOR UPDATED ITIN " << updatedItin->itinNum() << ": "
       << availScoreMap[updatedItin] << "\n";

  for (const SimilarItinData& similarItinData : updatedItin->getSimilarItins())
    diag << "  CHILD SCORE FOR ITIN " << similarItinData.itin->itinNum() << ": "
         << availScoreMap[similarItinData.itin] << "\n";
}

void
FamilyLogicSplitter::addNewMothers(std::vector<Itin*>& itinsToSplit,
                                   std::vector<Itin*>& newMothers,
                                   uint16_t splitSize,
                                   AvailScoreMap* availScoreMap,
                                   std::stringstream& diag)
{
  std::vector<Itin*>::iterator i = itinsToSplit.begin();
  std::vector<Itin*>::iterator iEnd = itinsToSplit.end();

  std::vector<Itin*> newMothersTemp;

  // Add "splitSize" mothers, and then equally distribute the rest among mothers.
  for (uint16_t index = 0; i != iEnd; ++index, ++i)
  {
    if (index < splitSize)
      newMothersTemp.push_back(*i);
    else
      newMothersTemp[index % splitSize]->addSimilarItin(*i);
  }

  newMothers.insert(newMothers.end(), newMothersTemp.begin(), newMothersTemp.end());

  if (!diag.str().empty())
  {
    for (Itin* itin : newMothersTemp)
      availScoreMap ? printUpdatedMother(itin, *availScoreMap, diag)
                    : FamilyLogicUtils::printFamily(itin, diag);
  }
}

void
FamilyLogicSplitter::splitFamiliesBasedOnCabinAvailibility(AvailScoreMap& availScoreMap)
{
  uint16_t numSplits = _trx.flpVector()[PricingTrx::NUM_FAMILY_AVAIL_SPLITS];
  if (numSplits == 0)
    return;

  uint16_t splitThresholdPercentage =
      _trx.flpVector()[PricingTrx::PERCENTAGE_DIFF_FOR_FAMILY_SPLIT];

  double splitThreshold = 1 + ((double)splitThresholdPercentage / 100);

  std::vector<Itin*> newMothers, erasedMothers;
  std::stringstream diag;
  const bool diag982 = FamilyLogicUtils::diag982DDAllActive(_trx);

  if (UNLIKELY(diag982))
    diag << "\n*** SPLITTING ITINS USING AVAILIBILITY SCORE ***\n";

  for (Itin* itin : _trx.itin())
  {
    if (!itin->getSimilarItins().empty())
    {
      std::vector<Itin*> betterChildItins;
      double motherAvailScore = availScoreMap[itin];
      if (UNLIKELY(diag982))
        diag << "\nMOTHER SCORE FOR ITIN " << itin->itinNum() << ": " << motherAvailScore << "\n";

      for (const SimilarItinData& similarItinData : itin->getSimilarItins())
      {
        Itin& similarItin = *similarItinData.itin;
        double childScore = availScoreMap[&similarItin];
        if (UNLIKELY(diag982))
          diag << "  CHILD SCORE FOR ITIN " << similarItin.itinNum() << ": " << childScore << "\n";

        if (childScore > (motherAvailScore * splitThreshold))
          betterChildItins.push_back(&similarItin);
      }

      if (!betterChildItins.empty()) // means at least one child itin has better score
      {
        if (UNLIKELY(diag982))
        {
          diag << betterChildItins.size() << " CHILDREN HAVE BETTER SCORES\n"
               << "FAMILY WILL BE SPLIT INTO " << numSplits + 1 << "\n";
        }

        cleanInvalidatedItins(itin, betterChildItins);
        printUpdatedMother(itin, availScoreMap, diag);

        std::sort(
            betterChildItins.begin(), betterChildItins.end(), HigherAvailFirst(availScoreMap));

        addNewMothers(betterChildItins, newMothers, numSplits, &availScoreMap, diag);
      }
    }
  }

  _trx.itin().insert(_trx.itin().end(), newMothers.begin(), newMothers.end());
  eraseItins(erasedMothers);

  if (UNLIKELY(diag982))
  {
    if (newMothers.empty())
      diag << "NO SPLITS\n";

    displayDiag982String(_trx, diag.str().c_str());
  }
}

void
FamilyLogicSplitter::getBestItinKeys(Itin* itin, KeyScoreVector& keyScoreVector)
{
  itinanalyzerutils::setItinLegs(itin);

  std::vector<TravelSegJourneyType> journeyOrder;
  std::vector<FareMarket*> jnyMarkets;
  ItinUtil::journeys(*itin, _trx, jnyMarkets, &journeyOrder);

  uint16_t segIndex = 0;
  std::stringstream diag;

  bool diag982 = FamilyLogicUtils::diag982DDAllActive(_trx);
  if (diag982)
  {
    diag << "\n*** PROCESSING AVAILIBILITY FOR ITIN " << itin->itinNum() << " ***";
    displayDiag982String(_trx, diag.str().c_str());
  }

  for (PricingTrx::ClassOfServiceKey& key : itin->itinLegs())
  {
    const uint16_t segCount = getNonArunkSegCount(key);

    if ((segCount < 2) || (segCount > 3)) // no journey processing
    {
      if (diag982)
      {
        diag << "\nSEGMENT SIZE: " << segCount << ". NO JOURNEY PROCESSING.\n";
        displayDiag982String(_trx, diag.str().c_str());
      }

      uint16_t legAvailScore = 0;
      processBookingCodeForSegments(key, itin, &legAvailScore);
      keyScoreVector.push_back(std::make_pair(key, legAvailScore));
    }
    else
    {
      std::vector<TravelSegJourneyType> legJourneyOrder;
      for (size_t i = 0; i < key.size(); ++i, ++segIndex)
        legJourneyOrder.push_back(journeyOrder[segIndex]);

      if (processFlowJourneyCarriers(itin, legJourneyOrder, key, keyScoreVector))
        continue;

      if (processLocalJourneyCarriers(itin, legJourneyOrder, key, keyScoreVector))
        continue;

      processNonJourneyCarriers(itin, key, keyScoreVector);
    }
  }
}

void
FamilyLogicSplitter::processNonJourneyCarriers(Itin* itin,
                                               const PricingTrx::ClassOfServiceKey& key,
                                               KeyScoreVector& keyScoreVector)
{
  uint16_t legAvailScore = 0;
  processBookingCodeForSegments(key, itin, &legAvailScore);

  KeyScoreVector flightKeyScoreVector;
  uint16_t flightAvailScore = 0;
  for (TravelSeg* tvl : key)
  {
    PricingTrx::ClassOfServiceKey key;
    key.push_back(tvl);

    uint16_t score = 0;
    processBookingCodeForSegments(key, itin, &score);

    flightAvailScore += score;
    flightKeyScoreVector.push_back(std::make_pair(key, score));
  }

  bool diag982 = FamilyLogicUtils::diag982DDAllActive(_trx);

  if (legAvailScore >= flightAvailScore)
  {
    keyScoreVector.push_back(std::make_pair(key, legAvailScore));
    if (diag982)
      displayDiag982String(_trx, "THRU AVAILIBILITY WILL BE USED FOR NON-JOURNEY SEGMENTS\n");
  }
  else
  {
    keyScoreVector.insert(
        keyScoreVector.end(), flightKeyScoreVector.begin(), flightKeyScoreVector.end());
    if (diag982)
      displayDiag982String(_trx, "LOCAL AVAILIBILITY WILL BE USED FOR NON-JOURNEY SEGMENTS\n");
  }
}

bool
FamilyLogicSplitter::processLocalJourneyCarriers(
    Itin* itin,
    const std::vector<TravelSegJourneyType>& legJourneyOrder,
    const PricingTrx::ClassOfServiceKey& key,
    KeyScoreVector& keyScoreVector)
{
  std::vector<TravelSeg*> localJourneySegments;

  uint16_t legIndex = 0;
  for (TravelSeg* tvl : key)
  {
    if (legJourneyOrder[legIndex++] == TravelSegJourneyType::LocalJourneySegment)
      localJourneySegments.push_back(tvl);
  }

  uint16_t flowLen = localJourneySegments.size();
  if (flowLen < 2)
    return false;

  uint16_t flowAvailScore = 0;
  processBookingCodeForSegments(localJourneySegments, itin, &flowAvailScore, true, true);

  // Now try segment by segment
  uint16_t localAvailScore = 0;
  for (TravelSeg* tSeg : localJourneySegments)
  {
    PricingTrx::ClassOfServiceKey key;
    key.push_back(tSeg);

    uint16_t score = 0;
    processBookingCodeForSegments(key, itin, &score, true, true);

    localAvailScore += score;
  }

  bool useFlowAvail = flowAvailScore >= localAvailScore;

  uint16_t index = 0;
  if (FamilyLogicUtils::diag982DDAllActive(_trx))
  {
    std::string diag = (useFlowAvail ? "THRU" : "LOCAL");
    diag += " AVAILIBILITY WILL BE USED FOR LOCAL JOURNEY SEGMENTS\n";
    displayDiag982String(_trx, diag.c_str());
  }
  std::vector<TravelSeg*>::const_iterator tvlI = key.begin();
  while (tvlI != key.end())
  {
    PricingTrx::ClassOfServiceKey key;
    if (useFlowAvail && (legJourneyOrder[index] == TravelSegJourneyType::LocalJourneySegment))
    {
      for (uint16_t i = 0; i < flowLen; ++i)
      {
        key.push_back(*tvlI);
        ++tvlI;
        ++index;
      }

      uint16_t score = 0;
      processBookingCodeForSegments(key, itin, &score, true);

      keyScoreVector.push_back(std::make_pair(key, score));

      useFlowAvail = false; // only one journey per leg
    }
    else
    {
      key.push_back(*tvlI);

      uint16_t score = 0;
      processBookingCodeForSegments(key, itin, &score);

      keyScoreVector.push_back(std::make_pair(key, score));

      ++tvlI;
      ++index;
    }
  }

  return true;
}

bool
FamilyLogicSplitter::processFlowJourneyCarriers(
    Itin* itin,
    const std::vector<TravelSegJourneyType>& legJourneyOrder,
    const PricingTrx::ClassOfServiceKey& key,
    KeyScoreVector& keyScoreVector)
{
  uint16_t flowLen = 0;
  for (const TravelSegJourneyType segType : legJourneyOrder)
    if (segType == TravelSegJourneyType::FlowJourneySegment) // if a flow journey exists, we should
      flowLen++; // use the key intact

  if (flowLen < 2)
    return false;

  std::vector<TravelSeg*>::const_iterator tvlI = key.begin();
  std::vector<TravelSeg*>::const_iterator tvlIEnd = key.end();

  uint16_t index = 0;
  bool useFlowAvail = true;

  while (tvlI != tvlIEnd)
  {
    PricingTrx::ClassOfServiceKey key;

    if (useFlowAvail && (legJourneyOrder[index] == TravelSegJourneyType::FlowJourneySegment))
    {
      for (uint16_t i = 0; i < flowLen; ++i)
      {
        key.push_back(*tvlI);
        ++tvlI;
        ++index;
      }

      uint16_t score = 0;
      processBookingCodeForSegments(key, itin, &score);

      keyScoreVector.push_back(std::make_pair(key, score));

      useFlowAvail = false; // only one journey per leg
    }
    else
    {
      key.push_back(*tvlI);

      uint16_t score = 0;
      processBookingCodeForSegments(key, itin, &score);

      keyScoreVector.push_back(std::make_pair(key, score));

      ++tvlI;
      ++index;
    }
  }

  if (_trx.diagnostic().diagnosticType() == Diagnostic982)
    displayDiag982String(_trx, "THRU AVAILIBILITY WILL BE USED FOR FLOW JOURNEY SEGMENTS\n");

  return true;
}

bool
FamilyLogicSplitter::processBookingCodeForSegments(const PricingTrx::ClassOfServiceKey& key,
                                                   Itin* itin,
                                                   uint16_t* score,
                                                   bool calculateLocalJourneyScore,
                                                   bool useLeastAvail)
{
  if (getNonArunkSegCount(key) == 0)
    return true;

  bool diag982 = FamilyLogicUtils::diag982DDAllActive(_trx);
  if (diag982)
    displayKeySegments(key);

  std::vector<ClassOfServiceList>& cosListVector =
      ShoppingUtil::getClassOfService(_trx, key);

  if (cosListVector.empty() || key.size() != cosListVector.size())
  {
    LOG4CXX_ERROR(logger, "Class of service list could not be found.");
    return true;
  }

  std::stringstream diag;
  JourneyAvailScore journeyAvailScore(calculateLocalJourneyScore);

  std::vector<TravelSeg*>::const_iterator tvlI = key.begin();
  std::vector<TravelSeg*>::const_iterator tvlIEnd = key.end();
  for (int index = 0; tvlI != tvlIEnd; ++tvlI, ++index)
  {
    TravelSeg* seg = *tvlI;
    ClassOfServiceList& cosList = cosListVector[index];

    if (seg->segmentType() == Arunk)
      continue;

    journeyAvailScore.addSegment();

    int segPos = FamilyLogicUtils::segmentOrderWithoutArunk(itin, seg);
    if (segPos > static_cast<int>(itin->origBooking().size()))
    {
      LOG4CXX_ERROR(logger, "Segment position exceeds origBooking vector size");
      return false;
    }

    ClassOfService* cos = itin->origBooking()[segPos - 1];

    int16_t classOrder = NUMBER_OF_CLASSES_FOR_AVAIL_COMPARE + 1;
    std::vector<ClassOfService*>::const_reverse_iterator cosI = cosList.rbegin();
    std::vector<ClassOfService*>::const_reverse_iterator cosIEnd = cosList.rend();
    for (; cosI != cosIEnd; ++cosI)
    {
      if ((*cosI)->cabin() == cos->cabin())
      {
        if ((*cosI)->numSeats() >= _requestedNumberOfSeats)
          journeyAvailScore.addBookingCode((*cosI)->bookingCode(), classOrder);

        if (UNLIKELY(diag982))
          diag << (*cosI)->bookingCode() << (*cosI)->numSeats() << "("
               << (classOrder > 0 ? classOrder : 0) << ") ";

        --classOrder;
      }
    }

    if (diag982)
      diag << "\n";
  }

  if (useLeastAvail)
  {
    *score = journeyAvailScore.getLeastAvailScore();
    if (diag982)
      diag << "LEAST AVAIL SCORE: " << *score;
  }
  else
  {
    *score = journeyAvailScore.getCabinAvailScore();
    if (diag982)
      diag << "CABIN AVAIL SCORE: " << *score;
  }

  if (diag982)
    displayDiag982String(_trx, diag.str().c_str());

  return true;
}

bool
FamilyLogicSplitter::processBookingCodeForSegments(const PricingTrx::ClassOfServiceKey& key,
                                                   Itin* itin,
                                                   const std::vector<ClassOfService*>& origBooking)
{
  if (getNonArunkSegCount(key) == 0)
    return true;

  std::vector<ClassOfServiceList>& cosListVector =
      ShoppingUtil::getClassOfService(_trx, key);

  if (cosListVector.empty() || key.size() != cosListVector.size())
  {
    LOG4CXX_ERROR(logger, "Class of service list could not be found.");
    return false;
  }

  std::vector<TravelSeg*>::const_iterator tvlI = key.begin();
  std::vector<TravelSeg*>::const_iterator tvlIEnd = key.end();
  for (int index = 0; tvlI != tvlIEnd; ++tvlI, ++index)
  {
    TravelSeg* seg = *tvlI;
    bool bkcValidForSegments = false;
    ClassOfServiceList& cosList = cosListVector[index];

    if (seg->segmentType() == Arunk)
      continue;

    int segPos = FamilyLogicUtils::segmentOrderWithoutArunk(itin, seg);
    if (segPos > static_cast<int>(origBooking.size()))
      return false;

    ClassOfService* origCos = origBooking[segPos - 1];
    std::vector<ClassOfService*>::const_reverse_iterator cosI = cosList.rbegin();
    std::vector<ClassOfService*>::const_reverse_iterator cosIEnd = cosList.rend();
    for (; cosI != cosIEnd; ++cosI)
    {
      if ((*cosI)->bookingCode() == origCos->bookingCode())
      {
        if ((*cosI)->numSeats() >= _requestedNumberOfSeats)
        {
          bkcValidForSegments = true;
          break;
        }
        else
          return false;
      }
    }

    if (!bkcValidForSegments)
      return false;
  }

  return true;
}

void
FamilyLogicSplitter::displayKeySegments(const PricingTrx::ClassOfServiceKey& key)
{
  DCFactory* factory = DCFactory::instance();
  Diag982Collector* diag = dynamic_cast<Diag982Collector*>(factory->create(_trx));
  diag->enable(Diagnostic982);

  diag->displayKeySegments(key);

  diag->flushMsg();
}

bool
FamilyLogicSplitter::processBookingCodes(Itin* itin,
                                         const KeyScoreVector& keys,
                                         const std::vector<ClassOfService*>& origBooking)
{
  for (const KeyScore& keyScore : keys)
  {
    const PricingTrx::ClassOfServiceKey& key = keyScore.first;
    if (key.empty())
    {
      LOG4CXX_ERROR(logger, "Class of service key is empty.");
      continue;
    }

    if (!processBookingCodeForSegments(key, itin, origBooking))
      return false;
  }

  return true;
}

void
FamilyLogicSplitter::diag982(const std::vector<Itin*>& splittedItins,
                             const Itin* excItin,
                             const std::vector<Itin*>& excRemovedItins,
                             const std::vector<Itin*>& excSplittedItins)
{
  if (_trx.diagnostic().diagnosticType() != Diagnostic982)
    return;

  DCFactory* factory = DCFactory::instance();
  Diag982Collector* diag = dynamic_cast<Diag982Collector*>(factory->create(_trx));
  diag->enable(Diagnostic982);

  diag->displaySplittedItins(_trx, splittedItins, excItin, excRemovedItins, excSplittedItins);

  diag->flushMsg();
}

bool
FamilyLogicSplitter::checkExcItinFamily(RexExchangeTrx* excTrx,
                                        Itin* itin,
                                        std::vector<Itin*>& splitItins)
{
  Itin* excItin = excTrx->exchangeItin().front();
  std::vector<Itin*> removedItins;

  std::map<std::vector<bool>, std::vector<SimilarItinData>> newFamilies;
  for (const SimilarItinData& similarItinData : itin->getSimilarItins())
  {
    Itin* smItin = similarItinData.itin;
    std::vector<bool> sameSegments; // Determines which seg is the same.
    // Itins with the same sameSegments creates new family
    size_t noOfSameSeg = getNoOfSameSeg(excItin, smItin, sameSegments);
    if (sameAsExcItin(excItin, smItin, noOfSameSeg))
    {
      removedItins.push_back(smItin);
    }
    else if (noOfSameSeg > 0)
    {
      removedItins.push_back(smItin);
      newFamilies[sameSegments].emplace_back(smItin);
    }
  }

  for (Itin* rmItin : removedItins)
  {
    if (!itin->eraseSimilarItin(rmItin))
    {
      LOG4CXX_ERROR(logger, "Valid child do not found in similar itins");
    }
  }

  std::vector<bool> motherSameSegments;
  size_t motherNoOfSameSeg = getNoOfSameSeg(excItin, itin, motherSameSegments);
  bool motherTheSameAsExc = sameAsExcItin(excItin, itin, motherNoOfSameSeg);

  if (motherNoOfSameSeg > 0)
  {
    if (!itin->getSimilarItins().empty())
    {
      std::vector<SimilarItinData> similarItins(itin->clearSimilarItins());

      Itin* newMother = createFamily(similarItins);
      splitItins.push_back(newMother);
    }
  }

  for (auto& newFamily : newFamilies)
  {
    if (!motherTheSameAsExc && motherNoOfSameSeg > 0 && motherSameSegments == newFamily.first)
    {
      itin->addSimilarItins(newFamily.second.begin(), newFamily.second.end());
    }
    else
    {
      Itin* newMother = createFamily(newFamily.second);
      splitItins.push_back(newMother);
    }
  }

  return !motherTheSameAsExc;
}

void
FamilyLogicSplitter::checkItinFamily(Itin*& itin,
                                     std::vector<Itin*>& splitItins,
                                     bool splitItinsForDomesticOnly,
                                     bool checkChildren)
{
  if (itin->getSimilarItins().empty())
    return;

  std::vector<SimilarItinData> tempItinVec;
  bool motherInvalid = false;

  // Check mother
  if (!checkItin(itin, itin->origBooking()))
  {
    LOG4CXX_DEBUG(logger, "Mother is invalid");
    motherInvalid = true;
  }
  Itin* firstValid(nullptr);
  // Check children
  for (const SimilarItinData& similarItinData : itin->getSimilarItins())
  {
    Itin& similarItin = *similarItinData.itin;
    const std::vector<ClassOfService*>& origBooking =
        (motherInvalid) ? similarItin.origBooking() : itin->origBooking();

    // Check child
    if (!checkItin(&similarItin, origBooking))
    {
      LOG4CXX_DEBUG(logger, "Child is invalid");
      tempItinVec.emplace_back(&similarItin);
    }
    else if (!firstValid && splitItinsForDomesticOnly &&
             itin->geoTravelType() != GeoTravelType::Domestic)
    {
      firstValid = &similarItin;
    }
  }

  // Neither mother nor children were invalid = finish
  if (tempItinVec.empty() && !motherInvalid)
  {
    LOG4CXX_DEBUG(logger, "Entire Family is valid");
    return;
  }

  // Mother and children are all invalid = finish
  if (tempItinVec.size() == itin->getSimilarItins().size() && motherInvalid == true)
  {
    LOG4CXX_DEBUG(logger, "Entire Family is invalid");
    return;
  }

  // Check confguration, if we split only for domestic then international must be treat special way
  if (splitItinsForDomesticOnly && itin->geoTravelType() != GeoTravelType::Domestic)
  {
    LOG4CXX_DEBUG(logger, "Split itins for domestic only");
    if (motherInvalid && firstValid)
    {
      LOG4CXX_DEBUG(logger, "Split itins for domestic only-exchange mother");

      if (!itin->rotateHeadOfSimilarItins(firstValid))
      {
        LOG4CXX_ERROR(logger, "Valid child do not found in similar itins");
        return;
      }

      itin = firstValid;
    }
    return;
  }

  // Detach all invalid child itins
  cleanInvalidatedItins(itin, tempItinVec);

  if (motherInvalid)
  {
    LOG4CXX_DEBUG(logger,
                  "Mother invalid - swap childs from mother(" << itin->getSimilarItins().size()
                                                              << ") with invalid itins("
                                                              << tempItinVec.size() << ")");
    // Because mother is invalid we put all invalid child itins to her similarItins vector.
    //  And move all valid itins from mother into the temp vector so they could be transformed into
    // a new family later on
    itin->swapSimilarItins(tempItinVec);
  }

  // Create a new family based on the Itins in temp vector
  Itin* newmother = createFamily(tempItinVec);
  if (nullptr == newmother)
  {
    LOG4CXX_ERROR(logger, "Cannot create new family");
    // We were not able to create a new family
    return;
  }

  if (checkChildren)
    checkItinFamily(newmother, splitItins, splitItinsForDomesticOnly, false);

  splitItins.push_back(newmother);
}
void
FamilyLogicSplitter::getItinCOSKeys(ItinAvailKeys& itinAvailKeys, AvailScoreMap& cabinAvailScores)
{
  for (Itin* itin : _trx.itin())
  {
    if (!itin->getSimilarItins().empty())
    {
      getBestItinKeys(itin, itinAvailKeys[itin]);
      cabinAvailScores[itin] = getCabinAvailScore(itinAvailKeys[itin]);

      for (const SimilarItinData& similarItinData : itin->getSimilarItins())
      {
        Itin* childItin = similarItinData.itin;
        getBestItinKeys(childItin, itinAvailKeys[childItin]);
        cabinAvailScores[childItin] = getCabinAvailScore(itinAvailKeys[childItin]);
      }
    }
  }
}

//
// Check if an itin if still valid
bool
FamilyLogicSplitter::checkItin(Itin* itin, const std::vector<ClassOfService*>& origBooking)
{
  std::vector<PricingTrx::ClassOfServiceKey> keys = findAllCOSKeys(itin);

  if (keys.empty())
  {
    LOG4CXX_ERROR(logger, "Could not find any ClassOfService keys");
    return false;
  }
  bool bkcValid = false;

  for (PricingTrx::ClassOfServiceKey& key : keys)
  {
    TSE_ASSERT(!key.empty());
    std::vector<ClassOfServiceList>& cosListVector =
        ShoppingUtil::getClassOfService(_trx, key);

    if (cosListVector.empty())
    {
      LOG4CXX_ERROR(logger, "Class of service list could not be found");
      continue;
    }

    TSE_ASSERT(key.size() == cosListVector.size());
    std::vector<TravelSeg*>::iterator tvlI = key.begin();
    std::vector<TravelSeg*>::const_iterator tvlIEnd = key.end();

    for (int index = 0; tvlI != tvlIEnd; ++tvlI, ++index)
    {
      TravelSeg* seg = *tvlI;
      bkcValid = false;
      ClassOfServiceList& cosList = cosListVector[index];

      if (seg->segmentType() == Arunk)
      {
        bkcValid = true;
        continue;
      }

      int segPos = FamilyLogicUtils::segmentOrderWithoutArunk(itin, seg);
      if (segPos > static_cast<int>(origBooking.size()))
        return false;

      ClassOfService* cos = origBooking[segPos - 1];

      std::vector<ClassOfService*>::iterator cosI = cosList.begin();
      std::vector<ClassOfService*>::iterator cosIEnd = cosList.end();
      for (; cosI != cosIEnd; ++cosI)
      {
        if ((*cosI)->bookingCode() == cos->bookingCode() && (*cosI)->numSeats() > 0)
        {
          bkcValid = true;
          break;
        }
      }
      if (!bkcValid)
      {
        break;
      }
    }
    if (!bkcValid)
    {
      break;
    }
  }

  return bkcValid;
}

std::vector<PricingTrx::ClassOfServiceKey>
FamilyLogicSplitter::findAllCOSKeys(Itin* itin)
{
  std::vector<PricingTrx::ClassOfServiceKey> keyVector;
  std::vector<TravelSeg*>& tvlSegV = itin->travelSeg();

  std::vector<PricingTrx::OriginDestination>::iterator i = _trx.originDest().begin();

  for (; i != _trx.originDest().end(); ++i)
  {
    bool found = false;
    std::vector<TravelSeg*>::iterator boardOfOND;
    std::vector<TravelSeg*>::iterator offOfOND;

    for (offOfOND = tvlSegV.begin(); offOfOND != tvlSegV.end(); ++offOfOND)
    {
      if ((*offOfOND)->offMultiCity() != i->offMultiCity)
        continue;
      for (boardOfOND = tvlSegV.begin(); boardOfOND != tvlSegV.end(); ++boardOfOND)
      {
        if ((*boardOfOND)->boardMultiCity() != i->boardMultiCity)
          continue;
        if (LIKELY(offOfOND - boardOfOND >= 0))
        {
          if ((*boardOfOND)->departureDT().get64BitRepDateOnly() ==
              i->travelDate.get64BitRepDateOnly())
          {
            found = true;
            break;
          }
        }
      }
      if (found)
        break;
    }

    if (found)
    {
      PricingTrx::ClassOfServiceKey key;
      std::vector<TravelSeg*>::iterator segIter = boardOfOND;
      for (; segIter != offOfOND + 1; ++segIter)
      {
        key.push_back(*segIter);
      }
      keyVector.push_back(key);
    }
    else
    {
      LOG4CXX_DEBUG(logger,
                    "COS not found for " << i->boardMultiCity << "-" << i->offMultiCity
                                         << " OND pair.");
    }
  }
  return keyVector;
}
// Remove all invalidated child itins from mother
void
FamilyLogicSplitter::cleanInvalidatedItins(Itin* mother, std::vector<Itin*>& invalidatedItinVec)
{
  LOG4CXX_DEBUG(logger, "Clean invalid itins");
  for (Itin* invalidated : invalidatedItinVec)
    mother->eraseSimilarItin(invalidated);
}

// Remove all invalidated child itins from mother
void
FamilyLogicSplitter::cleanInvalidatedItins(Itin* mother,
                                           std::vector<SimilarItinData>& invalidatedItinVec)
{
  LOG4CXX_DEBUG(logger, "Clean invalid itins");
  for (const auto& invalidated : invalidatedItinVec)
    mother->eraseSimilarItin(invalidated.itin);
}
}
