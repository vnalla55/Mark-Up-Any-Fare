//-------------------------------------------------------------------
//
//  File:        FareDisplayResponseRoutingXMLTags.cpp
//
//
//  Copyright Sabre 2006
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "Xform/FareDisplayResponseRoutingXMLTags.h"

#include "Common/FallbackUtil.h"
#include "Common/FareDisplayResponseUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/RestrictionsText.h"
#include "Common/RoutingUtil.h"
#include "Common/XMLConstruct.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/GlobalDir.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/Nation.h"
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingRestriction.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Routing/MileageInfo.h"
#include "Routing/RoutingInfo.h"

#include <sstream>
#include <string>

using namespace tse;
using namespace std;

namespace tse
{

FALLBACK_DECL(fallbackFMRbyte58);
}

namespace
{
// this value defines the Effective Date Indicator
static const char SLASH = '/';
static const char DASH = '-';
static const std::string DELIMS = "-/";
static const std::string SINGLE_BLANK = " ";
static const std::string TWO_BLANKS = "  ";
static const std::string THREE_BLANKS = "   ";
static const std::string FOUR_BLANKS = "    ";
static const std::string FIVE_BLANKS = "     ";
static const std::string SIX_BLANKS = "      ";
static const std::string INDEF = "INDEF";
static const std::string TOO_MANY_LINES = "MORE THAN 20 STRINGS - RD/LINE NUM/*RTG TO VIEW ROUTINGS";
static const uint16_t MAX_RTE_LEN_FQ = 53;
static const uint16_t MAX_RTE_LEN_RD = 58;
static const uint16_t MAX_FD_RTG_LINES = 20;
static const uint16_t MAX_LINE_LENGTH = 63;
static const uint16_t GLOB_DIR_MARGIN = 20;
static const uint16_t STRING_INDENT_LENGTH = 7;
static const std::string
STRING_INDENT(STRING_INDENT_LENGTH, ' ');
static const std::string US_CA = "US CA";
static const std::string AND = " AND ";
static const std::string OR = " OR ";
static const std::string ANDOR = " AND/OR ";
static const uint16_t AND_LEN = AND.size();
static const uint16_t OR_LEN = OR.size();
static const uint16_t ANDOR_LEN = ANDOR.size();

typedef std::vector<TpdPsrViaGeoLoc*>::const_iterator TpdPsrViaGeoLocConstIter;
typedef std::pair<TpdPsrViaGeoLocConstIter, TpdPsrViaGeoLocConstIter> TpdPsrViaGeoLocConstIterPair;
typedef std::vector<TpdPsr*> TpdPsrs;
typedef std::map<RoutingNumber, RoutingInfo*> RoutingInfoMap;
typedef RoutingInfoMap::const_iterator RoutingInfoMapConstIter;

bool operator<(const TpdPsrViaGeoLoc& l, const TpdPsrViaGeoLoc& r)
{
  if (l.relationalInd() != r.relationalInd())
    return l.relationalInd() < r.relationalInd();
  if (l.loc().locType() != r.loc().locType())
    return l.loc().locType() < r.loc().locType();
  return l.loc().loc() < r.loc().loc();
}

struct ViaGeoLocEqual
{
  bool operator()(const TpdPsrViaGeoLoc* l, const TpdPsrViaGeoLoc* r) const
  {
    return l->relationalInd() == r->relationalInd() && l->loc().locType() == r->loc().locType() &&
           l->loc().loc() == r->loc().loc();
  }
} viaGeoLocEqual;

struct PSRLessThan
{
  bool operator()(const TpdPsr* l, const TpdPsr* r) const
  {
    if (l == nullptr || r == nullptr)
      return false;
    bool swapped(false);
    if (l->viaGeoLocs().size() > r->viaGeoLocs().size())
    {
      swap(l, r);
      swapped = true;
    }
    TpdPsrViaGeoLocConstIter lb(l->viaGeoLocs().begin()), le(l->viaGeoLocs().end()),
        rb(r->viaGeoLocs().begin());
    TpdPsrViaGeoLocConstIterPair mis(mismatch(lb, le, rb, viaGeoLocEqual));
    if (mis.first == le)
    {
      if (mis.second == r->viaGeoLocs().end())
      {
        if (swapped)
          return l->carrier().empty() && !r->carrier().empty();
        else
          return !l->carrier().empty() && r->carrier().empty();
      }
      else
        return !swapped;
    }
    return **mis.first < **mis.second;
  }
};

struct PSREqual
{
  bool operator()(const TpdPsr* l, const TpdPsr* r) const
  {
    if (l == nullptr || r == nullptr)
      return false;
    if (l->viaGeoLocs().size() != r->viaGeoLocs().size())
      return false;
    TpdPsrViaGeoLocConstIter lb(l->viaGeoLocs().begin()), le(l->viaGeoLocs().end()),
        rb(r->viaGeoLocs().begin());
    return equal(lb, le, rb, viaGeoLocEqual);
  }
};

struct Rest3or17 : public unary_function<const RoutingRestriction*, bool>
{
  bool operator()(const RoutingRestriction* restriction) const
  {
    return restriction->restriction() == ROUTING_RESTRICTION_3 ||
           restriction->restriction() == ROUTING_RESTRICTION_17;
  }
} rest3or17;

struct RestrictionRetriever
{
  RoutingRestriction* operator()(const RestrictionInfos::value_type& rest_pair) const
  {
    return const_cast<RoutingRestriction*>(rest_pair.first);
  }
} restrictionRetriever;
}

static Logger
logger("atseintl.Xform.FareDisplayResponseRoutingXMLTags");

void
FareDisplayResponseRoutingXMLTags::buildTags(FareDisplayTrx& trx, XMLConstruct& construct)
{
  // Don't do anything if there's no routing data
  if (trx.fdResponse()->uniqueRoutingMap().empty())
    return;

  if (trx.getRequest()->requestedInclusionCode() == ADDON_FARES)
    buildADDisplay(trx, construct);
  else if (trx.allPaxTypeFare().size() > 1 || !trx.isRD())
    buildFDDisplay(trx, construct);
}

void
FareDisplayResponseRoutingXMLTags::addMPM(RoutingInfo* rInfo, XMLConstruct& construct)
{
  MileageInfo* mileageInfo = rInfo->mileageInfo();
  if (mileageInfo != nullptr)
  {
    // MPM
    uint16_t mileageAllowance = mileageInfo->totalApplicableMPM();

    std::ostringstream s;
    s << mileageAllowance;
    construct.addAttribute("S48", s.str());
  }
}

// -------------------------------------------------------------------
// FareDisplayResponseRoutingXMLTags::buildFDDisplay()
// -------------------------------------------------------------------------
bool
FareDisplayResponseRoutingXMLTags::buildFDDisplay(FareDisplayTrx& trx, XMLConstruct& construct)
{
  RtgSeq2PaxTypeFareConstIter uniqueIter(trx.fdResponse()->uniqueRoutings().begin());
  RtgSeq2PaxTypeFareConstIter uniqueIterEnd(trx.fdResponse()->uniqueRoutings().end());

  RoutingInfoMapConstIter uniqueRtgMapIter;

  for (; uniqueIter != uniqueIterEnd; uniqueIter++)
  {
    // Empty it
    oss.str("");
    PaxTypeFare* paxTypeFare = (*uniqueIter).second;

    if (paxTypeFare->fareDisplayInfo() == nullptr)
    {
      LOG4CXX_WARN(logger, "No FareDisplayInfo for routing number: " << (*uniqueIter).first);
      continue;
    }

    construct.openElement("RTG");

    FareDisplayInfo* fareDisplayInfo = paxTypeFare->fareDisplayInfo();

    // Get global direction
    std::string globalDir;
    if (trx.isDomestic())
      globalDirectionToStr(globalDir, GlobalDirection::DO);
    else
      globalDirectionToStr(globalDir, paxTypeFare->globalDirection());

    construct.addAttribute("A60", globalDir);

    // Get the RoutingInfo
    std::string rtgSeq;
    if (!fareDisplayInfo->routingSequence().empty())
      rtgSeq = fareDisplayInfo->routingSequence();
    else
      rtgSeq = paxTypeFare->routingNumber();

    construct.addAttribute("S49", rtgSeq);

    uniqueRtgMapIter = trx.fdResponse()->uniqueRoutingMap().find(rtgSeq);
    if (uniqueRtgMapIter == trx.fdResponse()->uniqueRoutingMap().end())
    {
      LOG4CXX_WARN(logger, "No unique map for: " << fareDisplayInfo->routingSequence());
      construct.closeElement();
      continue;
    }
    RoutingInfo* routingInfo = (*uniqueRtgMapIter).second;
    if (routingInfo == nullptr)
    {
      LOG4CXX_WARN(logger, "Empty routingInfo for: " << fareDisplayInfo->routingSequence());
      construct.closeElement();
      continue;
    }
    else if (routingInfo->routing() == nullptr && routingInfo->origAddOnRouting() == nullptr &&
             routingInfo->destAddOnRouting() == nullptr &&
             paxTypeFare->routingNumber() != MILEAGE_ROUTING)
    {
      LOG4CXX_WARN(logger, "No routing data for: " << paxTypeFare->routingNumber());
      oss << right << setfill(' ') << setw(4) << rtgSeq << "* ";
      oss << " NO ROUTING INFORMATION\n";

      addMPM(routingInfo, construct);
      construct.closeElement();
      continue;
    }

    // Add MPM to XML tag
    addMPM(routingInfo, construct);

    if (fareDisplayInfo->routingSequence().empty())
    {
      bool indentRest(true);
      /*
         int16_t routingNum = atoi(paxTypeFare->routingNumber().c_str());
         oss.setf(std::ios::right, std::ios::adjustfield);
         oss.setf(std::ios::fixed, std::ios::floatfield);
         oss << std::setfill(' ') << std::setw(4) << routingNum << "* ";
         oss.setf(std::ios::left, std::ios::adjustfield);
         */

      if (routingInfo->mapInfo() != nullptr)
      {
        addRouteString(trx, routingInfo->mapInfo()->routeStrings(), false, true, true);
      }
      else
      {
        indentRest = false;
        LOG4CXX_INFO(logger, "Unable to get route string");
      }
      if (routingInfo->routing() != nullptr && !routingInfo->routing()->rests().empty())
        addRestrictions(trx, routingInfo->routing()->rests(), indentRest, true);
      else if (!indentRest) // there are no restrictions and no maps
        displayNonstop(trx, false);
    }
    else
    {
      // oss << fareDisplayInfo->routingSequence() << "* ";
      if (!addGlobalDescription(trx, paxTypeFare->globalDirection(), true))
      {
        LOG4CXX_INFO(logger, "Unable to get global direction desc");
      }

      bool hasConstructed = paxTypeFare->hasConstructedRouting();

      displayConstrVsPubl(trx, *paxTypeFare);
      addMPMvsRTG(trx, paxTypeFare->routingNumber(), routingInfo, hasConstructed);
      RoutingRestrictions rests;
      if (routingInfo != nullptr && routingInfo->restrictions() != nullptr &&
          !routingInfo->restrictions()->empty())
      {
        transform(routingInfo->restrictions()->begin(),
                  routingInfo->restrictions()->end(),
                  back_inserter(rests),
                  restrictionRetriever);
      }
      if (!rests.empty())
      {
        addRestrictions(trx, rests, true, true);
      }

      displayTPD(trx, *routingInfo);
      displayPSR(trx, *routingInfo);
      if (hasConstructed)
      {
        displayConstructed(trx, *paxTypeFare, routingInfo, true);
      }
      else
      {
        if (routingInfo != nullptr && routingInfo->mapInfo() != nullptr)
        {
          if (!isNonstop(trx, routingInfo->mapInfo()->routeStrings()))
          {
            displayDRV(trx, routingInfo->routing(), *paxTypeFare, true);
          }
          addRouteString(trx, routingInfo->mapInfo()->routeStrings(), true, true, true);
        }
        else
        {
          LOG4CXX_INFO(logger, "Unable to get route string");
        }
      }
    }
    std::string desc(oss.str());
    unsigned int lastPos = 0;
    while (1)
    {
      lastPos = desc.rfind("\n");
      if (lastPos > 0 && lastPos == (desc.length() - 1))
        desc.replace(lastPos, 1, "\0");
      else
        break;
    }
    char* pHolder = nullptr;
    for (char* token = strtok_r((char*)desc.c_str(), "\n", &pHolder); token != nullptr;
         token = strtok_r(nullptr, "\n~", &pHolder))
    {
      if (strlen(token) == 0)
        continue;

      std::string line(token);
      construct.openElement("RTD");
      while (line[0] == ' ')
        line.erase(0, 1);
      construct.addAttribute("S80", line);
      construct.closeElement();
    }
    construct.closeElement();
  }

  return true;
}

bool
FareDisplayResponseRoutingXMLTags::buildADDisplay(FareDisplayTrx& trx, XMLConstruct& construct)
{
  // Not Implemented Yet
  return false;

  if (trx.fdResponse()->uniqueAddOnRoutings().empty())
    return false;
  AddOnFareInfosConstIter itr(trx.fdResponse()->uniqueAddOnRoutings().begin());
  AddOnFareInfosConstIter end(trx.fdResponse()->uniqueAddOnRoutings().end());
  for (; itr != end; ++itr)
  {
    const FDAddOnFareInfo* fareInfo(itr->second);
    if (fareInfo == nullptr)
    {
      LOG4CXX_WARN(logger, "Empty fareInfo for: " << itr->first);
      continue;
    }
    std::string rtgSeq;
    if (!fareInfo->addOnRoutingSeq().empty())
      rtgSeq = fareInfo->addOnRoutingSeq();
    else
      rtgSeq = fareInfo->routing();
    RoutingInfoMapConstIter i(trx.fdResponse()->uniqueRoutingMap().find(rtgSeq));
    if (i == trx.fdResponse()->uniqueRoutingMap().end())
    {
      LOG4CXX_WARN(logger, "No unique map for: " << fareInfo->addOnRoutingSeq());
      continue;
    }
    const RoutingInfo* routingInfo(i->second);
    if (routingInfo == nullptr)
    {
      LOG4CXX_WARN(logger, "Empty routingInfo for: " << fareInfo->addOnRoutingSeq());
      continue;
    }
    else if (routingInfo->routing() == nullptr && fareInfo->routing() != MILEAGE_ROUTING)
    {
      LOG4CXX_WARN(logger, "No routing data for: " << fareInfo->routing());
      oss << right << setfill(' ') << setw(4) << rtgSeq << "* ";
      oss << " NO ROUTING INFORMATION\n";
      continue;
      oss << right << setfill(' ') << setw(4) << rtgSeq << "*  NO ROUTING INFORMATION\n";
      continue;
    }

    if (fareInfo->addOnRoutingSeq().empty())
    {
      bool indentRest(true);
      oss << FareDisplayResponseUtil::routingNumberToStringFormat(fareInfo->routing(),
                                                                  std::ios::right) << "* ";
      oss.setf(std::ios::left, std::ios::adjustfield);

      if (routingInfo->mapInfo() != nullptr)
      {
        addRouteString(trx, routingInfo->mapInfo()->routeStrings(), false, true, true);
      }
      else
      {
        indentRest = false;
        LOG4CXX_INFO(logger, "Unable to get route string");
      }
      if (routingInfo->routing() != nullptr && !routingInfo->routing()->rests().empty())
        addRestrictions(trx, routingInfo->routing()->rests(), indentRest, true);
    }
    else
    {
      oss << fareInfo->addOnRoutingSeq() << "* ";
      if (!addGlobalDescription(trx, fareInfo->globalDir(), true))
        LOG4CXX_INFO(logger, "Unable to get global direction desc");
      oss << " PUBLISHED";
      addMPMvsRTG(trx, fareInfo->routing(), routingInfo, false);
      if (routingInfo->routing() != nullptr && !routingInfo->routing()->rests().empty())
        addRestrictions(trx, routingInfo->routing()->rests(), true, true);
      displayTPD(trx, *routingInfo);
      displayPSR(trx, *routingInfo);
      if (routingInfo->mapInfo() != nullptr)
      {
        addRouteString(trx, routingInfo->mapInfo()->routeStrings(), true, true, true);
      }
      else
        LOG4CXX_INFO(logger, "Unable to get route string");
    }
  }
  return true;
}

// -------------------------------------------------------------------
// FareDisplayResponseRoutingXMLTags::addRouteString()
// -------------------------------------------------------------------------
bool
FareDisplayResponseRoutingXMLTags::addRouteString(FareDisplayTrx& trx,
                                                  const RoutingMapStrings* strings,
                                                  bool indent,
                                                  bool useLineNumbers,
                                                  bool fdDisplay)
{
  if (isNonstop(trx, strings))
  {
    displayNonstop(trx, fdDisplay && indent);
    return true;
  }
  uint32_t lineNum = 1;
  uint32_t noLineNum = 0;
  uint16_t lines(0);
  RoutingMapStrings::const_iterator iter = strings->begin();
  RoutingMapStrings::const_iterator iterEnd = strings->end();
  for (; iter != iterEnd; iter++)
  {
    LOG4CXX_DEBUG(logger, "Size: " << (*iter).size() << " '" << (*iter) << "'");
    if ((*iter).size() == 0)
    {
      continue;
    }
    else if ((*iter).size() > (fdDisplay ? MAX_RTE_LEN_FQ : MAX_RTE_LEN_RD))
    {
      LOG4CXX_DEBUG(logger, "Line too long!!");
      splitLine(trx, (*iter), !indent, (useLineNumbers ? lineNum++ : noLineNum), fdDisplay);
    }
    else
    {
      if (indent)
        oss << SIX_BLANKS;
      if (useLineNumbers)
      {
        oss.setf(std::ios::right, std::ios::adjustfield);
        oss.setf(std::ios::fixed, std::ios::floatfield);
        oss << std::setw(2) << std::setfill(' ') << lineNum++ << ". ";
      }
      else
        oss << TWO_BLANKS;
      oss << *iter << '\n';
    }
    if (!indent && fdDisplay)
      indent = true;
    if (fdDisplay && ++lines >= MAX_FD_RTG_LINES)
    {
      if (indent)
        oss << STRING_INDENT;
      oss << TOO_MANY_LINES << std::endl;
      return true;
    }
  }
  return true;
}

bool
FareDisplayResponseRoutingXMLTags::isNonstop(const FareDisplayTrx& trx,
                                             const RoutingMapStrings* strings)
{
  return strings == nullptr || strings->empty() ||
         (strings->size() == 1 && (*strings)[0] == trx.boardMultiCity() + "-" + trx.offMultiCity());
}

void
FareDisplayResponseRoutingXMLTags::displayNonstop(FareDisplayTrx& trx, bool indent)
{
  if (indent)
    oss << SIX_BLANKS;
  oss << " NONSTOP\n";
}

// -------------------------------------------------------------------
// FareDisplayResponseRoutingXMLTags::addGlobalDescription()
// -------------------------------------------------------------------------
bool
FareDisplayResponseRoutingXMLTags::addGlobalDescription(FareDisplayTrx& trx,
                                                        GlobalDirection global,
                                                        bool inLine)
{
  // Find the Global Direction description
  const GlobalDir* globalDir = trx.dataHandle().getGlobalDir(global, trx.travelDate());

  if (!inLine)
    oss << "\n";

  if (globalDir == nullptr)
    return false;

  if (inLine)
    oss << SINGLE_BLANK;
  ostringstream line;
  line << SLASH << globalDir->description() << SLASH;
  const std::string& str(line.str());
  const uint16_t maxLen(inLine ? MAX_RTE_LEN_FQ : MAX_LINE_LENGTH);
  const uint16_t lineLimit(maxLen - GLOB_DIR_MARGIN);
  if (str.size() <= maxLen)
  {
    oss << str;
    if (str.size() > lineLimit)
      oss << "\n";
  }
  else
  {
    std::string::size_type pos(str.rfind(' ', maxLen));
    if (pos == std::string::npos)
      pos = maxLen - 1;
    oss << str.substr(0, pos) << "\n";
    if (inLine)
      oss << STRING_INDENT;
    oss << str.substr(pos + 1);
    if (str.substr(pos + 1).size() > lineLimit)
      oss << "\n";
  }
  return true;
}

bool
FareDisplayResponseRoutingXMLTags::addMPMvsRTG(FareDisplayTrx& trx,
                                               const RoutingNumber& routingNumber,
                                               const RoutingInfo* info,
                                               bool constructed)
{
  if (info == nullptr)
  {
    return false;
    ;
  }

  const MileageInfo* mileage(info->mileageInfo());
  if (mileage != nullptr && routingNumber == MILEAGE_ROUTING)
  {
    oss << SINGLE_BLANK << "MPM ";
    if (mileage->totalApplicableMPM() > 0)
      oss << mileage->totalApplicableMPM();
    oss << "\n";
  }
  else
  {
    oss << " RTG ";
    if (!constructed && info->routing() != nullptr)
      oss << FareDisplayResponseUtil::routingNumberToString(info->routing()->routing());
    oss << "\n";
  }
  return true;
}

void
FareDisplayResponseRoutingXMLTags::addRestrictions(FareDisplayTrx& trx,
                                                   const RoutingRestrictions& restrictions,
                                                   bool indent,
                                                   bool fdDisplay)
{
  RoutingRestrictionsConstIter restIter = restrictions.begin();
  RoutingRestrictionsConstIter restIterEnd = restrictions.end();
  // Handler
  RestrictionsText restText(oss, false);

  bool isRTWorCT = RestrictionsText::isRtwOrCt(trx);

  for (; restIter != restIterEnd; restIter++)
  {
    const RoutingRestriction* routingRestriction = (*restIter);
    oss << " ";
    LOG4CXX_DEBUG(logger, "Restriction number: " << routingRestriction->restriction());

    switch (atoi(routingRestriction->restriction().c_str()))
    {
    case 0: // Ignore   MILEAGE_ROUTING
      break;
    case 1:
      restText.restriction1(*routingRestriction, indent);
      break;
    case 2:
      restText.restriction2(*routingRestriction, indent);
      break;
    case 3:
      if (!isRTWorCT)
        restText.restriction3(*routingRestriction, indent);
      break;
    case 4:
      restText.restriction4(*routingRestriction, indent);
      break;
    case 5:
      restText.restriction5(*routingRestriction, indent);
      break;
    case 6:
      restText.restriction6(*routingRestriction, indent);
      break;
    case 7:
      restText.restriction7(*routingRestriction, indent);
      break;
    case 8:
      if (!fallback::fallbackFMRbyte58(&trx))
        restText.restriction8FMR(trx, *routingRestriction, indent);
      else
        restText.restriction8(*routingRestriction, indent);
      break;
    case 9:
      restText.restriction9FMR(trx, indent);
      break;
    case 10:
      restText.restriction10(*routingRestriction, indent);
      break;
    case 11:
      restText.restriction11(*routingRestriction, indent);
      break;
    case 12:
      if (!isRTWorCT)
        restText.restriction12(*routingRestriction, indent);
      break;
    case 13:
      break;
    case 14:
      break;
    case 15:
      break;
    case 16:
      if (!isRTWorCT)
        restText.restriction16(*routingRestriction, indent);
      break;
    case 17:
      restText.restriction17(*routingRestriction, indent);
      break;
    case 18:
      break;
    case 19:
      break;
    case 21:
      restText.restriction21(*routingRestriction, indent);
      break;
    default:
      LOG4CXX_ERROR(logger, "Unhandled restriction: " << routingRestriction->restriction());
    }

    if (!indent && fdDisplay)
    {
      indent = true;
    }
  }
}

// -------------------------------------------------------------------
// FareDisplayResponseRoutingXMLTags::splitLine()
// -------------------------------------------------------------------------
void
FareDisplayResponseRoutingXMLTags::splitLine(
    FareDisplayTrx& trx, const std::string& theLine, bool inLine, uint32_t lineNum, bool fdDisplay)
{
  const uint16_t maxLen(fdDisplay ? MAX_RTE_LEN_FQ : MAX_RTE_LEN_RD);
  size_t length = theLine.size(), currLen(maxLen);
  std::string partOf = theLine;
  bool first = true;
  while (currLen < length)
  {
    if (fdDisplay && (!inLine || !first))
    {
      oss << SIX_BLANKS;
    }

    if (lineNum != 0 && first)
    {
      oss.setf(std::ios::right, std::ios::adjustfield);
      oss.setf(std::ios::fixed, std::ios::floatfield);
      oss << std::setw(2) << std::setfill(' ') << lineNum << ". ";
    }
    else
    {
      oss << FOUR_BLANKS;
    }

    currLen = partOf.find_last_of(DELIMS, maxLen - 1);
    if (currLen == std::string::npos)
    {
      currLen = maxLen;
    }
    else
    {
      ++currLen;
    }

    LOG4CXX_DEBUG(logger, "to display: " << partOf.substr(0, currLen) << "\n");
    oss << partOf.substr(0, currLen) << std::endl;

    partOf = partOf.substr(currLen);
    length = partOf.size();

    if (first == true)
    {
      first = false;
    }
  }

  // The last part
  if (partOf.size() > 0)
  {
    if (fdDisplay)
    {
      oss << SIX_BLANKS;
    }
    oss << FOUR_BLANKS << partOf << std::endl;
  }
}

void
FareDisplayResponseRoutingXMLTags::displayPSR(FareDisplayTrx& trx, const RoutingInfo& rtgInfo)
{
  // eliminate duplicate (as far as the display is concerned) PSR strings
  // PSR strings are considered duplicates if they have identical viaGeoLoc vectors
  // from the duplicate PSRs, the one with carrier set (non-blank) should be displayed
  TpdPsrs noDupPSR(rtgInfo.fdPSR());
  sort(noDupPSR.begin(), noDupPSR.end(), PSRLessThan());
  noDupPSR.erase(unique(noDupPSR.begin(), noDupPSR.end(), PSREqual()), noDupPSR.end());

  std::vector<TpdPsr*>::const_iterator itrpsrB = noDupPSR.begin();
  std::vector<TpdPsr*>::const_iterator itrpsrE = noDupPSR.end();

  for (; itrpsrB != itrpsrE; ++itrpsrB)
  {

    TpdPsr& psr(**itrpsrB);

    LOG4CXX_DEBUG(logger,
                  "PSR: carrier " << psr.carrier() << " area 1 " << psr.area1() << " area 2 "
                                  << psr.area2() << " seqNo " << psr.seqNo())

    if (psr.viaGeoLocs().empty())
      continue;

    oss << STRING_INDENT << "SPECIFIED ROUTING PERMITTED AS FOLLOWS" << std::endl;

    oss << STRING_INDENT;

    std::vector<TpdPsrViaGeoLoc*>::const_iterator geoLocItr = psr.viaGeoLocs().begin();
    std::vector<TpdPsrViaGeoLoc*>::const_iterator geoLocEnd = psr.viaGeoLocs().end();

    uint16_t lineLength = 59;
    int16_t charCount = STRING_INDENT_LENGTH;

    oss << trx.origin()->loc() << DASH;
    charCount += 4;

    for (; geoLocItr != geoLocEnd; ++geoLocItr)
    {
      if ((charCount + 8) > lineLength)
      {
        oss << std::endl;
        oss << STRING_INDENT;
        charCount = STRING_INDENT_LENGTH;
      }

      if ((**geoLocItr).relationalInd() == VIAGEOLOCREL_AND)
      {
        oss << DASH; //" AND ";
        // charCount += 4;
        charCount += 1;
      }
      else if ((**geoLocItr).relationalInd() == VIAGEOLOCREL_OR)
      {
        oss << SLASH; //" OR ";
        // charCount += 4;
        charCount += 1;
      }
      else if ((**geoLocItr).relationalInd() == VIAGEOLOCREL_ANDOR)
      {
        oss << DASH; //" AND/OR ";
        // charCount += 8;
        charCount += 1;
      }

      if ((charCount + 15) > lineLength)
      {
        oss << std::endl;
        oss << STRING_INDENT;
        charCount = STRING_INDENT_LENGTH;
      }

      if ((**geoLocItr).loc().locType() == LOCTYPE_ZONE)
      {
        oss << "ZONE ";
        charCount += 5;
      }
      else if ((**geoLocItr).loc().locType() == LOCTYPE_AREA)
      {
        oss << "AREA ";
        charCount += 5;
      }
      else if ((**geoLocItr).loc().locType() == LOCTYPE_SUBAREA)
      {
        oss << "SUB AREA ";
        charCount += 9;
      }
      else if ((**geoLocItr).loc().locType() == LOCTYPE_STATE)
      {
        oss << "STATE/PROVINCE ";
        charCount += 15;
      }

      std::string desc;

      if ((**geoLocItr).loc().locType() == LOCTYPE_NATION)
      {
        const Nation* nation =
            trx.dataHandle().getNation((**geoLocItr).loc().loc(), trx.travelDate());
        desc = nation->description();
      }
      else
      {
        desc = (**geoLocItr).loc().loc();
      }

      if ((charCount + desc.size()) > lineLength)
      {
        oss << std::endl;
        oss << STRING_INDENT;
        charCount = STRING_INDENT_LENGTH;
      }

      oss << desc;
      charCount += desc.size();
    } // End of iner for

    oss << DASH << trx.destination()->loc();
    charCount += 4;

    oss << std::endl;

    geoLocItr = psr.viaGeoLocs().begin();
    geoLocEnd = psr.viaGeoLocs().end();
    if (psr.stopoverCnt() != -1)
      switch (psr.stopoverCnt())
      {
      case 0:
        trx.response() << STRING_INDENT << "ROUTING VALID PROVIDED NO STOPOVER ENROUTE"
                       << std::endl;
        break;
      case 1:
        trx.response() << STRING_INDENT << "ROUTING VALID PROVIDED ONLY ONE STOPOVER ENROUTE"
                       << std::endl;
        break;
      default:
        trx.response() << STRING_INDENT << "ROUTING VALID PROVIDED AT LEAST ONE STOPOVER ENROUTE"
                       << std::endl;
      }

    for (; geoLocItr != geoLocEnd; ++geoLocItr)
    {
      if (((*geoLocItr)->stopoverNotAllowed() == 'Y'))
      {
        std::string desc = (*geoLocItr)->loc().loc();
        if ((*geoLocItr)->loc().locType() == LOCTYPE_NATION)
        {
          const Nation* nation =
              trx.dataHandle().getNation((*geoLocItr)->loc().loc(), trx.travelDate());
          if (nation)
            desc = nation->description();
        }
        trx.response() << STRING_INDENT << "ROUTING VALID PROVIDED NO STOPOVER AT " << desc
                       << std::endl;
      }
    }
  } // End of for ( itrpsrB != itrpsrE)
}

void
FareDisplayResponseRoutingXMLTags::displayTPD(FareDisplayTrx& trx, const RoutingInfo& rtgInfo)
{
  std::vector<TpdPsr*>::const_iterator itrpsrB = rtgInfo.fdTPD().begin();
  std::vector<TpdPsr*>::const_iterator itrpsrE = rtgInfo.fdTPD().end();

  for (; itrpsrB != itrpsrE; ++itrpsrB)
  {
    TpdPsr& tpd(**itrpsrB);

    LOG4CXX_DEBUG(logger,
                  "TPD: carrier " << tpd.carrier() << " area 1 " << tpd.area1() << " area 2 "
                                  << tpd.area2() << " seqNo " << tpd.seqNo())

    if (tpd.viaGeoLocs().empty())
      continue;

    oss << STRING_INDENT << "TICKETED POINT DEDUCTION OF " << tpd.tpmDeduction()
        << " MILES APPLIES\n";
    oss << STRING_INDENT << "WHEN TRAVEL IS VIA ";

    std::vector<TpdPsrViaGeoLoc*>::const_iterator geoLocItr = tpd.viaGeoLocs().begin();
    std::vector<TpdPsrViaGeoLoc*>::const_iterator geoLocEnd = tpd.viaGeoLocs().end();

    uint16_t lineLength(MAX_LINE_LENGTH);
    int16_t charCount(25);
    vector<TpdPsrViaGeoLoc*> stopovers;

    for (; geoLocItr != geoLocEnd; ++geoLocItr)
    {
      if ((*geoLocItr)->stopoverNotAllowed() == STPOVRNOTALWD_YES)
        stopovers.push_back(*geoLocItr);

      if ((charCount + 8) > lineLength)
      {
        oss << std::endl;
        oss << STRING_INDENT;
        charCount = STRING_INDENT_LENGTH;
      }

      if ((**geoLocItr).relationalInd() == VIAGEOLOCREL_AND)
      {
        oss << AND;
        charCount += AND_LEN;
      }
      else if ((**geoLocItr).relationalInd() == VIAGEOLOCREL_OR)
      {
        oss << OR;
        charCount += OR_LEN;
      }
      else if ((**geoLocItr).relationalInd() == VIAGEOLOCREL_ANDOR)
      {
        oss << ANDOR;
        charCount += ANDOR_LEN;
      }
      else if (geoLocItr != tpd.viaGeoLocs().begin())
      { // if relational indicator is empty and it is not the first via loc
        // it means the beginning of new set - sets are combined with OR
        oss << OR;
        charCount += OR_LEN;
      }

      if ((charCount + 15) > lineLength)
      {
        oss << std::endl;
        oss << STRING_INDENT;
        charCount = STRING_INDENT_LENGTH;
      }

      else if ((**geoLocItr).loc().locType() == LOCTYPE_AREA)
      {
        oss << "AREA ";
        charCount += 5;
      }
      else if ((**geoLocItr).loc().locType() == LOCTYPE_SUBAREA)
      {
        oss << "SUB AREA ";
        charCount += 9;
      }
      else if ((**geoLocItr).loc().locType() == LOCTYPE_STATE)
      {
        oss << "STATE/PROVINCE ";
        charCount += 15;
      }

      std::string desc;

      if ((**geoLocItr).loc().locType() == LOCTYPE_NATION)
      {
        const Nation* nation =
            trx.dataHandle().getNation((**geoLocItr).loc().loc(), trx.travelDate());
        desc = nation->description();
      }
      else if ((**geoLocItr).loc().locType() == LOCTYPE_ZONE)
      {
        const size_t len(MAX_RTE_LEN_FQ + STRING_INDENT_LENGTH);
        desc = FareDisplayUtil::splitLines(
            FareDisplayUtil::getZoneDescr((*geoLocItr)->loc().loc(), trx),
            len,
            "/ ",
            len - charCount,
            STRING_INDENT);
      }
      else
      {
        desc = (**geoLocItr).loc().loc();
      }

      if ((charCount + desc.size()) > lineLength)
      {
        oss << std::endl;
        oss << STRING_INDENT;
        charCount = STRING_INDENT_LENGTH;
      }

      oss << desc;
      charCount += desc.size();
    } // End of iner for

    oss << std::endl;

    if (!stopovers.empty())
    {
      oss << STRING_INDENT << "PROVIDED NO STOPOVER AT ";
      charCount = 24;
      vector<TpdPsrViaGeoLoc*>::const_iterator stpovrItr(stopovers.begin());
      vector<TpdPsrViaGeoLoc*>::const_iterator stpovrEnd(stopovers.end());
      for (; stpovrItr != stpovrEnd; ++stpovrItr)
      {
        if ((charCount + 8) > lineLength)
        {
          oss << std::endl;
          oss << STRING_INDENT;
          charCount = STRING_INDENT_LENGTH;
        }
        if ((*stpovrItr)->relationalInd() == VIAGEOLOCREL_AND)
        {
          oss << AND;
          charCount += AND_LEN;
        }
        else if ((*stpovrItr)->relationalInd() == VIAGEOLOCREL_OR)
        {
          oss << OR;
          charCount += OR_LEN;
        }
        else if ((*stpovrItr)->relationalInd() == VIAGEOLOCREL_ANDOR)
        {
          oss << ANDOR;
          charCount += ANDOR_LEN;
        }
        else if (stpovrItr != stopovers.begin())
        { // if relational indicator is empty and it is not the first via loc
          // it means the beginning of new set - sets are combined with OR
          oss << OR;
          charCount += OR_LEN;
        }
        if ((charCount + 3) > lineLength)
        {
          oss << std::endl;
          oss << STRING_INDENT;
          charCount = STRING_INDENT_LENGTH;
        }
        oss << (*stpovrItr)->loc().loc();
      }
      oss << endl;
    }

    if (tpd.thisCarrierRestr() == 'Y')
    {
      oss << STRING_INDENT << "PROVIDED ALL TRAVEL IS VIA " << tpd.carrier() << std::endl;
    }

  } // End of for ( itrpsrB != itrpsrE)
}

bool
FareDisplayResponseRoutingXMLTags::displayDRV(FareDisplayTrx& trx,
                                              const Routing* routing,
                                              const PaxTypeFare& paxTypeFare,
                                              bool indent)
{
  if (paxTypeFare.routingNumber() != MILEAGE_ROUTING && routing != nullptr &&
      routing->domRtgvalInd() == DOMESTIC_ROUTE_VALIDATION &&
      LocUtil::isInternational(*paxTypeFare.fareMarket()->origin(),
                               *paxTypeFare.fareMarket()->destination()))
  {
    if (indent)
      oss << STRING_INDENT;
    const CarrierPreference* cxrPref(
        trx.dataHandle().getCarrierPreference(paxTypeFare.carrier(), trx.travelDate()));
    if (cxrPref != nullptr && cxrPref->noApplydrvexceptus() == YES)
      oss << "DOM ROUTE VALIDATION APPLIES WITHIN US ONLY\n";
    else
      oss << "DOM ROUTE VALIDATION APPLIES WITHIN ORIG/DEST COUNTRIES\n";
    return true;
  }
  return false;
}

void
FareDisplayResponseRoutingXMLTags::displayDRV(FareDisplayTrx& trx,
                                              const Routings& routings,
                                              const PaxTypeFare& paxTypeFare,
                                              bool indent)
{
  RoutingsConstIter itr(routings.begin());
  RoutingsConstIter end(routings.end());
  for (; itr != end; ++itr)
  {
    if (displayDRV(trx, *itr, paxTypeFare, indent))
      break;
  }
}

void
FareDisplayResponseRoutingXMLTags::displayConstrVsPubl(FareDisplayTrx& trx,
                                                       const PaxTypeFare& paxTypeFare)
{
  if (paxTypeFare.hasConstructedRouting())
  {
    oss << " CONSTRUCTED";
  }
  else
  {
    oss << " PUBLISHED";
  }
}

void
FareDisplayResponseRoutingXMLTags::displayConstructed(FareDisplayTrx& trx,
                                                      const PaxTypeFare& paxTypeFare,
                                                      const RoutingInfo* info,
                                                      bool fq)
{
  if (info == nullptr)
    return;
  const Routing* base(info->routing());
  const Routing* origAddOn(info->origAddOnRouting());
  const Routing* destAddOn(info->destAddOnRouting());
  const MapInfo* mapInfo(info->mapInfo());
  RoutingRestrictions rests;
  const FareMarket& fareMarket(*paxTypeFare.fareMarket());
  if (info->rtgAddonMapInfo() != nullptr)
  { // RTG-MPM-RTG construction
    if (base != nullptr && !base->rests().empty())
    {
      remove_copy_if(
          base->rests().begin(), base->rests().end(), back_inserter(rests), not1(rest3or17));
      addRestrictions(trx, rests, false, fq);
    }
    const MapInfo* rtgAddonMapInfo(info->rtgAddonMapInfo());
    if ((mapInfo != nullptr && mapInfo->routeStrings() != nullptr && !mapInfo->routeStrings()->empty()) ||
        (origAddOn != nullptr && !origAddOn->rests().empty()))
    {
      displayConstrMsg(trx,
                       fareMarket.boardMultiCity(),
                       gateway(translateNation(fareMarket.origin()->nation())),
                       fq);
      if (origAddOn != nullptr && !origAddOn->rests().empty())
      {
        rests.clear();
        remove_copy_if(origAddOn->rests().begin(),
                       origAddOn->rests().end(),
                       back_inserter(rests),
                       not1(rest3or17));
        addRestrictions(trx, rests, false, fq);
      }
      if (mapInfo != nullptr)
      {
        if (!isNonstop(trx, mapInfo->routeStrings()))
          displayDRV(trx, origAddOn, paxTypeFare, fq);
        addRouteString(trx, mapInfo->routeStrings(), fq, true, fq);
      }
    }
    if ((rtgAddonMapInfo->routeStrings() != nullptr && !rtgAddonMapInfo->routeStrings()->empty()) ||
        (destAddOn != nullptr && !destAddOn->rests().empty()))
    {
      displayConstrMsg(trx,
                       gateway(translateNation(fareMarket.destination()->nation())),
                       fareMarket.offMultiCity(),
                       fq);
      if (destAddOn != nullptr && !destAddOn->rests().empty())
      {
        rests.clear();
        remove_copy_if(destAddOn->rests().begin(),
                       destAddOn->rests().end(),
                       back_inserter(rests),
                       not1(rest3or17));
        addRestrictions(trx, rests, false, fq);
      }
      if (!isNonstop(trx, rtgAddonMapInfo->routeStrings()))
        displayDRV(trx, destAddOn, paxTypeFare, fq);
      addRouteString(trx, rtgAddonMapInfo->routeStrings(), fq, true, fq);
    }
  }
  else
  { // other construction types
    std::string orig, dest;
    RoutingRestrictions restsBeforeStrings;
    Routings drvs;
    if (origAddOn != nullptr && !origAddOn->rmaps().empty())
    {
      orig = fareMarket.boardMultiCity();
      dest = gateway(translateNation(fareMarket.origin()->nation()));
      if (!origAddOn->rests().empty())
      {
        remove_copy_if(origAddOn->rests().begin(),
                       origAddOn->rests().end(),
                       back_inserter(rests),
                       not1(rest3or17));
      }
      drvs.push_back(origAddOn);
    }
    else if (origAddOn != nullptr && !origAddOn->rests().empty())
    {
      remove_copy_if(origAddOn->rests().begin(),
                     origAddOn->rests().end(),
                     back_inserter(restsBeforeStrings),
                     not1(rest3or17));
    }
    if (base != nullptr && !base->rmaps().empty())
    {
      if (orig.empty())
      {
        if (origAddOn != nullptr)
          orig = gateway(translateNation(fareMarket.origin()->nation()));
        else
          orig = fareMarket.boardMultiCity();
      }
      if (destAddOn != nullptr)
        dest = gateway(translateNation(fareMarket.destination()->nation()));
      else
        dest = fareMarket.offMultiCity();
      if (!base->rests().empty())
      {
        remove_copy_if(
            base->rests().begin(), base->rests().end(), back_inserter(rests), not1(rest3or17));
      }
      drvs.push_back(base);
    }
    else if (base != nullptr && !base->rests().empty())
    {
      remove_copy_if(base->rests().begin(),
                     base->rests().end(),
                     back_inserter(restsBeforeStrings),
                     not1(rest3or17));
    }
    if (destAddOn != nullptr && !destAddOn->rmaps().empty())
    {
      if (orig.empty())
        orig = gateway(translateNation(fareMarket.destination()->nation()));
      dest = fareMarket.offMultiCity();
      if (!destAddOn->rests().empty())
      {
        remove_copy_if(destAddOn->rests().begin(),
                       destAddOn->rests().end(),
                       back_inserter(rests),
                       not1(rest3or17));
      }
      drvs.push_back(destAddOn);
    }
    else if (destAddOn != nullptr && !destAddOn->rests().empty())
    {
      remove_copy_if(destAddOn->rests().begin(),
                     destAddOn->rests().end(),
                     back_inserter(restsBeforeStrings),
                     not1(rest3or17));
    }
    addRestrictions(trx, restsBeforeStrings, false, fq);
    // display routing restrictions limitation message only if not all existing parts
    // of constructed routing were included in routing strings extranction (some are mileage)
    uint16_t parts(1); // assume base is always present - if it is null it means it is SITA mileage
    if (origAddOn != nullptr)
      ++parts;
    if (destAddOn != nullptr)
      ++parts;
    if (!orig.empty() && !dest.empty() && drvs.size() < parts)
      displayConstrMsg(trx, orig, dest, fq);
    addRestrictions(trx, rests, false, fq);
    if (info->mapInfo() != nullptr)
    {
      if (!isNonstop(trx, info->mapInfo()->routeStrings()))
        displayDRV(trx, drvs, paxTypeFare, fq);
      addRouteString(trx, info->mapInfo()->routeStrings(), fq, true, fq);
    }
  }
}

void
FareDisplayResponseRoutingXMLTags::displayConstrMsg(FareDisplayTrx& trx,
                                                    const std::string& loc1,
                                                    const std::string& loc2,
                                                    bool fq)
{
  if (fq)
    oss << STRING_INDENT;
  oss << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
  if (fq)
    oss << STRING_INDENT;
  oss << loc1 << " AND " << loc2 << " ONLY\n";
}

string
FareDisplayResponseRoutingXMLTags::translateNation(const NationCode& loc)
{
  if (loc == NATION_US || loc == NATION_CA)
    return US_CA;
  return loc;
}

string
FareDisplayResponseRoutingXMLTags::gateway(const std::string& loc)
{
  return loc + " GATEWAY";
}
