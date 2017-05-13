//-----------------------------------------------------------------------------
//
//  File:        SoloCarnivalIAUtil.cpp
//
//  Created:     Sep 27, 2011
//
//  Authors:     Artur de Sousa Rocha
//               Bartosz Kolpanowicz
//
//  Description: Sub-itinerary groups generator for the SOL Carnival project.
//
//  Copyright Sabre 2011
//
//               The copyright to the computer program(s) herein
//               is the property of Sabre.
//               The program(s) may be used and/or copied only with
//               the written permission of Sabre or in accordance
//               with the terms and conditions stipulated in the
//               agreement/contract under which the program(s)
//               have been supplied.
//
//-----------------------------------------------------------------------------

#include "ItinAnalyzer/SoloCarnivalIAUtil.h"

#include "Common/ClassOfService.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TravelSegUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "ItinAnalyzer/SoloCarnivalFareFamiliesGrouper.h"

#include <algorithm>
#include <functional>
#include <sstream>
#include <utility>

#include <boost/tokenizer.hpp>

namespace tse
{
static Logger
logger("atseintl.ItinAnalyzer.SoloCarnivalIAUtil");

namespace
{

const IntIndex SUB_ITINS_START_ID = 10000;

/**
 * those functions are to be used instead of operator "less" for std::max_element,
 * thats why we ignore special values like INVALID_INT_INDEX
 */

bool
lessByItinNum(const Itin* lhs, const Itin* rhs)
{
  return lhs->itinNum() < rhs->itinNum();
}

/*****************************************************/

} // anon namespace

// --- SOLItinInfo class functions ---

/* Function: Checks whether the itinerary is online/interline by generating the set of its carriers,
 *           counts its legs, the number of segments per each leg and classifies it as OW, RT or MD.
 */
void
SOLItinInfo::fill(Itin* itin, size_t limit)
{
  // Clear contents on each new call.
  itinPtr() = nullptr;
  itinType() = 0;
  carriers().clear();
  legToNumSegs().clear();

  if (!itin)
    return;

  itinPtr() = itin;

  for (const auto tSegPtr : itinPtr()->travelSeg())
  {
    if (!tSegPtr)
      continue;

    // Used by SoloCarnivalIAUtil::fillGapsInPath() function.
    // Calculates the number of all segments (incl. arunk) for each leg.
    if (0 <= tSegPtr->legId())
      ++legToNumSegs()[tSegPtr->legId()];

    // Skipping arunks. An arunk is not considered a carrier change.
    // Assumption: A valid air segment always has a non-empty carrier code.
    if (tSegPtr->isAir())
    {
      // The number of carriers must be known in advance (online/interline) for the
      // SOLO Carnival algorithm to decide which sub-itinerary groups to generate.
      // Must cast to get to the carrier() member.
      carriers().insert(dynamic_cast<AirSeg*>(tSegPtr)->carrier());
    }
  }

  // 0 legs - shouldn't happen, OW = 1 leg, RT = 2 legs, MD = 3 or more legs (at present).
  itinType() = std::min(legToNumSegs().size(), limit);
}

/* Function: Returns information on a segment's placement on its leg. */
size_t
SOLItinInfo::segType(int16_t legId, size_t segId) const
{
  if (isOneSegLeg(legId))
    return 0; // whole leg segment (one segment per leg)
  else if (isSegFirstOnLeg(segId))
    return 1; // first segment on leg
  else if (isSegLastOnLeg(legId, segId))
    return 2; // last segment on leg
  else
    return 3; // none of the above (internal segment)
}

// --- SoloCarnivalIAUtil class functions ---

SoloCarnivalIAUtil::SoloCarnivalIAUtil(PricingTrx& trx)
  : _trx(trx),
    _subitinGroups(nullptr),
    _generationOptions(parseGenerationOptions()),
    _prevMaxItinNum(INVALID_INT_INDEX)
{
}

/* Function: Adds a new itinerary to the transaction if the supplied segment sequence (possibly
 *           multi-leg) is unique.
 */
void
SoloCarnivalIAUtil::addNewItin(SOLItinGroups::ItinGroup& itinGroup,
                               std::list<SOLSegInfo>& newPath,
                               bool removeExternalArunks)
{
  if (newPath.empty())
    return;

  if (!newPath.front().segPtr())
    return;

  std::vector<TravelSeg*> tSegs;
  tSegs.push_back(
      newPath.front().segPtr()); // First pointer, the rest collected in the below 'for'.

  std::list<SOLSegInfo>::iterator prevNewPathIt = newPath.begin();
  std::list<SOLSegInfo>::iterator newPathIt = newPath.begin();
  std::list<SOLSegInfo>::iterator newPathEndIt = newPath.end();

  // Check the segment sequence for continuity (in fillGapsInPath(), which prepares segment
  // sequences on behalf of group 3 and 5, segments are silently dropped on errors).
  // Also, extract pointers into a vector of TravelSeg* for use below.
  for (++newPathIt; newPathIt != newPathEndIt; ++newPathIt, ++prevNewPathIt)
  {
    if (!newPathIt->segPtr())
      return;

    if (SOLSegInfo::segEnd(prevNewPathIt->segPtr()) != SOLSegInfo::segBegin(newPathIt->segPtr()))
      return;

    tSegs.push_back(newPathIt->segPtr());
  }

  // Group 4 only.
  if (removeExternalArunks)
    removeExternalArunkSegs(tSegs);

  if (tSegs.empty())
    return;

  // If the supplied sequence of travel segments is unique, a new itinerary based on that sequence
  // is generated and added to both: the relevant sub-itinerary group and the transaction's
  // itinerary vector. If not, the original itinerary pointer from the transaction's itinerary
  // vector is added to the sub-itinerary group.
  // 'Unique' means that no other original base itinerary (from the transaction's itinerary vector)
  // nor any of those that have been generated and added by this function by the time the algorithm
  // gets here, has an identical sequence of segments.

  Itin* newItinPtr = static_cast<Itin*>(nullptr);
  std::vector<int16_t> newItinHash = getItinHash(tSegs);

  // If the insertion is successful (with Itin* == 0 for now)...
  if ((itinHashes().insert(std::make_pair(newItinHash, static_cast<Itin*>(nullptr)))).second)
  {
    newItinPtr = createSubitin(tSegs);

    if (!newItinPtr)
      return;

    // ...update the map (now with Itin* != 0) and the transaction.
    itinHashes()[newItinHash] = newItinPtr;
    subitins().push_back(newItinPtr);
  }
  // If an identical element already exists, get its pointer instead.
  else
  {
    newItinPtr = itinHashes()[newItinHash];
  }

  // Insert either the new itinerary's pointer or - in the case of a duplicate - the existing/
  // /original itinerary's pointer from the transaction's itinerary vector (trx().itin()).
  itinGroup.push_back(newItinPtr);
}

/* Group 2:  (Interline) Itinerary per carrier and leg change.
 * Function: Generates a new sub-itinerary when:
 *           a) carrier changes,
 *           b) leg changes,
 *           c) arunk segment is found.
 *           In group 2, at any position, arunk segments are removed from the segment sequence.
 */
void
SoloCarnivalIAUtil::createInterlineByCxrAndLeg()
{
  // Currently processed itinerary.
  if (!itinInfo().itinPtr())
    return;

  if (!subitinGroups())
    return;

  if (!itinGroupVec()[SOLItinGroups::INTERLINE_BY_CXR_AND_LEG])
    return;

  Itin& itin = *itinInfo().itinPtr();
  SOLItinGroups::ItinGroup& itinGroup = *itinGroupVec()[SOLItinGroups::INTERLINE_BY_CXR_AND_LEG];

  // Check generation options.
  if (!shouldItinBeProcessed(SOLItinGroups::INTERLINE_BY_CXR_AND_LEG))
    return;

  // New itinerary's segment sequence accumulator.
  std::list<SOLSegInfo> newPath;

  // Source itinerary segments' iterators.
  std::vector<TravelSeg*>::iterator tSegIt = itin.travelSeg().begin();
  std::vector<TravelSeg*>::iterator tSegEndIt = itin.travelSeg().end();

  const int16_t INVALID_LEG_ID = -2;
  int16_t prevLegId = INVALID_LEG_ID;

  const std::string INVALID_CXR_ID = "";
  std::string prevCxrId = INVALID_CXR_ID;

  for (; tSegIt != tSegEndIt; ++tSegIt)
  {
    if (!*tSegIt)
      continue;

    // Arunk segment. Always dropped.
    if (!(*tSegIt)->isAir())
    {
      addNewItin(itinGroup, newPath);
      newPath.clear();
      prevLegId = INVALID_LEG_ID;
      prevCxrId = INVALID_CXR_ID;
    }
    // Air segment.
    // Assumption: A valid air segment always has a non-empty carrier code (cxr != "") and a leg ID
    // set (leg >= 0).
    else
    {
      // Must cast because the TravelSeg class has no member named carrier().
      AirSeg* aSegPtr = dynamic_cast<AirSeg*>(*tSegIt);

      if (INVALID_LEG_ID == prevLegId)
        prevLegId = aSegPtr->legId();
      if (INVALID_CXR_ID == prevCxrId)
        prevCxrId = aSegPtr->carrier();

      // Nothing changes or start of a new sequence - just add new segments.
      if (aSegPtr->legId() == prevLegId && aSegPtr->carrier() == prevCxrId)
      {
        // In group 2 segment position information is not necessary, using 0.
        newPath.push_back(SOLSegInfo(aSegPtr, 0));
      }
      // Change. Try to generate a new itinerary from accumulated segments.
      else
      {
        addNewItin(itinGroup, newPath);
        newPath.clear();
        newPath.push_back(SOLSegInfo(aSegPtr, 0));
        prevLegId = aSegPtr->legId();
        prevCxrId = aSegPtr->carrier();
      } // else
    } // else
  } // for(; tSegIt != tSegEndIt; ++tSegIt)

  // Anything left for processing?
  addNewItin(itinGroup, newPath);
}

/* Group 3:  (Interline) Itinerary per carrier change.
 * Function: Generates new sub-itineraries by grouping those segments of the original itinerary that
 *           have the same carrier ID but - simultaneously - may have different leg IDs (no breaks
 *           at leg changes).
 *           All gaps are filled with arunk segments (provided that their addition will not make the
 *           itinerary start nor end with an arunk segment) so the resulting segment sequence forms
 *           a legitimate OW/RT/CT itinerary. If the algorithm needs a gap-filling-arunk-segment
 *           that - if generated - would be identical to one of the original arunk segments from the
 *           input itinerary, then the original arunk segment is used instead. This is beneficial
 *           because original arunk segments have their leg IDs correctly set which may not always
 *           be the case with generated arunk segments (Although leg IDs are correctly set in most
 *           cases there are situations when some heuristics must be applied).
 *           Example when heuristics would give incorrect results (generated arunk would be assigned
 *           the leg ID of the upper leg):
 *           (A -aa-> B -aa-> C -cl-> D -cl-> E || E -cl-> C -arunk-> F -aa-> G -aa-> A)
 */
void
SoloCarnivalIAUtil::createInterlineByCxrGrouping()
{
  // Currently processed itinerary.
  if (!itinInfo().itinPtr())
    return;

  if (!subitinGroups())
    return;

  if (!itinGroupVec()[SOLItinGroups::INTERLINE_BY_CXR_GROUPING])
    return;

  Itin& itin = *itinInfo().itinPtr();
  SOLItinGroups::ItinGroup& itinGroup = *itinGroupVec()[SOLItinGroups::INTERLINE_BY_CXR_GROUPING];

  // Check generation options.
  if (!shouldItinBeProcessed(SOLItinGroups::INTERLINE_BY_CXR_GROUPING))
    return;

  // Lists of segments serviced by individual carriers.
  std::map<CarrierCode, std::list<SOLSegInfo> > carrierPaths;

  // Source itinerary segments' iterators.
  std::vector<TravelSeg*>::iterator tSegIt = itin.travelSeg().begin();
  std::vector<TravelSeg*>::iterator tSegEndIt = itin.travelSeg().end();

  const int16_t INVALID_ID = -1; // Do not change!
  int16_t prevLegId = INVALID_ID; // Any value less than 0 OK.
  int16_t curLegId = INVALID_ID; // Any value OK.
  int16_t curLegSegId = INVALID_ID; // Do not change! Ordinal of a segment on its leg.

  // Assign segments to all carrier paths (in one pass).
  for (; tSegIt != tSegEndIt; ++tSegIt)
  {
    if (!*tSegIt)
      continue;

    // Segments-per-current-leg count.
    curLegId = (*tSegIt)->legId();

    if (0 > curLegId)
      continue;

    if (prevLegId != curLegId)
    {
      prevLegId = curLegId;
      curLegSegId = INVALID_ID;
    }

    ++curLegSegId;

    // Air segment - add to the path of the appropriate carrier. Skip arunk segments.
    if ((*tSegIt)->isAir())
    {
      // Must cast because the TravelSeg class has no member named carrier().
      AirSeg* aSegPtr = dynamic_cast<AirSeg*>(*tSegIt);
      carrierPaths[aSegPtr->carrier()].push_back(SOLSegInfo(aSegPtr, curLegSegId));
    }
  } // for(; tSegIt != tSegEndIt; ++tSegIt)

  std::map<CarrierCode, std::list<SOLSegInfo> >::iterator carrierPathsIt = carrierPaths.begin();
  std::map<CarrierCode, std::list<SOLSegInfo> >::iterator carrierPathsEndIt = carrierPaths.end();

  // Process each carrier's path.
  for (; carrierPathsIt != carrierPathsEndIt; ++carrierPathsIt)
  {
    // Fill the gaps (if needed) and add the new itinerary.
    fillGapsInPath(carrierPathsIt->second);
    addNewItin(itinGroup, carrierPathsIt->second);
  } // for
}

/* Group 4:  (Online) Itinerary per leg change.
 * Function: Generates new sub-itineraries by dividing the source itinerary at leg changes.
 *           Arunk segments at the beginning and end of a new sub-itinerary are removed. Ones in the
 *           middle are preserved.
 */
void
SoloCarnivalIAUtil::createOnlineByLeg()
{
  // Currently processed itinerary.
  if (!itinInfo().itinPtr())
    return;

  if (!subitinGroups())
    return;

  if (!itinGroupVec()[SOLItinGroups::ONLINE_BY_LEG])
    return;

  Itin& itin = *itinInfo().itinPtr();
  SOLItinGroups::ItinGroup& itinGroup = *itinGroupVec()[SOLItinGroups::ONLINE_BY_LEG];

  // Check generation options.
  if (!shouldItinBeProcessed(SOLItinGroups::ONLINE_BY_LEG))
    return;

  // New itinerary's segment sequence accumulator.
  std::list<SOLSegInfo> newPath;

  // Source itinerary segments' iterators.
  std::vector<TravelSeg*>::iterator tSegIt = itin.travelSeg().begin();
  std::vector<TravelSeg*>::iterator tSegEndIt = itin.travelSeg().end();

  const int16_t INVALID_ID = -1;
  int16_t prevLegId = INVALID_ID;
  int16_t curLegId = INVALID_ID;

  for (; tSegIt != tSegEndIt; ++tSegIt)
  {
    if (!*tSegIt)
      continue;

    curLegId = (*tSegIt)->legId();

    if (0 > curLegId)
      continue;

    // Usually. Continue current leg.
    if (curLegId == prevLegId)
    {
      // In group 4 segment position information is not necessary, using 0.
      newPath.push_back(SOLSegInfo(*tSegIt, 0));
    }
    // First segment.
    else if (INVALID_ID == prevLegId)
    {
      newPath.push_back(SOLSegInfo(*tSegIt, 0));
      prevLegId = curLegId;
    }
    // New leg detected.
    else
    {
      // In group 4 remove external arunk segments ('true').
      addNewItin(itinGroup, newPath, true);
      newPath.clear();
      newPath.push_back(SOLSegInfo(*tSegIt, 0));
      prevLegId = curLegId;
    }
  } // for

  // Anything left for processing?
  addNewItin(itinGroup, newPath, true);
}

/* Group 5:  (Online) Itinerary per country change.
 * Function: Generates new sub-itineraries by grouping those segments of the original itinerary that
 *           begin and end in the same country (domestic; one set per country) and those that cross
 *           borders (international; one set for all countries).
 *           All arunks are removed. Then all gaps are filled as in group 3.
 */
void
SoloCarnivalIAUtil::createOnlineByDomIntGrouping()
{
  // Currently processed itinerary.
  if (!itinInfo().itinPtr())
    return;

  if (!subitinGroups())
    return;

  if (!itinGroupVec()[SOLItinGroups::ONLINE_BY_DOM_INT_GROUPING])
    return;

  Itin& itin = *itinInfo().itinPtr();
  SOLItinGroups::ItinGroup& itinGroup = *itinGroupVec()[SOLItinGroups::ONLINE_BY_DOM_INT_GROUPING];

  // Check generation options.
  if (!shouldItinBeProcessed(SOLItinGroups::ONLINE_BY_DOM_INT_GROUPING))
    return;

  // Lists of segments flown within individual country (domestic) + one list of segments that
  // cross borders (international).
  std::map<NationCode, std::list<SOLSegInfo> > countryPaths;

  // Source itinerary segments' iterators.
  std::vector<TravelSeg*>::iterator tSegIt = itin.travelSeg().begin();
  std::vector<TravelSeg*>::iterator tSegEndIt = itin.travelSeg().end();

  const int16_t INVALID_ID = -1; // Do not change!
  int16_t prevLegId = INVALID_ID; // Any value less than 0 OK.
  int16_t curLegId = INVALID_ID; // Any value OK.
  int16_t curLegSegId = INVALID_ID; // Do not change! Ordinal of a segment on its leg.

  // Assign segments to domestic and international paths.
  for (; tSegIt != tSegEndIt; ++tSegIt)
  {
    if (!*tSegIt)
      continue;

    // Segments-per-current-leg count.
    curLegId = (*tSegIt)->legId();

    if (0 > curLegId)
      continue;

    if (prevLegId != curLegId)
    {
      prevLegId = curLegId;
      curLegSegId = INVALID_ID;
    }

    ++curLegSegId;

    // Air segment - add to the domestic path of the appropriate country or to the international
    // path.
    // Skip arunk segments.
    if ((*tSegIt)->isAir())
    {
      NationCode nationCode = NATION_INTERNATIONAL;

      // Segments joining UNITED_STATES and CANADA ("US" and "CA"), called 'transborder', are
      // considered to be within one country in Carnival SOL sub-itinerary group 5 and will be
      // assigned to UNITED_STATES path. Consequently, segments within CANADA will also be assigned
      // to that path.
      // All other domestic (foreign domestic) segments will be assigned to their respective country
      // paths.
      // All international segments will be assigned to (one) NATION_INTERNATIONAL ("INTN") path.
      if (!(*tSegIt)->origin() || !(*tSegIt)->destination())
        continue;

      if (!LocUtil::isInternational(*(*tSegIt)->origin(), *(*tSegIt)->destination()))
      {
        // Transborder (US <-> CA or CA <-> US) or domestic (US<->US or CA<->CA). Assign to
        // UNITED_STATES.
        if (LocUtil::isTransBorder(*(*tSegIt)->origin(), *(*tSegIt)->destination()) ||
            LocUtil::isDomestic(*(*tSegIt)->origin(), *(*tSegIt)->destination()))
        {
          nationCode = UNITED_STATES;
        }
        // Foreign domestic. Assign to the particular country path.
        else
        {
          nationCode = (*tSegIt)->origin()->nation();
        }
      }

      countryPaths[nationCode].push_back(SOLSegInfo(*tSegIt, curLegSegId));
    } // if((*tSegIt)->isAir())
  } // for(; tSegIt != tSegEndIt; ++tSegIt)

  std::map<NationCode, std::list<SOLSegInfo> >::iterator countryPathsIt = countryPaths.begin();
  std::map<NationCode, std::list<SOLSegInfo> >::iterator countryPathsEndIt = countryPaths.end();

  // Process each country path and the international path.
  for (; countryPathsIt != countryPathsEndIt; ++countryPathsIt)
  {
    // Fill the gaps (if needed) and add the new itinerary.
    fillGapsInPath(countryPathsIt->second);
    addNewItin(itinGroup, countryPathsIt->second);
  } // for
}

/* Function: Creates a new sub-itinerary based on supplied travel segments and the original source
 *           itinerary. Called by addNewItin().
 */
Itin*
SoloCarnivalIAUtil::createSubitin(const std::vector<TravelSeg*>& tSegs)
{
  if (tSegs.empty())
    return nullptr;

  if (!tSegs.front())
    return nullptr;

  if (!itinInfo().itinPtr())
    return nullptr;

  Itin* newItinPtr = nullptr;
  trx().dataHandle().get(newItinPtr);

  if (!newItinPtr)
    return nullptr;

  newItinPtr->duplicate(*itinInfo().itinPtr(), trx().dataHandle());
  newItinPtr->travelSeg() = tSegs;
  newItinPtr->setTravelDate(tSegs.front()->departureDT());
  newItinPtr->clearSimilarItins();
  newItinPtr->itinNum() = incItinNum();
  newItinPtr->setItinFamily(itinInfo().itinPtr()->getItinFamily());
  PropagateOrigBooking(*itinInfo().itinPtr(), trx().dataHandle()).to(*newItinPtr);

  return newItinPtr;
}

/* Function: Creates a SOLItinGroups object and the vectors that pointers in its _itinGroups member
 *           vector point at.
 */
void
SoloCarnivalIAUtil::createSubitinGroups()
{
  subitinGroups() = nullptr;
  trx().dataHandle().get(subitinGroups());

  if (subitinGroups())
  { // (size set in constructor, elements zeroed)
    for (size_t i = 0; i < subitinGroups()->itinGroups().size(); ++i)
    {
      trx().dataHandle().get(subitinGroups()->itinGroups()[i]);

      if (!subitinGroups()->itinGroups()[i])
      {
        subitinGroups() = nullptr;
        break;
      }
    } // for
  } // if(subitinGroups())
}

class CompareByTravelOrder
{
  const Itin* _original;

public:
  CompareByTravelOrder(const Itin* original) : _original(original) {}

  bool operator()(const Itin* lhs, const Itin* rhs) const
  {
    TSE_ASSERT(!lhs->travelSeg().empty());
    TSE_ASSERT(!rhs->travelSeg().empty());

    return _original->segmentOrder(lhs->travelSeg().front()) <
           _original->segmentOrder(rhs->travelSeg().front());
  }
};

void
SoloCarnivalIAUtil::sortGroupInTravelOrder(SOLItinGroups::ItinGroup& group, const Itin* original)
{
  std::sort(group.begin(), group.end(), CompareByTravelOrder(original));
}

void
SoloCarnivalIAUtil::sortSubitinGroups()
{
  for (SOLItinGroups::ItinGroup* group : itinGroupVec())
  {
    TSE_ASSERT(group);
    sortGroupInTravelOrder(*group, itinInfo().itinPtr());
  }
}

/* Function: Fills gaps in a segment sequence by using existing or generated arunk segments.
 *           Gap is a missing part of the itinerary, between one air segment's end and the next air
 *           segment's beginning, where both points are different. When a new arunk segment must be
 *           generated, the function uses a partially heuristic (and rather simple!) algorithm for
 *           finding the correct leg ID for it (may be not always 100% optimal).
 */
void
SoloCarnivalIAUtil::fillGapsInPath(std::list<SOLSegInfo>& pathInfo)
{
  /* Segment types (t0, t1,...):
   *    0 - first and last on leg (whole leg segment),
   *    1 - first on leg but not last,
   *    2 - last on leg but not first,
   *    3 - none of the above.
   *
   * States (s0, s1,...):
   *  <80 - jump to that state,
   *   80 - exit and use left leg number,
   *   81 - exit and use right leg number,
   *   82 - exit and calculate relative position of both segments' ends.
   *
   * Only 2 jumps needed: first for the left segment type, second for the right one.
   *
   * Processing goes from top to bottom. The first combination found is followed:
   *   both are whole leg segments              -> extend left leg,
   *   one is whole leg segment                 -> extend opposite leg,
   *   both are beginning segments              -> extend left leg,
   *   both are ending segments                 -> extend right leg,
   *   left is ending segment, right is none    -> extend right leg,
   *   right is beginning segment, left is none -> extend left leg,
   *   none of the above                        -> calculate relative position of segments' ends and
   *                                               extend "shorter" leg.
   */
  const size_t fillGapsFstDim = 5;
  const size_t fillGapsSndDim = 4;

  static const size_t fillGapsSM[fillGapsFstDim][fillGapsSndDim] = { /*        t0  t1  t2  t3 */
                                                                     /* s0 */ { 1, 2, 3, 4 },
                                                                     /* s1 */ { 80, 81, 81, 81 },
                                                                     /* s2 */ { 80, 80, 82, 82 },
                                                                     /* s3 */ { 80, 80, 81, 81 },
                                                                     /* s4 */ { 80, 80, 82, 82 } };

  if (2 > pathInfo.size())
    return;

  if (!itinInfo().itinPtr())
    return;

  std::vector<TravelSeg*>& itinSegs = itinInfo().itinPtr()->travelSeg();
  std::list<SOLSegInfo>::iterator pathIt = pathInfo.begin();
  std::list<SOLSegInfo>::iterator pathEndIt = pathInfo.end();

  for (++pathIt; pathIt != pathEndIt; ++pathIt)
  {
    // Can't move in parallel because new segments are added to the list.
    std::list<SOLSegInfo>::iterator prevPathIt = pathIt;
    --prevPathIt;

    // ArunkSeg's constructor fills the originalId member with a value specific for arunk segments.
    ArunkSeg* arSeg = nullptr;
    int16_t arSegLegId = -1;

    if (!prevPathIt->segPtr() || !pathIt->segPtr())
      continue;

    if (pathIt->gapOnTheLeft(*prevPathIt))
    {
      // Try to find an appropriate segment - preferably an arunk one - among the original segments
      // of the itinerary.
      std::vector<TravelSeg*>::iterator segsIt = itinSegs.begin();
      std::vector<TravelSeg*>::iterator segsEndIt = itinSegs.end();

      // Check if any of the original itinerary segments may be used as the needed arunk segment or
      // at least be the source of the leg ID.
      for (; segsIt != segsEndIt; ++segsIt)
      {
        if (!*segsIt)
          continue;

        // Find first that fits. Leg ID is important. In the below example, a new segment must be
        // generated, despite the presence of an ends-matching-segment:
        // Leg 1: A -> B -> C -> D
        // Leg 2: D -> B -> E -> F
        // Leg 3: F -> G -> E -> A
        // Suppose B -> C -> D -> B and E -> F -> G -> E are removed from the original path.
        if (SOLSegInfo::segBegin(*segsIt) == prevPathIt->segEnd() &&
            SOLSegInfo::segEnd(*segsIt) == pathIt->segBegin() &&
            (SOLSegInfo::segLegId(*segsIt) == prevPathIt->segLegId() ||
             SOLSegInfo::segLegId(*segsIt) == pathIt->segLegId()))
        {
          // Even if "not air segment" doesn't mean "arunk segment", dynamic cast will return 0 if
          // destination pointer does not fit.
          if ((*segsIt)->isAir())
            arSegLegId = SOLSegInfo::segLegId(*segsIt);
          else
            arSeg = dynamic_cast<ArunkSeg*>(*segsIt);

          break;
        }
      } // for

      // Create a new arunk segment object if none of the existing ones fits.
      if (!arSeg)
      {
        // Copy pointers to the origin and destination structures from adjacent (with a gap between)
        // segments.
        // _originalId set in constructor,
        // _legId set below.
        arSeg =
            TravelSegUtil::buildArunk(trx().dataHandle(), prevPathIt->segPtr(), pathIt->segPtr());

        if (!arSeg)
          continue;

        // Best bet: New segment between two air segments on the same leg.
        if (prevPathIt->segLegId() == pathIt->segLegId())
        {
          arSeg->legId() = pathIt->segLegId();
        }
        // Second best bet: Use leg ID from another air segment.
        else if (0 <= arSegLegId)
        {
          arSeg->legId() = arSegLegId;
        }
        // Otherwise: Calculate leg ID.
        else
        {
          bool useRightLegId = false;
          float maxLeftReach = 0.0;
          float maxRightReach = 0.0;
          std::map<int16_t, std::size_t>::const_iterator legToNumSegsPrevIt;
          std::map<int16_t, std::size_t>::const_iterator legToNumSegsCurIt;
          std::size_t numSegsPrev = 0;
          std::size_t numSegsCur = 0;

          // Values for the "left" segment.
          size_t state = 0;
          size_t segType = itinInfo().segType(prevPathIt->segLegId(), prevPathIt->segSegId());

          for (int i = 0; i < 2; ++i)
          {
            if (fillGapsFstDim <= state || fillGapsSndDim <= segType)
              break; // -> switch -> default -> continue.

            // Values for the "right" segment.
            state = fillGapsSM[state][segType];
            segType = itinInfo().segType(pathIt->segLegId(), pathIt->segSegId());
          }

          switch (state)
          {
          case 80:
            useRightLegId = false;
            break;

          case 81:
            useRightLegId = true;
            break;

          case 82: // Estimate how far the segments surrounding the gap reach on their
            // respective legs.
            useRightLegId = false;

            legToNumSegsPrevIt = itinInfo().legToNumSegs().find(prevPathIt->segLegId());
            legToNumSegsCurIt = itinInfo().legToNumSegs().find(pathIt->segLegId());

            if ((itinInfo().legToNumSegs().end() == legToNumSegsPrevIt) ||
                (itinInfo().legToNumSegs().end() == legToNumSegsCurIt))
              break;

            numSegsPrev = legToNumSegsPrevIt->second;
            numSegsCur = legToNumSegsCurIt->second;

            if (!numSegsPrev || !numSegsCur)
              break;

            maxLeftReach = float(prevPathIt->segSegId() + 1) / numSegsPrev;
            maxRightReach = 1.0 - (float(pathIt->segSegId()) / numSegsCur);

            if (maxLeftReach <= maxRightReach)
              useRightLegId = false; // If equal, extend source (left) leg.
            else
              useRightLegId = true;

            break;

          default:
            continue;
          }

          if (useRightLegId)
            arSeg->legId() = pathIt->segLegId();
          else
            arSeg->legId() = prevPathIt->segLegId();
        } // else <- if(0 <= arSegLegId) <- if(prevPathIt->segLegId() == pathIt->segLegId())
      } // if(!arSeg)

      pathInfo.insert(pathIt, SOLSegInfo(arSeg, (size_t) - 1)); // The arunk's segment ID (-1) will
    } // if(pathIt->gapOnTheLeft())                           // not be used in further processing.
  } // for(; pathIt != pathEndIt; ++pathIt)
}

/* Function: Resizes the _generationOptions vector and fills it with values parsed from the ACMS's
 *           entry SOLO_CARNIVAL_OPT\GROUP_GENERATION_CONFIG.
 */
std::vector<std::bitset<SoloCarnivalIAUtil::_genOptNumFields> >
SoloCarnivalIAUtil::parseGenerationOptions() const
{
  const int digitsStart = 0x30; // Character '0' (zero).

  // Attention! The SOLO Carnival project defines sub-itinerary groups as groups 1 through 5.
  //            ACMS also uses this numbering in its SOLO_CARNIVAL_OPT\GROUP_GENERATION_CONFIG
  // entry.
  //            However the internal representation of groups' numbers is 0 through 4, see:
  //            SOLItinGroups.h.

  std::vector<std::bitset<_genOptNumFields> > generationOptions;
  generationOptions.resize(SOLItinGroups::GROUP_TYPE_MAX); // Number of groups.

  // Although all options for group 1 (index 0!) are set here by default, none of them is actually
  // checked at its generation, as group 1 is always generated. Compare diagnostic 198.
  generationOptions.at(0).set();

  typedef boost::char_separator<char> separatorType;
  typedef boost::tokenizer<separatorType> tokenizerType;

  separatorType separator("|");
  tokenizerType tokens(trx().getOptions()->getSolGroupGenerationConfig(), separator);

  tokenizerType::iterator iter = tokens.begin();
  tokenizerType::iterator endIter = tokens.end();

  // Break the configuration string into tokens on the '|' character.
  for (; iter != endIter; ++iter)
  {
    // Options for one group.
    std::string optionsSet = *iter;

    // Check the string's length (1 char for group number + number of options to set for one
    // group)...
    if (optionsSet.length() != 1 + genOptNumFields())
      continue;

    // ...and the range of its first character (group number). Group 1 is always generated.
    if (optionsSet.at(0) < '2' ||
        optionsSet.at(0) > (digitsStart + SOLItinGroups::GROUP_TYPE_MAX)) // for now: '5'
    {
      std::stringstream ss;

      ss << "SoloCarnivalIAUtil::parseGenerationOptions() - "
         << "Ignored options for invalid group number " << optionsSet.at(0)
         << ". Valid numbers are 2.." << char(digitsStart + SOLItinGroups::GROUP_TYPE_MAX);

      LOG4CXX_ERROR(logger, ss.str());
      continue;
    }

    size_t idx = optionsSet.at(0) - digitsStart - 1;

    // Set options for individual groups (starting with group 2).
    // Leave record for group 1 (index 0) as is (all options set).
    // Options to set:
    //   generate group (as a whole), generate for OWs, generate for RTs, generate for MDs.
    // Last three (OW, RT, MD) are generated only if 'generate group' (group ON/OFF) is set.
    for (size_t i = 0; i < genOptNumFields(); ++i)
    {
      // Options begin at the second character.
      if ('Y' == optionsSet.at(i + 1) || 'T' == optionsSet.at(i + 1))
        generationOptions.at(idx).set(i);
    }
  }

  return generationOptions;
}

/* Function: Fills the itinHashes map with hashes of itineraries from transaction. */
void
SoloCarnivalIAUtil::fillItinHashes()
{
  itinHashes().clear();

  for (Itin* itin : trx().itin())
  {
    itinHashes().insert(std::make_pair(getItinHash(itin->travelSeg()), itin));
    for (const SimilarItinData& similarItinData : itin->getSimilarItins())
    {
      itinHashes().insert(
          std::make_pair(getItinHash(similarItinData.itin->travelSeg()), similarItinData.itin));
    }
  }
}

IntIndex
SoloCarnivalIAUtil::findMaxItinNum() const
{
  if (_prevMaxItinNum != INVALID_INT_INDEX)
    return _prevMaxItinNum;

  if (trx().itin().empty())
    return INVALID_INT_INDEX;

  return (**std::max_element(trx().itin().begin(), trx().itin().end(), lessByItinNum)).itinNum();
}

/* Function: Processes each source itinerary from the transaction by, inter alia, calling the
 *           sub-itinerary groups generator function processInputItin().
 */
void
SoloCarnivalIAUtil::generateSubitins(PricingTrx& trx)
{
  SoloCarnivalIAUtil ctx(trx);

  // Hashes of sequences-of-travel-segments. Such sequences are candidates
  // for becoming a new itinerary, provided that they are unique.
  // Duplicates are identified before generating a new itinerary.
  // Pre-fill the set with hashes of existing source itineraries.
  ctx.fillItinHashes();

  // Generate groups of sub-itineraries for each source itinerary.
  for (Itin* itin : trx.itin())
  {
    ctx.processInputItin(itin);
    for (const SimilarItinData& similarItinData : itin->getSimilarItins())
      ctx.processInputItin(similarItinData.itin);
  }

  // group subitins into fare families
  SoloCarnivalFareFamiliesGrouper::group(ctx.subitins());

  // add subitins to trx
  trx.itin().insert(trx.itin().end(), ctx.subitins().begin(), ctx.subitins().end());
}

/* Function: Creates a hash code for an itinerary, by joining its travel segments' originalIds in
 *           a vector.
 */
std::vector<int16_t>
SoloCarnivalIAUtil::getItinHash(std::vector<TravelSeg*>& tSegs)
{
  std::vector<int16_t> hash;

  for (const auto tSeg : tSegs)
  {
    if (tSeg)
      hash.push_back(tSeg->originalId());
  }

  return hash;
}

IntIndex
SoloCarnivalIAUtil::incItinNum()
{
  IntIndex maxItinNum = findMaxItinNum();
  IntIndex nextItinNum = (INVALID_INT_INDEX == maxItinNum) ? 1 // start from "1"
                                                           : maxItinNum + 1;

  nextItinNum = std::max(nextItinNum, IntIndex(SUB_ITINS_START_ID));

  return (_prevMaxItinNum = nextItinNum);
}

/* Function: Generates sub-itinerary groups based on source itinerary type (online/interline). */
void
SoloCarnivalIAUtil::processInputItin(Itin* itin)
{
  if (!itin)
    return;

  // Create sub-itinerary groups object. Clears pointer first.
  createSubitinGroups();

  // Not logging 'out of memory'.
  if (!subitinGroups())
    return;

  // Fill group 1 - just copy the original source itinerary (pointer).
  subitinGroups()->itinGroups()[SOLItinGroups::ORIGINAL]->push_back(itin);

  // Assign sub-itinerary groups object to the original itinerary.
  if (!(trx().solItinGroupsMap().insert(std::make_pair(itin, subitinGroups()))).second)
    return;

  // Get info on the currently processed itinerary: i.a. its carriers (online?) and legs.
  // Clears contents first.
  itinInfo().fill(itin, genOptNumFields() - 1);

  // Interline: generate groups 1, 2 and 3.
  // Online:    generate groups 1, 4 and 5.
  if (1 < itinInfo().carriers().size())
  {
    /*2*/ createInterlineByCxrAndLeg();
    /*3*/ createInterlineByCxrGrouping();
  }
  else
  {
    /*4*/ createOnlineByLeg();
    /*5*/ createOnlineByDomIntGrouping();
  }

  sortSubitinGroups();
}

/* Function: Removes arunk segments at ends of the sequence of travel segment.
 *           Used only by group 4.
 */
void
SoloCarnivalIAUtil::removeExternalArunkSegs(std::vector<TravelSeg*>& tSegs)
{
  while (!tSegs.empty())
  {
    if (!tSegs.front()->isAir())
    {
      tSegs.erase(tSegs.begin());
      continue;
    }

    if (!tSegs.back()->isAir())
    {
      tSegs.pop_back();
      continue;
    }

    break;
  } // while
}

/* Function: Checks whether or not to generate sub-itineraries for the current itinerary, based on
 *           the configuration options.
 *           Groups 2-5 indexed with values 1-4!
 *           Bitset element 0 - generate whole group, elements 1-3 generate OW, RT, MD.
 */
bool
SoloCarnivalIAUtil::shouldItinBeProcessed(SOLItinGroups::GroupType groupType) const
{
  // Test argument range, group generation ON/OFF and type of itinerary.
  return (groupType > 0) && // Group number valid? Group 1 always processed,
         (groupType < SOLItinGroups::GROUP_TYPE_MAX) && // its generation not consulted here.
         (generationOptions()[groupType].test(0)) && // Generate group?
         (itinInfo().itinType() > 0) && // Itin type valid? 1 for OW, 2 for RT, 3 for MD.
         (itinInfo().itinType() < genOptNumFields()) && //
         (generationOptions()[groupType].test(itinInfo().itinType())); // Generate OW/RT/MD?
}

// --- SoloCarnivalIAUtil::PropagateOrigBooking class functions ---

void
SoloCarnivalIAUtil::PropagateOrigBooking::to(Itin& subItin) const
{
  subItin.origBooking().resize(subItin.travelSeg().size(), nullptr);

  for (std::size_t subItinSegIndex = 0; subItinSegIndex < subItin.travelSeg().size();
       ++subItinSegIndex)
  {
    ClassOfService* bkk = lookupParentBKK(subItin.travelSeg().at(subItinSegIndex));
    if (bkk)
    {
      subItin.origBooking().at(subItinSegIndex) = bkk;
    }
    else
    {
      // use default values
      _dh.get(bkk);
      subItin.origBooking().at(subItinSegIndex) = bkk;
    }
  }
}

ClassOfService*
SoloCarnivalIAUtil::PropagateOrigBooking::lookupParentBKK(const TravelSeg* subItinSeg) const
{
  if (!subItinSeg)
    return nullptr;

  for (std::size_t parentItinSegIndex = 0; parentItinSegIndex < _parent.travelSeg().size();
       ++parentItinSegIndex)
  {
    TravelSeg* parentItinSeg = _parent.travelSeg().at(parentItinSegIndex);
    if (!parentItinSeg)
      continue;

    if (subItinSeg->originalId() == parentItinSeg->originalId())
    {
      if (parentItinSegIndex >= _parent.origBooking().size())
      {
        LOG4CXX_ERROR(logger, "parent misses booking code for a segment");
        return nullptr;
      }

      return _parent.origBooking().at(parentItinSegIndex);
    }
  }

  // most probably that was arunk segment and parent does not contain any of
  return nullptr;
}
}
