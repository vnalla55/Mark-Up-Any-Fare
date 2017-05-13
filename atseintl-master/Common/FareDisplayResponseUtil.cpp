//-------------------------------------------------------------------
//  Copyright Sabre 2009
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
#include "Common/FareDisplayResponseUtil.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/NUCCollectionResults.h"
#include "Common/NUCCurrencyConverter.h"
#include "Common/RestrictionsText.h"
#include "Common/RoutingUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FBCategoryRuleRecord.h"
#include "DataModel/TravelInfo.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/GlobalDir.h"
#include "DBAccess/Nation.h"
#include "DBAccess/Routing.h"
#include "Diagnostic/RoutingDiagCollector.h"
#include "Routing/RoutingInfo.h"

#include <algorithm>
#include <array>

using namespace tse;
using namespace std;

namespace tse
{
FALLBACK_DECL(fallbackFMRbyte58);
}

namespace
{
static const char SLASH = '/';
static const char DASH = '-';
static const string DELIMS = "-/";
static const string SINGLE_BLANK = " ";
static const string TWO_BLANKS = "  ";
static const string THREE_BLANKS = "   ";
static const string FOUR_BLANKS = "    ";
static const string FIVE_BLANKS = "     ";
static const string SIX_BLANKS = "      ";
static const string INDEF = "INDEF";
static const string TOO_MANY_LINES = "OVER 20 STRINGS - RD LINE NUM *RTG TO VIEW ROUTINGS";
static const size_t MAX_RTE_LEN_FQ = 53;
static const size_t MAX_RTE_LEN_FQ_RTG = 56;
static const size_t MAX_RTE_LEN_RD = 58;
static const size_t MAX_FD_RTG_LINES = 20;
static const size_t MAX_LINE_LENGTH = 63;
static const size_t GLOB_DIR_MARGIN = 20;
static const size_t STRING_INDENT_LENGTH = 7;
static const string
STRING_INDENT(STRING_INDENT_LENGTH, ' ');
static const string US_CA = "US CA";
static const string AND = " AND ";
static const string OR = " OR ";
static const string ANDOR = " AND/OR ";
static const size_t AND_LEN = AND.size();
static const size_t OR_LEN = OR.size();
static const size_t ANDOR_LEN = ANDOR.size();

typedef vector<TpdPsrViaGeoLoc*>::const_iterator TpdPsrViaGeoLocConstIter;
typedef pair<TpdPsrViaGeoLocConstIter, TpdPsrViaGeoLocConstIter> TpdPsrViaGeoLocConstIterPair;
typedef vector<TpdPsr*> TpdPsrs;
typedef map<RoutingNumber, RoutingInfo*> RoutingInfoMapVec;
typedef RoutingInfoMapVec::const_iterator RoutingInfoMapConstIter;

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

class Not3nor17 : public std::unary_function<const RoutingRestriction*, bool>
{
public:
  Not3nor17(bool constructed) : _constructed(constructed) {}
  bool operator()(const RoutingRestriction* restriction) const
  {
    return !_constructed || (restriction->restriction() != ROUTING_RESTRICTION_3 &&
                             restriction->restriction() != ROUTING_RESTRICTION_17);
  }

private:
  bool _constructed;
};

struct RestrictionRetriever
{
  RoutingRestriction* operator()(const RestrictionInfos::value_type& rest_pair) const
  {
    return const_cast<RoutingRestriction*>(rest_pair.first);
  }
} restrictionRetriever;

struct is17 : public unary_function<const RoutingRestriction*, bool>
{
  bool operator()(const RoutingRestriction* restriction) const
  {
    return restriction->restriction() == ROUTING_RESTRICTION_17;
  }
} is17;

constexpr size_t consolidatedRestsNo = 11;
const std::array<RestrictionNumber, consolidatedRestsNo>&
getConsolidatedRests()
{
  static const std::array<RestrictionNumber, consolidatedRestsNo> consolidatedRests{
      ROUTING_RESTRICTION_1,
      ROUTING_RESTRICTION_2,
      ROUTING_RESTRICTION_5,
      ROUTING_RESTRICTION_7,
      RTW_ROUTING_RESTRICTION_8,
      RTW_ROUTING_RESTRICTION_9,
      RTW_ROUTING_RESTRICTION_10,
      ROUTING_RESTRICTION_17,
      ROUTING_RESTRICTION_18,
      ROUTING_RESTRICTION_19,
      ROUTING_RESTRICTION_21};
  return consolidatedRests;
}

struct IsConsolidated
{
  bool _exceptRestr2;
  bool _isRtw;
  IsConsolidated(bool exceptRestr2, bool isRtw) : _exceptRestr2(exceptRestr2), _isRtw(isRtw) {}
  bool operator()(const RoutingRestriction* r) const
  {
    if (_exceptRestr2 && r->restriction() == ROUTING_RESTRICTION_2)
      return false;
    if (_isRtw &&
        (r->restriction() == RTW_ROUTING_RESTRICTION_8 || r->restriction() == RTW_ROUTING_RESTRICTION_9))
      return false;
    return std::find(getConsolidatedRests().cbegin(),
                     getConsolidatedRests().cend(),
                     r->restriction()) != getConsolidatedRests().cend();
  }
};

struct RestLess
{
  bool operator()(const RoutingRestriction* l, const RoutingRestriction* r) const
  {
    if (l->restriction() < r->restriction())
      return true;
    if (r->restriction() < l->restriction())
      return false;
    if (l->market1() < r->market1())
      return true;
    if (r->market1() < l->market1())
      return false;
    if (l->market2() < r->market2())
      return true;
    if (r->market2() < l->market2())
      return false;
    if (l->negViaAppl() < r->negViaAppl())
      return true;
    if (r->negViaAppl() < l->negViaAppl())
      return false;
    return false;
  }
} restLess;
}

static Logger
logger("atseintl.Common.FareDisplayResponseUtil");

FareDisplayResponseUtil::FareDisplayResponseUtil() {}

FareDisplayResponseUtil::~FareDisplayResponseUtil() {}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::buildRTGDisplay()
// -------------------------------------------------------------------------
bool
FareDisplayResponseUtil::buildRTGDisplay(FareDisplayTrx& trx, const PaxTypeFare& paxTypeFare)
{
  if (displayAnyRoutingForRD(trx, paxTypeFare.routingNumber(), paxTypeFare.globalDirection()))
    return true;

  // FBR may have constructed info, but isConstructed bit is unset
  bool hasConstructed = paxTypeFare.hasConstructedRouting();

  if (hasConstructed)
  {
    trx.response() << "**** CONSTRUCTED ROUTING ";
  }
  else
  {
    trx.response() << "PUBLISHED RTG ";
  }

  // Origin, destination and carrier
  TravelSeg* tvlSeg = paxTypeFare.fareMarket()->travelSeg().front();
  trx.response() << tvlSeg->boardMultiCity() << DASH << tvlSeg->offMultiCity() << SLASH
                 << paxTypeFare.carrier();

  // Get the RoutingInfo
  if (paxTypeFare.fareDisplayInfo() == nullptr)
  {
    LOG4CXX_WARN(logger, "No FareDisplayInfo for routing number: " << paxTypeFare.routingNumber());
    return false;
  }

  const FareDisplayInfo* fareDisplayInfo(paxTypeFare.fareDisplayInfo());
  // Get the RoutingInfo
  string rtgSeq;
  if (!fareDisplayInfo->routingSequence().empty())
    rtgSeq = fareDisplayInfo->routingSequence();
  else
    rtgSeq = paxTypeFare.routingNumber();

  RoutingInfoMapConstIter uniqueRtgMapIter = trx.fdResponse()->uniqueRoutingMap().find(rtgSeq);
  if (uniqueRtgMapIter == trx.fdResponse()->uniqueRoutingMap().end())
  {
    LOG4CXX_WARN(logger, "No unique map for: " << fareDisplayInfo->routingSequence());
    return false;
  }
  RoutingInfo* routingInfo = (*uniqueRtgMapIter).second;
  if (routingInfo == nullptr)
  {
    LOG4CXX_WARN(logger, "Empty routingInfo for: " << fareDisplayInfo->routingSequence());
    return false;
  }

  // Routing number
  if (!hasConstructed && !paxTypeFare.routingNumber().empty() &&
      paxTypeFare.routingNumber() != MILEAGE_ROUTING)
  {
    trx.response() << routingNumberToStringFormat(paxTypeFare.routingNumber(), std::ios::left);
    trx.response() << SLASH << "TAR-";

    trx.response().setf(std::ios::left, std::ios::adjustfield);
    trx.response().setf(std::ios::fixed, std::ios::floatfield);
    trx.response() << std::setfill(' ') << std::setw(7) << routingInfo->tcrRoutingTariffCode();
  }
  else
  {
    trx.response() << FIVE_BLANKS;
  }

  // Effective date
  trx.response() << " EF-";
  if (paxTypeFare.effectiveDate().isValid())
  {
    trx.response() << paxTypeFare.effectiveDate().dateToString(DDMMMYY, "");
  }
  else
  {
    trx.response() << SIX_BLANKS;
  }

  // Discontinue date
  trx.response() << " DIS-";
  if (paxTypeFare.expirationDate().isValid())
  {
    trx.response() << paxTypeFare.expirationDate().dateToString(DDMMMYY, "") << std::endl;
  }
  else
  {
    trx.response() << INDEF << std::endl;
  }

  // Specific to constructed routes
  if (hasConstructed)
    addConstructedInfo(trx, paxTypeFare, routingInfo);

  // Skip line
  trx.response() << SINGLE_BLANK << std::endl;

  addGlobalDescription(trx, paxTypeFare.globalDirection(), false);

  if (isIncomplete(*routingInfo))
  {
    displayIncomplete(trx, true);
    return true;
  }
  addRTGMPM(trx, paxTypeFare, routingInfo);

  displayMileageMsg(trx, routingInfo);
  RoutingRestrictions rests;
  if (routingInfo->restrictions() != nullptr && !routingInfo->restrictions()->empty())
  {
    transform(routingInfo->restrictions()->begin(),
              routingInfo->restrictions()->end(),
              back_inserter(rests),
              restrictionRetriever);
  }

  const Routing* base(routingInfo->routing());
  const Routing* origAddOn(routingInfo->origAddOnRouting());
  const Routing* destAddOn(routingInfo->destAddOnRouting());
  Not3nor17 except(hasConstructed);

  if (base != nullptr && !base->rests().empty())
  {
    std::vector<RoutingRestriction*> rests;
    remove_copy_if(base->rests().begin(), base->rests().end(), back_inserter(rests), not1(except));
    addRestrictions(trx, rests, false, false);
  }
  if (origAddOn != nullptr && !origAddOn->rests().empty())
  {
    std::vector<RoutingRestriction*> rests;
    remove_copy_if(
        origAddOn->rests().begin(), origAddOn->rests().end(), back_inserter(rests), not1(except));
    addRestrictions(trx, rests, false, false);
  }
  if (destAddOn != nullptr && !destAddOn->rests().empty())
  {
    std::vector<RoutingRestriction*> rests;
    remove_copy_if(
        destAddOn->rests().begin(), destAddOn->rests().end(), back_inserter(rests), not1(except));
    addRestrictions(trx, rests, false, false);
  }

  displayTPD(trx, *routingInfo);
  displayPSR(trx, *routingInfo);

  if (hasConstructed)
  {
    displayConstructed(trx, paxTypeFare, routingInfo, false);
  }
  else
  {
    if (routingInfo != nullptr && routingInfo->mapInfo() != nullptr)
    {
      if (!isNonstop(trx, routingInfo->mapInfo()->routeStrings()))
        displayDRV(trx, routingInfo->routing(), paxTypeFare, false);

      displayMapDirectionalityInfo(trx, *routingInfo);

      if (rests.empty() || !isNonstop(trx, routingInfo->mapInfo()->routeStrings()))
        addRouteString(trx, routingInfo->mapInfo()->routeStrings(), false, true, false);
    }
    else
    {
      LOG4CXX_INFO(logger, "Unable to get route string");
    }
  }

  return true;
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::displayAnyRoutingForRD()
// -------------------------------------------------------------------------
bool
FareDisplayResponseUtil::displayAnyRoutingForRD(FareDisplayTrx& trx,
                                                const std::string& rtg,
                                                const GlobalDirection& gld)
{
  if (rtg != "SEVN" && rtg != "EIGH")
    return false;

  std::string gd;
  globalDirectionToStr(gd, gld);
  std::string rtgSeq = "ANY";

  trx.response() << rtgSeq;
  if (rtg == "SEVN")
  {
    if (trx.isDomestic() || trx.isForeignDomestic())
    {
      if (gd.empty())
        trx.response() << " VALID ROUTING APPLIES-77777\n";
      else
        trx.response() << " VALID " << gd << " ROUTING APPLIES-77777\n";
    }
    else
    {
      if (gd.empty())
        trx.response() << " VALID ROUTING OR MPM VIA ANY GLOBAL DIR APPLIES-77777\n";
      else
        trx.response() << " VALID " << gd << " ROUTING OR MPM APPLIES-77777\n";
    }
  }
  else
  {
    //  88888
    if (trx.isDomestic() || trx.isForeignDomestic())
    {
      if (gd.empty())
        trx.response() << " VALID ROUTING APPLIES-88888\n";
      else
        trx.response() << " VALID " << gd << " ROUTING APPLIES-88888\n";
    }
    else
    {
      if (gd.empty())
        trx.response() << " VALID ROUTING EXCEPT MPM APPLIES-88888\n";
      else
        trx.response() << " VALID " << gd << " ROUTING EXCEPT MPM APPLIES-88888\n";
    }
  }
  return true;
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addConstructedInfo()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addConstructedInfo(FareDisplayTrx& trx,
                                            const PaxTypeFare& paxTypeFare,
                                            const RoutingInfo* info)
{
  if (info == nullptr)
  {
    LOG4CXX_ERROR(logger, "RoutingInfo object is null in RoutingSection::addConstructionInfo()");
    return;
  }
  LocCode gateway1, gateway2;
  FareClassCode origAddonFareClass, destAddonFareClass;
  TariffNumber origAddonTariff, destAddonTariff;
  TariffCode origAddonTariffCode, destAddonTariffCode;
  if (paxTypeFare.isReversed())
  {
    origAddonFareClass = paxTypeFare.destAddonFareClass();
    destAddonFareClass = paxTypeFare.origAddonFareClass();
    gateway1 = paxTypeFare.gateway2();
    gateway2 = paxTypeFare.gateway1();
    // addon routings are already in correct order
    if (const Routing* origAddon = info->origAddOnRouting())
    {
      origAddonTariff = origAddon->routingTariff();
      origAddonTariffCode = info->tcrAddonTariff2Code();
    }
    else
    {
      origAddonTariff = info->routingTariff();
      origAddonTariffCode = info->tcrRoutingTariffCode();
    }
    if (const Routing* destAddon = info->destAddOnRouting())
    {
      destAddonTariff = destAddon->routingTariff();
      destAddonTariffCode = info->tcrAddonTariff1Code();
    }
    else
    {
      destAddonTariff = info->routingTariff();
      destAddonTariffCode = info->tcrRoutingTariffCode();
    }
  }
  else
  {
    origAddonFareClass = paxTypeFare.origAddonFareClass();
    destAddonFareClass = paxTypeFare.destAddonFareClass();
    gateway1 = paxTypeFare.gateway1();
    gateway2 = paxTypeFare.gateway2();
    // addon routings are already in correct order
    if (const Routing* origAddon = info->origAddOnRouting())
    {
      origAddonTariff = origAddon->routingTariff();
      origAddonTariffCode = info->tcrAddonTariff1Code();
    }
    else
    {
      origAddonTariff = info->routingTariff();
      origAddonTariffCode = info->tcrRoutingTariffCode();
    }
    if (const Routing* destAddon = info->destAddOnRouting())
    {
      destAddonTariff = destAddon->routingTariff();
      destAddonTariffCode = info->tcrAddonTariff2Code();
    }
    else
    {
      destAddonTariff = info->routingTariff();
      destAddonTariffCode = info->tcrRoutingTariffCode();
    }
  }

  if (!origAddonFareClass.empty())
  {
    trx.response() << "**ADDON ORG ";
    trx.response().setf(std::ios::right, std::ios::adjustfield);
    trx.response() << std::setw(8) << std::setfill(' ') << trx.boardMultiCity();
    trx.response() << setw(7) << setfill(' ') << origAddonTariffCode << SLASH << left << setw(4)
                   << setfill(' ') << origAddonTariff;
    if (info->origAddOnRouting() == nullptr)
      trx.response() << " MILEAGE   MPM\n";
    else if (info->origAddOnRouting()->rmaps().empty())
    {
      trx.response() << " NO MAP    ROUTING ";
      trx.response().setf(std::ios::left, std::ios::adjustfield);
      trx.response() << info->origAddOnRouting()->routing() << '\n';
    }
    else
    {
      trx.response() << " PUBLISHED ROUTING ";
      trx.response().setf(std::ios::left, std::ios::adjustfield);
      trx.response() << info->origAddOnRouting()->routing() << '\n';
    }
  }

  trx.response() << "**PUBLISHED ";
  trx.response().setf(std::ios::right, std::ios::adjustfield);
  trx.response() << std::setw(4) << std::setfill(' ') << gateway1 << DASH << gateway2;
  trx.response() << setw(7) << setfill(' ') << info->tcrRoutingTariffCode() << SLASH << left
                 << setw(4) << setfill(' ') << info->routingTariff();

  if (info->routing() == nullptr)
    trx.response() << " MILEAGE   MPM\n";
  else if (info->routing()->rmaps().empty())
  {
    trx.response() << " NO MAP    ROUTING ";
    trx.response().setf(std::ios::left, std::ios::adjustfield);
    trx.response() << info->routing()->routing() << '\n';
  }
  else
  {
    trx.response() << " PUBLISHED ROUTING ";
    trx.response().setf(std::ios::left, std::ios::adjustfield);
    trx.response() << info->routing()->routing() << '\n';
  }

  if (!destAddonFareClass.empty())
  {
    trx.response() << "**ADDON DST ";
    trx.response().setf(std::ios::right, std::ios::adjustfield);
    trx.response() << std::setw(8) << std::setfill(' ') << trx.offMultiCity();
    trx.response() << setw(7) << setfill(' ') << destAddonTariffCode << SLASH << left << setw(4)
                   << setfill(' ') << destAddonTariff;

    if (info->destAddOnRouting() == nullptr)
      trx.response() << " MILEAGE   MPM\n";
    else if (info->destAddOnRouting()->rmaps().empty())
    {
      trx.response() << " NO MAP    ROUTING ";
      trx.response().setf(std::ios::left, std::ios::adjustfield);
      trx.response() << info->destAddOnRouting()->routing() << '\n';
    }
    else
    {
      trx.response() << " PUBLISHED ROUTING ";
      trx.response().setf(std::ios::left, std::ios::adjustfield);
      trx.response() << info->destAddOnRouting()->routing() << '\n';
    }
  }
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addGlobalDescription()
// -------------------------------------------------------------------------
bool
FareDisplayResponseUtil::addGlobalDescription(FareDisplayTrx& trx,
                                              GlobalDirection global,
                                              bool inLine)
{
  // Find the Global Direction description
  const GlobalDir* globalDir = trx.dataHandle().getGlobalDir(global, trx.travelDate());

  if (!inLine)
    trx.response() << "\n";

  if (globalDir == nullptr)
    return false;

  if (inLine)
    trx.response() << SINGLE_BLANK;
  ostringstream line;
  line << SLASH << globalDir->description() << SLASH;
  const string& str(line.str());
  const size_t maxLen(inLine ? MAX_RTE_LEN_FQ_RTG : MAX_LINE_LENGTH);
  const size_t lineLimit(maxLen - GLOB_DIR_MARGIN);
  if (str.size() <= maxLen)
  {
    trx.response() << str;
    if (str.size() > lineLimit)
    {
      trx.response() << "\n";
      if (inLine)
        trx.response() << SIX_BLANKS;
    }
  }
  else
  {
    size_t pos(str.rfind(' ', maxLen));
    if (pos == string::npos)
      pos = maxLen - 1;
    trx.response() << str.substr(0, pos) << "\n";
    if (inLine)
      trx.response() << STRING_INDENT;
    trx.response() << str.substr(pos + 1);
    if (str.substr(pos + 1).size() > lineLimit)
    {
      trx.response() << "\n";
      if (inLine)
        trx.response() << SIX_BLANKS;
    }
  }
  return true;
}

bool
FareDisplayResponseUtil::isNonstop(const FareDisplayTrx& trx, const RoutingMapStrings* strings)
{
  return strings->size() == 1 && (*strings)[0] == trx.boardMultiCity() + "-" + trx.offMultiCity();
}

bool
FareDisplayResponseUtil::all17(const RoutingRestrictions& rests)
{
  return find_if(rests.begin(), rests.end(), not1(is17)) == rests.end();
}

bool
FareDisplayResponseUtil::isCTRW(FareDisplayTrx& trx, GlobalDirection gd, const RoutingInfo& info)
{
  if (gd != GlobalDirection::CT && gd != GlobalDirection::RW)
    return false;
  if (info.mileageInfo() != nullptr && info.mileageInfo()->totalApplicableMPM() == 0)
    return true;
  if (info.mapInfo() != nullptr && !isNonstop(trx, info.mapInfo()->routeStrings()))
    return false;
  if (info.routing() != nullptr && !info.routing()->rests().empty() && !all17(info.routing()->rests()))
    return false;
  if (info.origAddOnRouting() != nullptr && !info.origAddOnRouting()->rests().empty() &&
      !all17(info.origAddOnRouting()->rests()))
    return false;
  if (info.destAddOnRouting() != nullptr && !info.destAddOnRouting()->rests().empty() &&
      !all17(info.destAddOnRouting()->rests()))
    return false;
  return true;
}

bool
FareDisplayResponseUtil::isIncomplete(const RoutingInfo& info)
{
  return info.mileageInfo() == nullptr && (info.mapInfo() == nullptr || info.mapInfo()->routeStrings() == nullptr ||
                                     info.mapInfo()->routeStrings()->empty()) &&
         (info.routing() == nullptr || info.routing()->rests().empty()) &&
         (info.origAddOnRouting() == nullptr || info.origAddOnRouting()->rests().empty()) &&
         (info.destAddOnRouting() == nullptr || info.destAddOnRouting()->rests().empty());
}

void
FareDisplayResponseUtil::displayIncomplete(FareDisplayTrx& trx, bool indent)
{
  if (indent)
    trx.response() << SIX_BLANKS;
  trx.response() << " NONSTOP - INCOMPLETE ROUTE MAP ON FILE\n";
}

// -------------------------------------------------------------------
// FareDisplayResponseUtil::addRTGMPM()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addRTGMPM(FareDisplayTrx& trx,
                                   const PaxTypeFare& paxTypeFare,
                                   const RoutingInfo* info)
{
  // Add MPM and mileage
  if (info != nullptr && info->mileageInfo() != nullptr)
  {
    const MileageInfo* mileageInfo = info->mileageInfo();
    trx.response() << " MPM ";
    if (!trx.getOptions()->isRtw() || mileageInfo->totalApplicableMPM())
      trx.response() << mileageInfo->totalApplicableMPM();
    trx.response() << "\n";
  }
  else
    trx.response() << "\n";
}

void
FareDisplayResponseUtil::displayMileageMsg(FareDisplayTrx& trx, const RoutingInfo* info)
{
  if (info != nullptr && info->mileageInfo() != nullptr)
  {
    trx.response() << "MILEAGE SYSTEM APPLIES BETWEEN ORIGIN AND DESTINATION\n";
  }
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addRestrictions()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addRestrictions(FareDisplayTrx& trx,
                                         const RoutingRestrictions& restrictions,
                                         bool indent,
                                         bool fdDisplay)
{
  if (restrictions.empty())
    return;

  bool useAndForRest2 = false;
  // if all restrictions are 2, and any of them is negative, display in
  // separate lines
  RoutingRestrictionsConstIter itr = restrictions.begin();
  RoutingRestrictionsConstIter end = restrictions.end();
  for (; itr != end; ++itr)
  {
    if ((*itr)->restriction() == ROUTING_RESTRICTION_2 && (*itr)->negViaAppl() != PERMITTED &&
        (*itr)->negViaAppl() != REQUIRED)
    {
      useAndForRest2 = true;
      break;
    }
  }

  typedef std::vector<RoutingRestriction*>::iterator RoutingRestrictionsIter;
  bool isRTWorCT = RestrictionsText::isRtwOrCt(trx);

  std::vector<RoutingRestriction*> tempRests(restrictions);
  RoutingRestrictionsIter consolidatedEnd = stable_partition(
      tempRests.begin(), tempRests.end(), IsConsolidated(useAndForRest2, isRTWorCT));
  stable_sort(tempRests.begin(), consolidatedEnd, restLess);
  RoutingRestrictionsIter i(tempRests.begin()), e;

  // Handler
  RestrictionsText restText(trx.response());

  while (i != consolidatedEnd)
  {
    e = upper_bound(i, consolidatedEnd, *i, restLess);

    switch (atoi((*i)->restriction().c_str()))
    {
    case 1:
      restText.restriction1(i, e, indent);
      break;
    case 2:
      restText.restriction2(i, e, indent);
      break;
    case 5:
      restText.restriction5(i, e, indent);
      break;
    case 7:
      restText.restriction7(i, e, indent);
      break;

    case 10:
      restText.restriction10(i, e, indent);
      break;
    case 17:
      restText.restriction17(i, e, indent);
      break;
    case 21:
      restText.restriction21(i, e, indent);
      break;
    default:
      LOG4CXX_ERROR(logger, "Unhandled restriction: " << (*i)->restriction());
    }
    if (!indent && fdDisplay)
      indent = true;
    i = e;
  }

  RoutingRestrictionsConstIter restIter = consolidatedEnd;
  RoutingRestrictionsConstIter restIterEnd = tempRests.end();

  for (; restIter != restIterEnd; restIter++)
  {
    const RoutingRestriction* routingRestriction = (*restIter);
    LOG4CXX_DEBUG(logger, "Restriction number: " << routingRestriction->restriction());

    switch (atoi(routingRestriction->restriction().c_str()))
    {
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
    case 6:
      restText.restriction6(*routingRestriction, indent);
      break;
    case 8:
      if (!fallback::fallbackFMRbyte58(&trx) || isRTWorCT)
      {
        restText.restriction8FMR(trx, *routingRestriction, indent);
      }
      break;
    case 9:
      restText.restriction9FMR(trx, indent);
      break;
    case 11:
      restText.restriction11(*routingRestriction, indent);
      break;
    case 12:
      if (!isRTWorCT)
        restText.restriction12(*routingRestriction, indent);
      break;
    case 16:
      if (!isRTWorCT)
        restText.restriction16(*routingRestriction, indent);
      break;
    default:
      LOG4CXX_ERROR(logger, "Unhandled restriction: " << routingRestriction->restriction());
    }
    if (!indent && fdDisplay)
      indent = true;
  }
}

void
FareDisplayResponseUtil::displayTPD(FareDisplayTrx& trx, const RoutingInfo& rtgInfo)
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

    trx.response() << STRING_INDENT << "TICKETED POINT DEDUCTION OF " << tpd.tpmDeduction()
                   << " MILES APPLIES\n";
    trx.response() << STRING_INDENT << "WHEN TRAVEL IS VIA ";

    std::vector<TpdPsrViaGeoLoc*>::const_iterator geoLocItr = tpd.viaGeoLocs().begin();
    std::vector<TpdPsrViaGeoLoc*>::const_iterator geoLocEnd = tpd.viaGeoLocs().end();

    size_t charCount(25);
    vector<TpdPsrViaGeoLoc*> stopovers;

    for (; geoLocItr != geoLocEnd; ++geoLocItr)
    {
      if ((*geoLocItr)->stopoverNotAllowed() == STPOVRNOTALWD_YES)
        stopovers.push_back(*geoLocItr);

      if ((charCount + 8) > MAX_LINE_LENGTH)
      {
        trx.response() << std::endl;
        trx.response() << STRING_INDENT;
        charCount = STRING_INDENT_LENGTH;
      }

      if ((**geoLocItr).relationalInd() == VIAGEOLOCREL_AND)
      {
        trx.response() << AND;
        charCount += AND_LEN;
      }
      else if ((**geoLocItr).relationalInd() == VIAGEOLOCREL_OR)
      {
        trx.response() << OR;
        charCount += OR_LEN;
      }
      else if ((**geoLocItr).relationalInd() == VIAGEOLOCREL_ANDOR)
      {
        trx.response() << ANDOR;
        charCount += ANDOR_LEN;
      }
      else if (geoLocItr != tpd.viaGeoLocs().begin())
      { // if relational indicator is empty and it is not the first via loc
        // it means the beginning of new set - sets are combined with OR
        trx.response() << OR;
        charCount += OR_LEN;
      }

      if ((charCount + 15) > MAX_LINE_LENGTH)
      {
        trx.response() << std::endl;
        trx.response() << STRING_INDENT;
        charCount = STRING_INDENT_LENGTH;
      }
      else if ((**geoLocItr).loc().locType() == LOCTYPE_AREA)
      {
        trx.response() << "AREA ";
        charCount += 5;
      }
      else if ((**geoLocItr).loc().locType() == LOCTYPE_SUBAREA)
      {
        trx.response() << "SUB AREA ";
        charCount += 9;
      }
      else if ((**geoLocItr).loc().locType() == LOCTYPE_STATE)
      {
        trx.response() << "STATE/PROVINCE ";
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

      if ((charCount + desc.size()) > MAX_LINE_LENGTH)
      {
        trx.response() << std::endl;
        trx.response() << STRING_INDENT;
        charCount = STRING_INDENT_LENGTH;
      }

      trx.response() << desc;
      charCount += desc.size();
    } // End of iner for

    trx.response() << std::endl;

    if (!stopovers.empty())
    {
      trx.response() << STRING_INDENT << "PROVIDED NO STOPOVER AT ";
      charCount = 24;
      vector<TpdPsrViaGeoLoc*>::const_iterator stpovrItr(stopovers.begin());
      vector<TpdPsrViaGeoLoc*>::const_iterator stpovrEnd(stopovers.end());
      for (; stpovrItr != stpovrEnd; ++stpovrItr)
      {
        if ((charCount + 8) > MAX_LINE_LENGTH)
        {
          trx.response() << std::endl;
          trx.response() << STRING_INDENT;
          charCount = STRING_INDENT_LENGTH;
        }
        if ((*stpovrItr)->relationalInd() == VIAGEOLOCREL_AND)
        {
          trx.response() << AND;
          charCount += AND_LEN;
        }
        else if ((*stpovrItr)->relationalInd() == VIAGEOLOCREL_OR)
        {
          trx.response() << OR;
          charCount += OR_LEN;
        }
        else if ((*stpovrItr)->relationalInd() == VIAGEOLOCREL_ANDOR)
        {
          trx.response() << ANDOR;
          charCount += ANDOR_LEN;
        }
        else if (stpovrItr != stopovers.begin())
        { // if relational indicator is empty and it is not the first via loc
          // it means the beginning of new set - sets are combined with OR
          trx.response() << OR;
          charCount += OR_LEN;
        }
        if ((charCount + 3) > MAX_LINE_LENGTH)
        {
          trx.response() << std::endl;
          trx.response() << STRING_INDENT;
          charCount = STRING_INDENT_LENGTH;
        }
        trx.response() << (*stpovrItr)->loc().loc();
      }
      trx.response() << endl;
    }

    if (tpd.thisCarrierRestr() == 'Y')
    {
      trx.response() << STRING_INDENT << "PROVIDED ALL TRAVEL IS VIA " << tpd.carrier()
                     << std::endl;
    }
  } // End of for ( itrpsrB != itrpsrE)
}

void
FareDisplayResponseUtil::displayPSR(FareDisplayTrx& trx, const RoutingInfo& rtgInfo)
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
    std::vector<std::string> responseVect;

    LOG4CXX_DEBUG(logger,
                  "PSR: carrier " << psr.carrier() << " area 1 " << psr.area1() << " area 2 "
                                  << psr.area2() << " seqNo " << psr.seqNo())

    if (psr.viaGeoLocs().empty())
      continue;

    trx.response() << STRING_INDENT << "SPECIFIED ROUTING PERMITTED AS FOLLOWS" << std::endl;

    trx.response() << STRING_INDENT;

    std::vector<TpdPsrViaGeoLoc*>::const_iterator geoLocItr = psr.viaGeoLocs().begin();
    std::vector<TpdPsrViaGeoLoc*>::const_iterator geoLocEnd = psr.viaGeoLocs().end();

    const size_t lineLength = 59; // TODO What is 59?
    size_t charCount = STRING_INDENT_LENGTH;

    bool revOrigAndDest = LocUtil::isInLoc(
        *trx.origin(), psr.loc1().locType(), psr.loc1().loc(), Vendor::SABRE, MANUAL);
    if (revOrigAndDest)
      responseVect.push_back(trx.origin()->loc());
    else
      responseVect.push_back(trx.destination()->loc());
    responseVect.push_back(DASH);

    for (; geoLocItr != geoLocEnd; ++geoLocItr)
    {
      if ((**geoLocItr).relationalInd() == VIAGEOLOCREL_AND)
        responseVect.push_back(DASH); //" AND ";
      else if ((**geoLocItr).relationalInd() == VIAGEOLOCREL_OR)
        responseVect.push_back(SLASH); //" OR ";
      else if ((**geoLocItr).relationalInd() == VIAGEOLOCREL_ANDOR)
        responseVect.push_back(DASH); //" AND/OR ";

      if ((**geoLocItr).loc().locType() == LOCTYPE_ZONE)
        responseVect.push_back("ZONE ");
      else if ((**geoLocItr).loc().locType() == LOCTYPE_AREA)
        responseVect.push_back("AREA ");
      else if ((**geoLocItr).loc().locType() == LOCTYPE_SUBAREA)
        responseVect.push_back("SUB AREA ");
      else if ((**geoLocItr).loc().locType() == LOCTYPE_STATE)
        responseVect.push_back("STATE/PROVINCE ");

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

      responseVect.push_back(desc);
    } // End of iner for

    responseVect.push_back(DASH);
    if (revOrigAndDest)
      responseVect.push_back(trx.destination()->loc());
    else
      responseVect.push_back(trx.origin()->loc());

    if (revOrigAndDest)
    {
      std::vector<std::string>::const_iterator rvIter = responseVect.begin();
      for (; rvIter != responseVect.end(); rvIter++)
      {
        charCount += (*rvIter).size();
        if (charCount > lineLength)
        {
          trx.response() << std::endl;
          trx.response() << STRING_INDENT;
          charCount = STRING_INDENT_LENGTH;
        }
        trx.response() << *rvIter;
      }
    }
    else
    {
      std::vector<std::string>::reverse_iterator rrvIter = responseVect.rbegin();
      for (; rrvIter != responseVect.rend(); rrvIter++)
      {
        charCount += (*rrvIter).size();
        if (charCount > lineLength)
        {
          trx.response() << std::endl;
          trx.response() << STRING_INDENT;
          charCount = STRING_INDENT_LENGTH;
        }
        trx.response() << *rrvIter;
      }
    }
    trx.response() << std::endl;

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
FareDisplayResponseUtil::displayConstructed(FareDisplayTrx& trx,
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

        if (!fq)
          displayMapDirectionalityInfo(trx, *info);
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
    string orig, dest;
    RoutingRestrictions restsBeforeStrings;
    Routings drvs;

    copyRestriction17and3(origAddOn, drvs, restsBeforeStrings, rests);
    copyRestriction17and3(base, drvs, restsBeforeStrings, rests);
    copyRestriction17and3(destAddOn, drvs, restsBeforeStrings, rests);

    origAdnDesfForRtgWithoutMap(returnRoutingType(origAddOn),
                                returnRoutingType(base),
                                returnRoutingType(destAddOn),
                                orig,
                                dest,
                                fareMarket);

    displayConstrMsg(trx, orig, dest, fq);

    origAdnDesfForRtg(returnRoutingType(origAddOn),
                      returnRoutingType(base),
                      returnRoutingType(destAddOn),
                      orig,
                      dest,
                      fareMarket);

    addRestrictions(trx, restsBeforeStrings, false, fq);
    // display routing restrictions limitation message only if not all existing parts
    // of constructed routing were included in routing strings extranction (some are mileage)

    if (drvs.size() < countAddon(origAddOn, destAddOn))
      displayConstrMsg(trx, orig, dest, fq);

    addRestrictions(trx, rests, false, fq);
    if (info->mapInfo() != nullptr)
    {
      if (!isNonstop(trx, info->mapInfo()->routeStrings()))
        displayDRV(trx, drvs, paxTypeFare, fq);

      if (!fq)
        displayMapDirectionalityInfo(trx, *info);
      addRouteString(trx, info->mapInfo()->routeStrings(), fq, true, fq);
    }
  }
}

uint16_t
FareDisplayResponseUtil::countAddon(const Routing* origAddOn, const Routing* destAddOn)
{
  uint16_t parts(1); // assume base is always present - if it is null it means it is SITA mileage
  if (origAddOn != nullptr)
    ++parts;
  if (destAddOn != nullptr)
    ++parts;
  return parts;
}

void
FareDisplayResponseUtil::copyRestriction17and3(const Routing* routing,
                                               Routings& drvs,
                                               RoutingRestrictions& restsBeforeStrings,
                                               RoutingRestrictions& rests)
{
  if (routing != nullptr && !routing->rmaps().empty())
  {
    if (!routing->rests().empty())
    {
      remove_copy_if(
          routing->rests().begin(), routing->rests().end(), back_inserter(rests), not1(rest3or17));
    }
    drvs.push_back(routing);
  }
  else if (routing != nullptr && !routing->rests().empty())
  {
    remove_copy_if(routing->rests().begin(),
                   routing->rests().end(),
                   back_inserter(restsBeforeStrings),
                   not1(rest3or17));
  }
}

void
FareDisplayResponseUtil::origAdnDesfForRtg(AddonAndPublishType origin,
                                           AddonAndPublishType base,
                                           AddonAndPublishType dest,
                                           std::string& originString,
                                           std::string& destString,
                                           const FareMarket& fareMarket)
{
  originString.clear();
  destString.clear();
  if (origin == AptWithMap)
  {
    originString = fareMarket.boardMultiCity();
    destString = gateway(translateNation(fareMarket.origin()->nation()));
  }

  if (base == AptWithMap)
  {
    if (origin == AptMileage || origin == AptWithoutMap)
      originString = gateway(translateNation(fareMarket.origin()->nation()));
    if (origin == AptEmpty)
      originString = fareMarket.boardMultiCity();

    if (dest == AptMileage || dest == AptWithoutMap)
      destString = gateway(translateNation(fareMarket.destination()->nation()));
    if (dest == AptEmpty)
      destString = fareMarket.offMultiCity();
  }

  if (dest == AptWithMap)
  {
    if ((origin != AptWithMap) && (base != AptWithMap))
      originString = gateway(translateNation(fareMarket.destination()->nation()));

    destString = fareMarket.offMultiCity();
  }
}

void
FareDisplayResponseUtil::origAdnDesfForRtgWithoutMap(AddonAndPublishType origin,
                                                     AddonAndPublishType base,
                                                     AddonAndPublishType dest,
                                                     std::string& originString,
                                                     std::string& destString,
                                                     const FareMarket& fareMarket)
{
  // don't display any message for this case
  if (origin == AptWithoutMap && base == AptWithMap && dest == AptWithoutMap)
    return;

  if (origin == AptWithoutMap && base == AptWithMap)
  {
    originString = "THE ORIGIN";
    destString = gateway(translateNation(fareMarket.origin()->nation()));
  }
  else if (base == AptWithMap && dest == AptWithoutMap)
  {
    originString = gateway(translateNation(fareMarket.destination()->nation()));
    destString = "THE DESTINATION";
  }
}

FareDisplayResponseUtil::AddonAndPublishType
FareDisplayResponseUtil::returnRoutingType(const Routing* routing)
{
  if (nullptr == routing)
    return AptEmpty;
  else if (!routing->rmaps().empty() && routing->routing() != MILEAGE_ROUTING)
    return AptWithMap;
  else if (routing->rmaps().empty() && routing->routing() != MILEAGE_ROUTING)
    return AptWithoutMap;
  else
    return AptMileage;
}

void
FareDisplayResponseUtil::displayConstrMsg(FareDisplayTrx& trx,
                                          const string& loc1,
                                          const string& loc2,
                                          bool fq)
{
  if (loc1.empty() || loc2.empty())
    return;

  if (fq)
    trx.response() << STRING_INDENT;
  trx.response() << "ROUTING RESTRICTIONS INDICATED BELOW APPLY BETWEEN\n";
  if (fq)
    trx.response() << STRING_INDENT;
  trx.response() << loc1 << " AND " << loc2 << " ONLY\n";
}

bool
FareDisplayResponseUtil::displayDRV(FareDisplayTrx& trx,
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
      trx.response() << STRING_INDENT;
    const CarrierPreference* cxrPref(
        trx.dataHandle().getCarrierPreference(paxTypeFare.carrier(), trx.travelDate()));
    if (cxrPref != nullptr && cxrPref->noApplydrvexceptus() == YES)
      trx.response() << "DOM ROUTE VALIDATION APPLIES WITHIN US ONLY\n";
    else
      trx.response() << "DOM ROUTE VALIDATION APPLIES WITHIN ORIG/DEST COUNTRIES\n";
    return true;
  }
  return false;
}

void
FareDisplayResponseUtil::displayDRV(FareDisplayTrx& trx,
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

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addRouteString()
// -------------------------------------------------------------------------
bool
FareDisplayResponseUtil::addRouteString(FareDisplayTrx& trx,
                                        const RoutingMapStrings* strings,
                                        bool indent,
                                        bool useLineNumbers,
                                        bool fdDisplay)
{
  uint32_t lineNum = 1;
  uint32_t noLineNum = 0;
  size_t lines(0);
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
        trx.response() << SIX_BLANKS;
      if (useLineNumbers)
      {
        trx.response().setf(std::ios::right, std::ios::adjustfield);
        trx.response().setf(std::ios::fixed, std::ios::floatfield);
        trx.response() << std::setw(2) << std::setfill(' ') << lineNum++ << ". ";
      }
      else
        trx.response() << TWO_BLANKS;
      trx.response() << *iter << '\n';
    }
    if (!indent && fdDisplay)
      indent = true;
    if (fdDisplay && ++lines >= MAX_FD_RTG_LINES)
    {
      if (indent)
        trx.response() << STRING_INDENT;
      trx.response() << TOO_MANY_LINES << std::endl;
      return true;
    }
  }
  return true;
}

string
FareDisplayResponseUtil::translateNation(const NationCode& loc)
{
  if (loc == NATION_US || loc == NATION_CA)
    return US_CA;
  return loc;
}

string
FareDisplayResponseUtil::gateway(const string& loc)
{
  return loc + " GATEWAY";
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::splitLine()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::splitLine(
    FareDisplayTrx& trx, const std::string& theLine, bool inLine, uint32_t lineNum, bool fdDisplay)
{
  const size_t maxLen(fdDisplay ? MAX_RTE_LEN_FQ : MAX_RTE_LEN_RD);
  size_t length = theLine.size();
  size_t currLen(maxLen);
  std::string partOf = theLine;
  bool first = true;
  while (currLen < length)
  {
    if (fdDisplay && (!inLine || !first))
      trx.response() << SIX_BLANKS;
    if (lineNum != 0 && first)
    {
      trx.response().setf(std::ios::right, std::ios::adjustfield);
      trx.response().setf(std::ios::fixed, std::ios::floatfield);
      trx.response() << std::setw(2) << std::setfill(' ') << lineNum << ". ";
    }
    else
      trx.response() << FOUR_BLANKS;

    currLen = partOf.find_last_of(DELIMS, maxLen - 1);
    if (currLen == string::npos)
      currLen = maxLen;
    else
      ++currLen;

    LOG4CXX_DEBUG(logger, "to display: " << partOf.substr(0, currLen));
    trx.response() << partOf.substr(0, currLen) << std::endl;

    partOf = partOf.substr(currLen);
    length = partOf.size();
    if (first == true)
      first = false;
  }

  // The last part
  if (partOf.size() > 0)
  {
    if (fdDisplay)
      trx.response() << SIX_BLANKS;
    trx.response() << FOUR_BLANKS << partOf << std::endl;
  }
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::buildFBDisplay()
// -------------------------------------------------------------------------
bool
FareDisplayResponseUtil::buildFBDisplay(const CatNumber& cat,
                                        FareDisplayTrx& fareDisplayTrx,
                                        FareDisplayInfo* fareDisplayInfo,
                                        PaxTypeFare& paxTypeFare)
{
  const FBDisplay& fb = fareDisplayInfo->fbDisplay();

  FBCategoryRuleRecord* catRuleRecord = fb.getRuleRecordData(uint16_t(cat));
  FBCategoryRuleRecord* baseFareRuleRecord = fb.getBaseFareRuleRecordData(uint16_t(cat));

  if (!catRuleRecord)
  {
    return false;
  }

  std::string record2Type = EMPTY_STRING();
  if (paxTypeFare.isFareByRule() && (baseFareRuleRecord))
    addFBRLine(fareDisplayTrx);

  if (catRuleRecord->isAllEmpty())
  {
    addNoRuleDataLine(fareDisplayTrx);
    addBlankLine(fareDisplayTrx);
  }
  else
  {
    std::string record2Type = EMPTY_STRING();
    TariffCode ruleTariffCode;
    // ---------------------------------
    // CAT10
    // ---------------------------------
    if (catRuleRecord->hasCombinabilityRecord2Info())
    {
      const CombinabilityRuleInfo* cRR2 = catRuleRecord->combinabilityRecord2Info();

      getRuleTariffDescription(fareDisplayTrx,
                               paxTypeFare.fareTariff(),
                               cRR2->vendorCode(),
                               cRR2->carrierCode(),
                               cRR2->tariffNumber(),
                               ruleTariffCode);
      addFareRuleRec2(fareDisplayTrx);
      addLine3(fareDisplayTrx, catRuleRecord, record2Type, ruleTariffCode);
      addLine4(fareDisplayTrx, catRuleRecord, record2Type);
      addLine5(fareDisplayTrx, catRuleRecord, record2Type);
      addLine6(fareDisplayTrx, catRuleRecord, record2Type);
      addLine7(fareDisplayTrx, catRuleRecord, record2Type);
      addLine8(fareDisplayTrx);
      addRecord2StringTable(fareDisplayTrx, cRR2->categoryRuleItemInfoSet());
    }

    // ---------------------------------
    // CAT25
    // ---------------------------------
    if (catRuleRecord->hasFareByRuleRecord2Info())
    {
      record2Type = EMPTY_STRING();

      const FareByRuleCtrlInfo* fBRR2 = catRuleRecord->fareByRuleRecord2Info();

      getRuleTariffDescription(fareDisplayTrx,
                               paxTypeFare.fareTariff(),
                               fBRR2->vendorCode(),
                               fBRR2->carrierCode(),
                               fBRR2->tariffNumber(),
                               ruleTariffCode);
      addFareRuleRec2(fareDisplayTrx);
      addLine3(fareDisplayTrx, catRuleRecord, record2Type, ruleTariffCode);
      addLine4(fareDisplayTrx, catRuleRecord, record2Type);
      addLine5(fareDisplayTrx, catRuleRecord, record2Type);
      addLine6(fareDisplayTrx, catRuleRecord, record2Type);
      addLine7(fareDisplayTrx, catRuleRecord, record2Type);
      addLine8(fareDisplayTrx);
      addRecord2StringTable(fareDisplayTrx, fBRR2->categoryRuleItemInfoSet());
      return true;
    }

    // ---------------------------------
    // All other categories
    // ---------------------------------
    if (catRuleRecord->hasFareRuleRecord2Info())
    {
      record2Type = FB_FARE_RULE_RECORD_2;

      // Get Tariff Description.
      const FareRuleRecord2Info* fRR2 = catRuleRecord->fareRuleRecord2Info();
      getRuleTariffDescription(fareDisplayTrx,
                               paxTypeFare.fareTariff(),
                               fRR2->vendorCode(),
                               fRR2->carrierCode(),
                               fRR2->tariffNumber(),
                               ruleTariffCode);
      addFareRuleRec2(fareDisplayTrx);
      addLine3(fareDisplayTrx, catRuleRecord, record2Type, ruleTariffCode);
      addLine4(fareDisplayTrx, catRuleRecord, record2Type);
      addLine5(fareDisplayTrx, catRuleRecord, record2Type);
      addLine6(fareDisplayTrx, catRuleRecord, record2Type);
      addLine7(fareDisplayTrx, catRuleRecord, record2Type);
      addLine8(fareDisplayTrx);
      addRecord2StringTable(fareDisplayTrx, fRR2->categoryRuleItemInfoSet());
    }

    if (catRuleRecord->hasFootNoteRecord2Info())
    {
      record2Type = FB_FOOTNOTE_RECORD_2;

      // Get Tariff Description.
      const FootNoteRecord2Info* fNR2 = catRuleRecord->footNoteRecord2Info();
      getRuleTariffDescription(fareDisplayTrx,
                               paxTypeFare.fareTariff(),
                               fNR2->vendorCode(),
                               fNR2->carrierCode(),
                               fNR2->tariffNumber(),
                               ruleTariffCode);
      addFootnoteRec2(fareDisplayTrx);
      addLine3(fareDisplayTrx, catRuleRecord, record2Type, ruleTariffCode);
      addLine4(fareDisplayTrx, catRuleRecord, record2Type);
      addLine5(fareDisplayTrx, catRuleRecord, record2Type);
      addLine6(fareDisplayTrx, catRuleRecord, record2Type);
      addLine7(fareDisplayTrx, catRuleRecord, record2Type);
      addLine8(fareDisplayTrx);
      addRecord2StringTable(fareDisplayTrx, fNR2->categoryRuleItemInfoSet());
    }

    if (catRuleRecord->hasGeneralRuleRecord2Info())
    {
      record2Type = FB_GENERAL_RULE_RECORD_2;

      // Get Tariff Description.
      const GeneralRuleRecord2Info* fGR2 = catRuleRecord->generalRuleRecord2Info();
      getRuleTariffDescription(fareDisplayTrx, paxTypeFare.fareTariff(), fGR2, ruleTariffCode);
      addGeneralRuleRec2(fareDisplayTrx);
      addLine3(fareDisplayTrx, catRuleRecord, record2Type, ruleTariffCode);
      addLine4(fareDisplayTrx, catRuleRecord, record2Type);
      addLine5(fareDisplayTrx, catRuleRecord, record2Type);
      addLine6(fareDisplayTrx, catRuleRecord, record2Type);
      addLine7(fareDisplayTrx, catRuleRecord, record2Type);
      addLine8(fareDisplayTrx);
      addRecord2StringTable(fareDisplayTrx, fGR2->categoryRuleItemInfoSet());
    }
  } // end else
  if (!baseFareRuleRecord)
    return true;
  else
  {
    addBaseFareLine(fareDisplayTrx);
    if (baseFareRuleRecord->isAllEmpty())
    {
      addNoRuleDataLine(fareDisplayTrx);
      addBlankLine(fareDisplayTrx);
    }
    else
    {
      buildBaseFareDisplay(fareDisplayTrx, paxTypeFare, baseFareRuleRecord, record2Type);
    }
  }
  return true;
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addBaseFareLine()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addBaseFareLine(FareDisplayTrx& fareDisplayTrx)
{
  fareDisplayTrx.response() << "*** BASE FARE ***" << std::endl;
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addFBRLine()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addFBRLine(FareDisplayTrx& fareDisplayTrx)
{
  fareDisplayTrx.response() << "*** FARE BY RULE ***" << std::endl;
}

//---------------------------------------------------------------------------
// FareDisplayResponseUtil::addBlankLine()
//---------------------------------------------------------------------------
void
FareDisplayResponseUtil::addBlankLine(FareDisplayTrx& fareDisplayTrx)
{
  fareDisplayTrx.response() << SPACE << std::endl;
}

//---------------------------------------------------------------------------
// FareDisplayResponseUtil::spaces()
//---------------------------------------------------------------------------
void
FareDisplayResponseUtil::spaces(FareDisplayTrx& fareDisplayTrx, const int16_t& numSpaces)
{
  fareDisplayTrx.response() << std::setfill(' ') << std::setw(numSpaces) << SPACE;
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addNoRuleDataLine()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addNoRuleDataLine(FareDisplayTrx& fareDisplayTrx)
{
  fareDisplayTrx.response() << "NO DATA FOUND. DEFAULT APPLIES" << std::endl;
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addGeneralRuleRec2()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addGeneralRuleRec2(FareDisplayTrx& fareDisplayTrx)
{
  fareDisplayTrx.response() << "GENERAL FARE RULE RECORD 2" << std::endl;
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addFareRuleRec2()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addFareRuleRec2(FareDisplayTrx& fareDisplayTrx)
{
  fareDisplayTrx.response() << "FARE RULE RECORD 2" << std::endl;
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addFootnoteRec2()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addFootnoteRec2(FareDisplayTrx& fareDisplayTrx)
{
  fareDisplayTrx.response() << "FOOTNOTE RECORD 2" << std::endl;
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addLine3()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addLine3(FareDisplayTrx& fareDisplayTrx,
                                  FBCategoryRuleRecord* fbRuleRecord,
                                  std::string record2Type,
                                  std::string ruleTariffCode)
{
  ostringstream* oss = &fareDisplayTrx.response();
  *oss << "VND-";

  // CAT 10
  const CombinabilityRuleInfo* cRR2 = fbRuleRecord->combinabilityRecord2Info();
  if (cRR2)
  {
    oss->setf(std::ios::left, std::ios::adjustfield);
    *oss << std::setfill(' ') << std::setw(4) << cRR2->vendorCode();
    spaces(fareDisplayTrx, 3);
    *oss << "TAR-";
    *oss << cRR2->tariffNumber() << SLASH;
    *oss << ruleTariffCode << std::endl;
    return;
  }
  // CAT25
  const FareByRuleCtrlInfo* fBRR2 = fbRuleRecord->fareByRuleRecord2Info();
  if (fBRR2)
  {
    oss->setf(std::ios::left, std::ios::adjustfield);
    *oss << std::setfill(' ') << std::setw(4) << fBRR2->vendorCode();
    spaces(fareDisplayTrx, 3);
    *oss << "TAR-";
    *oss << fBRR2->tariffNumber() << SLASH;
    *oss << ruleTariffCode << std::endl;
    return;
  }

  // Other categories
  if (record2Type == FB_FARE_RULE_RECORD_2)
  {
    const FareRuleRecord2Info* fareRuleRecord2Info = fbRuleRecord->fareRuleRecord2Info();
    oss->setf(std::ios::left, std::ios::adjustfield);
    *oss << std::setfill(' ') << std::setw(4) << fareRuleRecord2Info->vendorCode();
    spaces(fareDisplayTrx, 3);
    *oss << "TAR-";
    *oss << fareRuleRecord2Info->tariffNumber() << SLASH;
    *oss << ruleTariffCode << std::endl;
  }
  else if (record2Type == FB_FOOTNOTE_RECORD_2)
  {
    const FootNoteRecord2Info* footNoteRecord2Info = fbRuleRecord->footNoteRecord2Info();
    oss->setf(std::ios::left, std::ios::adjustfield);
    *oss << std::setfill(' ') << std::setw(4) << footNoteRecord2Info->vendorCode();
    spaces(fareDisplayTrx, 3);
    *oss << "TAR-";
    *oss << footNoteRecord2Info->tariffNumber() << SLASH;
    *oss << ruleTariffCode << std::endl;
  }
  else if (record2Type == FB_GENERAL_RULE_RECORD_2)
  {
    const GeneralRuleRecord2Info* generalRuleRecord2Info = fbRuleRecord->generalRuleRecord2Info();
    oss->setf(std::ios::left, std::ios::adjustfield);
    *oss << std::setfill(' ') << std::setw(4) << generalRuleRecord2Info->vendorCode();
    spaces(fareDisplayTrx, 3);
    *oss << "TAR-";
    *oss << generalRuleRecord2Info->tariffNumber() << SLASH;
    *oss << ruleTariffCode << std::endl;
  }
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addLine4()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addLine4(FareDisplayTrx& fareDisplayTrx,
                                  FBCategoryRuleRecord* fbRuleRecord,
                                  std::string record2Type)
{
  ostringstream* oss = &fareDisplayTrx.response();
  *oss << "CXR-";

  // CAT 10
  const CombinabilityRuleInfo* cRR2 = fbRuleRecord->combinabilityRecord2Info();
  if (cRR2)
  {
    oss->setf(std::ios::left, std::ios::adjustfield);
    *oss << std::setfill(' ') << std::setw(3) << cRR2->carrierCode();
    spaces(fareDisplayTrx, 2);
    *oss << "RUL-";
    *oss << std::setfill(' ') << std::setw(4) << cRR2->ruleNumber();
    spaces(fareDisplayTrx, 3);
    *oss << "CAT-";
    oss->setf(std::ios::right, std::ios::adjustfield);
    *oss << std::setfill('0') << std::setw(2) << cRR2->categoryNumber();
    spaces(fareDisplayTrx, 3);
    oss->setf(std::ios::left, std::ios::adjustfield);
    *oss << "SEQ-";
    oss->setf(std::ios::right, std::ios::adjustfield);
    *oss << std::setfill('0') << std::setw(9) << cRR2->sequenceNumber() << std::endl;
    return;
  }

  // CAT25
  const FareByRuleCtrlInfo* fBRR2 = fbRuleRecord->fareByRuleRecord2Info();
  if (fBRR2)
  {
    oss->setf(std::ios::left, std::ios::adjustfield);
    *oss << std::setfill(' ') << std::setw(3) << fBRR2->carrierCode();
    spaces(fareDisplayTrx, 2);
    *oss << "RUL-";
    *oss << std::setfill(' ') << std::setw(4) << fBRR2->ruleNumber();
    spaces(fareDisplayTrx, 3);
    *oss << "CAT-";
    oss->setf(std::ios::right, std::ios::adjustfield);
    *oss << std::setfill('0') << std::setw(2) << fBRR2->categoryNumber();
    spaces(fareDisplayTrx, 3);
    *oss << "SEQ-";
    oss->setf(std::ios::right, std::ios::adjustfield);
    *oss << std::setfill('0') << std::setw(9) << fBRR2->sequenceNumber() << std::endl;
    return;
  }
  // Other categories
  if (record2Type == FB_FARE_RULE_RECORD_2)
  {
    const FareRuleRecord2Info* fareRuleRecord2Info = fbRuleRecord->fareRuleRecord2Info();
    oss->setf(std::ios::left, std::ios::adjustfield);
    *oss << std::setfill(' ') << std::setw(3) << fareRuleRecord2Info->carrierCode();
    spaces(fareDisplayTrx, 2);
    *oss << "RUL-";
    *oss << std::setfill(' ') << std::setw(4) << fareRuleRecord2Info->ruleNumber();
    spaces(fareDisplayTrx, 3);
    *oss << "CAT-";
    oss->setf(std::ios::right, std::ios::adjustfield);
    *oss << std::setfill('0') << std::setw(2) << fareRuleRecord2Info->categoryNumber();
    spaces(fareDisplayTrx, 3);
    *oss << "SEQ-";
    oss->setf(std::ios::right, std::ios::adjustfield);
    *oss << std::setfill('0') << std::setw(9) << fareRuleRecord2Info->sequenceNumber() << std::endl;
  }
  else if (record2Type == FB_FOOTNOTE_RECORD_2)
  {
    const FootNoteRecord2Info* footNoteRecord2Info = fbRuleRecord->footNoteRecord2Info();
    oss->setf(std::ios::left, std::ios::adjustfield);
    *oss << std::setfill(' ') << std::setw(3) << footNoteRecord2Info->carrierCode();
    spaces(fareDisplayTrx, 2);
    *oss << "RUL-";
    *oss << std::setfill(' ') << std::setw(4) << footNoteRecord2Info->ruleNumber();
    spaces(fareDisplayTrx, 3);
    *oss << "CAT-";
    oss->setf(std::ios::right, std::ios::adjustfield);
    *oss << std::setfill('0') << std::setw(2) << footNoteRecord2Info->categoryNumber();
    spaces(fareDisplayTrx, 3);
    *oss << "SEQ-";
    oss->setf(std::ios::right, std::ios::adjustfield);
    *oss << std::setfill('0') << std::setw(9) << footNoteRecord2Info->sequenceNumber() << std::endl;
  }
  else if (record2Type == FB_GENERAL_RULE_RECORD_2)
  {
    const GeneralRuleRecord2Info* generalRuleRecord2Info = fbRuleRecord->generalRuleRecord2Info();
    oss->setf(std::ios::left, std::ios::adjustfield);
    *oss << std::setfill(' ') << std::setw(3) << generalRuleRecord2Info->carrierCode();
    spaces(fareDisplayTrx, 2);
    *oss << "RUL-";
    *oss << std::setfill(' ') << std::setw(4) << generalRuleRecord2Info->ruleNumber();
    spaces(fareDisplayTrx, 3);
    *oss << "CAT-";
    oss->setf(std::ios::right, std::ios::adjustfield);
    *oss << std::setfill('0') << std::setw(2) << generalRuleRecord2Info->categoryNumber();
    spaces(fareDisplayTrx, 3);
    *oss << "SEQ-";
    oss->setf(std::ios::right, std::ios::adjustfield);
    *oss << std::setfill('0') << std::setw(9) << generalRuleRecord2Info->sequenceNumber()
         << std::endl;
  }
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addLine5()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addLine5(FareDisplayTrx& fareDisplayTrx,
                                  FBCategoryRuleRecord* fbRuleRecord,
                                  std::string record2Type)
{
  ostringstream* oss = &fareDisplayTrx.response();
  oss->setf(std::ios::left, std::ios::adjustfield);
  *oss << "EFF DATE- ";

  // CAT 10
  const CombinabilityRuleInfo* cRR2 = fbRuleRecord->combinabilityRecord2Info();
  if (cRR2)
  {
    *oss << cRR2->effDate().dateToString(DDMMMYY, "");
    spaces(fareDisplayTrx, 8);
    *oss << "DISC DATE- ";
    if (cRR2->discDate().isInfinity())
      *oss << INFINITY_TXT << std::endl;
    else
      *oss << cRR2->discDate().dateToString(DDMMMYY, "") << std::endl;
    return;
  }

  // CAT25
  const FareByRuleCtrlInfo* fBRR2 = fbRuleRecord->fareByRuleRecord2Info();
  if (fBRR2)
  {
    *oss << fBRR2->effDate().dateToString(DDMMMYY, "");
    spaces(fareDisplayTrx, 8);
    *oss << "DISC DATE- ";
    if (fBRR2->discDate().isInfinity())
      *oss << INFINITY_TXT << std::endl;
    else
      *oss << fBRR2->discDate().dateToString(DDMMMYY, "") << std::endl;
    return;
  }

  // Other categories
  if (record2Type == FB_FARE_RULE_RECORD_2)
  {
    const FareRuleRecord2Info* fareRuleRecord2Info = fbRuleRecord->fareRuleRecord2Info();
    *oss << fareRuleRecord2Info->effDate().dateToString(DDMMMYY, "");
    spaces(fareDisplayTrx, 8);
    *oss << "DISC DATE- ";
    if (fareRuleRecord2Info->discDate().isInfinity())
      *oss << INFINITY_TXT << std::endl;
    else
      *oss << fareRuleRecord2Info->discDate().dateToString(DDMMMYY, "") << std::endl;
  }
  else if (record2Type == FB_FOOTNOTE_RECORD_2)
  {
    const FootNoteRecord2Info* footNoteRecord2Info = fbRuleRecord->footNoteRecord2Info();
    *oss << footNoteRecord2Info->effDate().dateToString(DDMMMYY, "");
    spaces(fareDisplayTrx, 8);
    *oss << "DISC DATE- ";
    if (footNoteRecord2Info->discDate().isInfinity())
      *oss << INFINITY_TXT << std::endl;
    else
      *oss << footNoteRecord2Info->discDate().dateToString(DDMMMYY, "") << std::endl;
  }
  else if (record2Type == FB_GENERAL_RULE_RECORD_2)
  {
    const GeneralRuleRecord2Info* generalRuleRecord2Info = fbRuleRecord->generalRuleRecord2Info();
    *oss << generalRuleRecord2Info->effDate().dateToString(DDMMMYY, "");
    spaces(fareDisplayTrx, 8);
    *oss << "DISC DATE- ";
    if (generalRuleRecord2Info->discDate().isInfinity())
      *oss << INFINITY_TXT << std::endl;
    else
      *oss << generalRuleRecord2Info->discDate().dateToString(DDMMMYY, "") << std::endl;
  }
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addLine6()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addLine6(FareDisplayTrx& fareDisplayTrx,
                                  FBCategoryRuleRecord* fbRuleRecord,
                                  std::string record2Type)
{
  ostringstream* oss = &fareDisplayTrx.response();
  oss->setf(std::ios::left, std::ios::adjustfield);
  *oss << "LOC 1-" << std::endl;

  // CAT 10
  const CombinabilityRuleInfo* cRR2 = fbRuleRecord->combinabilityRecord2Info();
  if (cRR2)
  {
    *oss << cRR2->loc1().locType() << SLASH << SPACE;
    *oss << std::setfill(' ') << std::setw(4) << cRR2->loc1().loc();
    spaces(fareDisplayTrx, 12);
    *oss << "LOC 2-";
    *oss << cRR2->loc2().locType() << SLASH << SPACE;
    *oss << std::setfill(' ') << std::setw(4) << cRR2->loc2().loc();
    spaces(fareDisplayTrx, 4);
    *oss << "FARE CLASS-" << cRR2->fareClass() << std::endl;
    return;
  }

  // CAT25
  const FareByRuleCtrlInfo* fBRR2 = fbRuleRecord->fareByRuleRecord2Info();
  if (fBRR2)
  {
    *oss << fBRR2->loc1().locType() << SLASH << SPACE;
    *oss << std::setfill(' ') << std::setw(4) << fBRR2->loc1().loc();
    spaces(fareDisplayTrx, 12);
    *oss << "LOC 2-";
    *oss << fBRR2->loc2().locType() << SLASH << SPACE;
    *oss << std::setfill(' ') << std::setw(4) << fBRR2->loc2().loc();
    spaces(fareDisplayTrx, 4);
    // cat25 fare class in rec3, so nothing to display for this rec2
    *oss << "FARE CLASS-" << std::endl;
    return;
  }

  // Other categories
  if (record2Type == FB_FARE_RULE_RECORD_2)
  {
    const FareRuleRecord2Info* fareRuleRecord2Info = fbRuleRecord->fareRuleRecord2Info();
    *oss << fareRuleRecord2Info->loc1().locType() << SLASH << SPACE;
    *oss << std::setfill(' ') << std::setw(4) << fareRuleRecord2Info->loc1().loc();
    spaces(fareDisplayTrx, 12);
    *oss << "LOC 2-";
    *oss << fareRuleRecord2Info->loc2().locType() << SLASH << SPACE;
    *oss << std::setfill(' ') << std::setw(4) << fareRuleRecord2Info->loc2().loc();
    spaces(fareDisplayTrx, 4);
    *oss << "FARE CLASS-" << fareRuleRecord2Info->fareClass() << std::endl;
  }
  else if (record2Type == FB_FOOTNOTE_RECORD_2)
  {
    const FootNoteRecord2Info* footNoteRecord2Info = fbRuleRecord->footNoteRecord2Info();
    *oss << footNoteRecord2Info->loc1().locType() << SLASH << SPACE;
    *oss << std::setfill(' ') << std::setw(4) << footNoteRecord2Info->loc1().loc();
    spaces(fareDisplayTrx, 12);
    *oss << "LOC 2-";
    *oss << footNoteRecord2Info->loc2().locType() << SLASH << SPACE;
    *oss << std::setfill(' ') << std::setw(4) << footNoteRecord2Info->loc2().loc();
    spaces(fareDisplayTrx, 4);
    *oss << "FARE CLASS-" << footNoteRecord2Info->fareClass() << std::endl;
  }
  else if (record2Type == FB_GENERAL_RULE_RECORD_2)
  {
    const GeneralRuleRecord2Info* generalRuleRecord2Info = fbRuleRecord->generalRuleRecord2Info();
    *oss << generalRuleRecord2Info->loc1().locType() << SLASH << SPACE;
    *oss << std::setfill(' ') << std::setw(4) << generalRuleRecord2Info->loc1().loc();
    spaces(fareDisplayTrx, 12);
    *oss << "LOC 2-";
    *oss << generalRuleRecord2Info->loc2().locType() << SLASH << SPACE;
    *oss << std::setfill(' ') << std::setw(4) << generalRuleRecord2Info->loc2().loc();
    *oss << "FARE CLASS-" << generalRuleRecord2Info->fareClass() << std::endl;
  }
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addLine7()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addLine7(FareDisplayTrx& fareDisplayTrx,
                                  FBCategoryRuleRecord* fbRuleRecord,
                                  std::string record2Type)
{
  ostringstream* oss = &fareDisplayTrx.response();
  oss->setf(std::ios::left, std::ios::adjustfield);
  *oss << "NO APPL-";

  // CAT 10
  const CombinabilityRuleInfo* cRR2 = fbRuleRecord->combinabilityRecord2Info();
  if (cRR2)
  {
    *oss << std::setfill(' ') << std::setw(8) << cRR2->applInd() << std::endl;
    return;
  }

  // CAT25
  const FareByRuleCtrlInfo* fBRR2 = fbRuleRecord->fareByRuleRecord2Info();
  if (fBRR2)
  {
    *oss << std::setfill(' ') << std::setw(8) << fBRR2->applInd() << std::endl;
    return;
  }

  // Other categories
  if (record2Type == FB_FARE_RULE_RECORD_2)
  {
    const FareRuleRecord2Info* fareRuleRecord2Info = fbRuleRecord->fareRuleRecord2Info();
    *oss << std::setfill(' ') << std::setw(8) << fareRuleRecord2Info->applInd() << std::endl;
  }
  else if (record2Type == FB_FOOTNOTE_RECORD_2)
  {
    const FootNoteRecord2Info* footNoteRecord2Info = fbRuleRecord->footNoteRecord2Info();
    *oss << std::setfill(' ') << std::setw(8) << footNoteRecord2Info->applInd() << std::endl;
  }
  else if (record2Type == FB_GENERAL_RULE_RECORD_2)
  {
    const GeneralRuleRecord2Info* generalRuleRecord2Info = fbRuleRecord->generalRuleRecord2Info();
    *oss << std::setfill(' ') << std::setw(8) << generalRuleRecord2Info->applInd() << std::endl;
  }
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addLine8()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addLine8(FareDisplayTrx& fareDisplayTrx)
{
  addBlankLine(fareDisplayTrx);
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addDataTableInfo()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addDataTableInfo(FareDisplayTrx& fareDisplayTrx, size_t _noOfDataTables)
{
  ostringstream* oss = &fareDisplayTrx.response();
  oss->setf(std::ios::left, std::ios::adjustfield);
  *oss << "NUMBER OF DATA TABLES- ";
  *oss << _noOfDataTables << std::endl;
}


void
FareDisplayResponseUtil::buildBaseFareDisplay(FareDisplayTrx& fareDisplayTrx,
                                              PaxTypeFare& paxTypeFare,
                                              FBCategoryRuleRecord* baseFareRuleRecord,
                                              std::string record2Type)

{
  TariffCode ruleTariffCode;
  const PaxTypeFare& basePaxTypeFare = *(paxTypeFare.fareWithoutBase());
  if (baseFareRuleRecord->hasFareRuleRecord2Info())
  {
    record2Type = FB_FARE_RULE_RECORD_2;

    // Get Tariff Description.
    const FareRuleRecord2Info* bfRR2 = baseFareRuleRecord->fareRuleRecord2Info();

    getRuleTariffDescription(fareDisplayTrx,
                             basePaxTypeFare.fareTariff(),
                             bfRR2->vendorCode(),
                             bfRR2->carrierCode(),
                             bfRR2->tariffNumber(),
                             ruleTariffCode);
    addFareRuleRec2(fareDisplayTrx);
    addLine3(fareDisplayTrx, baseFareRuleRecord, record2Type, ruleTariffCode);
    addLine4(fareDisplayTrx, baseFareRuleRecord, record2Type);
    addLine5(fareDisplayTrx, baseFareRuleRecord, record2Type);
    addLine6(fareDisplayTrx, baseFareRuleRecord, record2Type);
    addLine7(fareDisplayTrx, baseFareRuleRecord, record2Type);
    addLine8(fareDisplayTrx);
    addRecord2StringTable(fareDisplayTrx, bfRR2->categoryRuleItemInfoSet());
  }

  if (baseFareRuleRecord->hasFootNoteRecord2Info())
  {
    record2Type = FB_FOOTNOTE_RECORD_2;

    // Get Tariff Description.
    const FootNoteRecord2Info* bfNR2 = baseFareRuleRecord->footNoteRecord2Info();

    getRuleTariffDescription(fareDisplayTrx,
                             basePaxTypeFare.fareTariff(),
                             bfNR2->vendorCode(),
                             bfNR2->carrierCode(),
                             bfNR2->tariffNumber(),
                             ruleTariffCode);
    addFootnoteRec2(fareDisplayTrx);
    addLine3(fareDisplayTrx, baseFareRuleRecord, record2Type, ruleTariffCode);
    addLine4(fareDisplayTrx, baseFareRuleRecord, record2Type);
    addLine5(fareDisplayTrx, baseFareRuleRecord, record2Type);
    addLine6(fareDisplayTrx, baseFareRuleRecord, record2Type);
    addLine7(fareDisplayTrx, baseFareRuleRecord, record2Type);
    addLine8(fareDisplayTrx);
    addRecord2StringTable(fareDisplayTrx, bfNR2->categoryRuleItemInfoSet());
  }

  if (baseFareRuleRecord->hasGeneralRuleRecord2Info())
  {
    record2Type = FB_GENERAL_RULE_RECORD_2;

    // Get Tariff Description.
    const GeneralRuleRecord2Info* bfGR2 = baseFareRuleRecord->generalRuleRecord2Info();

    getRuleTariffDescription(fareDisplayTrx, basePaxTypeFare.fareTariff(), bfGR2, ruleTariffCode);
    addGeneralRuleRec2(fareDisplayTrx);
    addLine3(fareDisplayTrx, baseFareRuleRecord, record2Type, ruleTariffCode);
    addLine4(fareDisplayTrx, baseFareRuleRecord, record2Type);
    addLine5(fareDisplayTrx, baseFareRuleRecord, record2Type);
    addLine6(fareDisplayTrx, baseFareRuleRecord, record2Type);
    addLine7(fareDisplayTrx, baseFareRuleRecord, record2Type);
    addLine8(fareDisplayTrx);
    addRecord2StringTable(fareDisplayTrx, bfGR2->categoryRuleItemInfoSet());
  }
}

// ------------------------------------------
// Gets Tariff Description for general rule
// ------------------------------------------
bool
FareDisplayResponseUtil::getRuleTariffDescription(const FareDisplayTrx& trx,
                                                  const TariffNumber& fareTariff,
                                                  const GeneralRuleRecord2Info* fGR2,
                                                  TariffCode& ruleTariffCode)
{ // lint !e578
  ruleTariffCode = EMPTY_STRING();

  // -------------------------------------------------
  // Get the record scope type INTERNATION / DOMESTIC
  // -------------------------------------------------
  const RecordScope crossRefType = getCrossRefType(trx);

  // -----------------------------------------------
  // Retrieve TariffCrossRef Info
  // -----------------------------------------------
  const std::vector<TariffCrossRefInfo*> tariffDescInfoList =
      getTariffXRefByGenRuleTariff(getDataHandle(trx),
                                   fGR2->vendorCode(),
                                   fGR2->carrierCode(),
                                   crossRefType,
                                   fGR2->tariffNumber(),
                                   trx.travelDate());

  const TariffCrossRefInfo* tariffCrossRefInfoPtr =
      matchByFareTariff(tariffDescInfoList, fareTariff);

  if (!tariffCrossRefInfoPtr)
    return false;

  ruleTariffCode = tariffCrossRefInfoPtr->governingTariffCode();
  return true;
}

// -------------------------------------------------
// Get the record scope type INTERNATION / DOMESTIC
// -------------------------------------------------
RecordScope
FareDisplayResponseUtil::getCrossRefType(const FareDisplayTrx& trx)
{
  GeoTravelType recordScope;
  if (!trx.fareMarket().empty())
  {
    recordScope = trx.fareMarket().front()->geoTravelType();
  }
  else
  {
    recordScope = trx.itin().front()->geoTravelType();
  }

  return ((recordScope == GeoTravelType::International || recordScope == GeoTravelType::ForeignDomestic) ? INTERNATIONAL
                                                                           : DOMESTIC);
}

const TariffCrossRefInfo*
FareDisplayResponseUtil::matchByFareTariff(
    const std::vector<TariffCrossRefInfo*>& tariffDescInfoList, const TariffNumber& fareTariff)
{
  if (tariffDescInfoList.empty())
    return nullptr;

  // -----------------------------------------------------------
  // Find appropriate Tariff Description based on Fare Tariff No
  // -----------------------------------------------------------
  if (tariffDescInfoList.size() > 1)
  {
    std::vector<TariffCrossRefInfo*>::const_iterator iB = tariffDescInfoList.begin();
    std::vector<TariffCrossRefInfo*>::const_iterator iE = tariffDescInfoList.end();
    for (; iB != iE; iB++)
    {
      if (fareTariff == (*iB)->fareTariff())
        return (*iB);
    }
    return nullptr;
  }

  // -----------------------------------
  // Only 1 Tariff Cross Ref info found
  // -----------------------------------
  const TariffCrossRefInfo* tariffCrossRefInfo = tariffDescInfoList.front();

  if (fareTariff == tariffCrossRefInfo->fareTariff())
    return tariffCrossRefInfo;

  return nullptr;
}

bool
FareDisplayResponseUtil::getRuleTariffDescription(const FareDisplayTrx& trx,
                                                  const TariffNumber& fareTariff,
                                                  const VendorCode& vendor,
                                                  const CarrierCode& carrier,
                                                  const TariffNumber& ruleTariff,
                                                  TariffCode& ruleTariffCode)
{
  ruleTariffCode = EMPTY_STRING();

  // -------------------------------------------------
  // Get the record scope type INTERNATION / DOMESTIC
  // -------------------------------------------------
  const RecordScope crossRefType = getCrossRefType(trx);

  // -----------------------------------------------
  // Retrieve TariffCrossRef Info
  // -----------------------------------------------
  const std::vector<TariffCrossRefInfo*> tariffDescInfoList = getTariffXRefByRuleTariff(
      getDataHandle(trx), vendor, carrier, crossRefType, ruleTariff, trx.travelDate());

  const TariffCrossRefInfo* tariffCrossRefInfoPtr =
      matchByFareTariff(tariffDescInfoList, fareTariff);

  if (!tariffCrossRefInfoPtr)
    return false;

  ruleTariffCode = tariffCrossRefInfoPtr->ruleTariffCode();
  return true;
}

// ------------------------
// Gets Addon Tariff Code
// ------------------------
bool
FareDisplayResponseUtil::getAddonTariffCode(const FareDisplayTrx& trx,
                                            const VendorCode& vendor,
                                            const CarrierCode& carrier,
                                            const TariffNumber& addonTariff,
                                            TariffCode& addonTariffCode)
{
  addonTariffCode = EMPTY_STRING();
  // -------------------------------------------------
  // Get the record scope type INTERNATION / DOMESTIC
  // -------------------------------------------------
  const RecordScope crossRefType = INTERNATIONAL;

  // -----------------------------------------------
  // Retrieve TariffCrossRef Info
  // -----------------------------------------------
  const std::vector<TariffCrossRefInfo*>& tariffDescInfoList =
      trx.dataHandle().getTariffXRefByAddonTariff(
          vendor, carrier, crossRefType, addonTariff, trx.travelDate());

  const TariffCrossRefInfo* tariffCrossRefInfoPtr =
      matchByAddonTariff(tariffDescInfoList, addonTariff);

  if (!tariffCrossRefInfoPtr)
    return false;

  if (tariffCrossRefInfoPtr->addonTariff1() == addonTariff)
  {
    if (!tariffCrossRefInfoPtr->addonTariff1Code().empty())
      addonTariffCode = tariffCrossRefInfoPtr->addonTariff1Code();
    else
      addonTariffCode = tariffCrossRefInfoPtr->addonTariff2Code();
  }
  else
  {
    if (!tariffCrossRefInfoPtr->addonTariff2Code().empty())
      addonTariffCode = tariffCrossRefInfoPtr->addonTariff2Code();
    else
      addonTariffCode = tariffCrossRefInfoPtr->addonTariff1Code();
  }

  return true;
}

const TariffCrossRefInfo*
FareDisplayResponseUtil::matchByAddonTariff(
    const std::vector<TariffCrossRefInfo*>& tariffDescInfoList, const TariffNumber& addonTariff)
{
  if (tariffDescInfoList.empty())
    return nullptr;

  // -----------------------------------------------------------
  // Find appropriate Tariff Code based on Addon Tariff Number
  // -----------------------------------------------------------
  if (tariffDescInfoList.size() > 1)
  {
    std::vector<TariffCrossRefInfo*>::const_iterator iB = tariffDescInfoList.begin();
    std::vector<TariffCrossRefInfo*>::const_iterator iE = tariffDescInfoList.end();
    for (; iB != iE; iB++)
    {
      if (addonTariff == (*iB)->addonTariff1())
        return (*iB);
      if (addonTariff == (*iB)->addonTariff2())
        return (*iB);
    }
    return nullptr;
  }

  // -----------------------------------
  // Only 1 Tariff Cross Ref info found
  // -----------------------------------
  const TariffCrossRefInfo* tariffCrossRefInfo = tariffDescInfoList.front();

  if (addonTariff == tariffCrossRefInfo->addonTariff1())
    return tariffCrossRefInfo;

  if (addonTariff == tariffCrossRefInfo->addonTariff2())
    return tariffCrossRefInfo;

  return nullptr;
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addMXText()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addMXText(FareDisplayInfo& fareDisplayInfo, ostringstream* oss)
{
  FBDisplay& fbDisplay = fareDisplayInfo.fbDisplay();

  *oss << fbDisplay.mxCombinabilityInfo() << std::endl;
}

// -------------------------------------------------------------------------
// FareDisplayResponseUtil::addCatText()
// -------------------------------------------------------------------------
void
FareDisplayResponseUtil::addCatText(FareDisplayTrx& fareDisplayTrx,
                                    const CatNumber& catNumber,
                                    FareDisplayInfo& fareDisplayInfo,
                                    ostringstream* oss)
{
  if (catNumber != IC_RULE_CATEGORY && catNumber != RETAILER_CATEGORY)
  {
    // Show rule text or default text
    std::map<CatNumber, std::string>::const_iterator ruleText =
        fareDisplayInfo.fbDisplay().ruleTextMap().find(catNumber);
    if ((ruleText != fareDisplayInfo.fbDisplay().ruleTextMap().end()) &&
        (ruleText->second.size() > 1))
    {
      *oss << ruleText->second << std::endl;
    }
    else
    {
      if (catNumber == TRANSFERS_RULE_CATEGORY)
      {
        *oss << "   UNLIMITED TRANSFERS PERMITTED. - SURFACE AT FARE BREAK\n";
        *oss << "   AND EMBEDDED SECTORS PERMITTED EXCEPT FOR PRICING UNITS\n";
        *oss << "   WITHIN/BETWEEN US AND CANADA - INCLUDING PR/USV1 -\n";
        *oss << "   SURFACE AT FARE BREAK IS NOT PERMITTED." << std::endl;
      }
      else if (catNumber == VOLUNTARY_CHANGES &&
               fareDisplayInfo.getCategoryApplicability(false, VOLUNTARY_CHANGES) != NO_PARAM)
      {
        const RuleCategoryDescInfo* descriptionInfo =
            fareDisplayTrx.dataHandle().getRuleCategoryDesc(catNumber);
        if (fareDisplayTrx.isERDFromSWS())
          *oss << "   REQUEST BY CATEGORY NUMBER REQUIRED FOR "
               << descriptionInfo->shortDescription() << PERIOD << std::endl;
        else
          *oss << "   ENTER RD*31 OR RD$LINE NUM$*31 FOR " << descriptionInfo->shortDescription()
               << PERIOD << std::endl;
      }

      else if (catNumber != COMBINABILITY_RULE_CATEGORY)
      {
        *oss << std::setfill(' ') << std::setw(3) << SPACE;
        const RuleCategoryDescInfo* descriptionInfo =
            fareDisplayTrx.dataHandle().getRuleCategoryDesc(catNumber);
        *oss << descriptionInfo->defaultMsg() << std::endl;
      }
      else
      {
        PaxTypeFare* paxTypeFare = fareDisplayTrx.allPaxTypeFare().front();
        *oss << "   OPEN JAW/ROUND TRIP/CIRCLE TRIP AND END-ON-END\n   COMBINATIONS ARE NOT "
                "PERMITTED.";
        if (paxTypeFare && paxTypeFare->owrt() != ONE_WAY_MAYNOT_BE_DOUBLED)
        {
          *oss << " MIRROR IMAGE\n   COMBINATIONS ARE PERMITTED.";
        }
        *oss << std::endl;
      }
    }
  }
}

//---------------------------------------------------------------------------
// FareDisplayResponseUtil::getRuleTariffCode()
//---------------------------------------------------------------------------
std::string
FareDisplayResponseUtil::getRuleTariffCode(FareDisplayTrx& fareDisplayTrx, PaxTypeFare& paxTypeFare)
{
  if (!paxTypeFare.tcrRuleTariffCode().empty())
    return paxTypeFare.tcrRuleTariffCode();
  else if (paxTypeFare.tcrRuleTariff() != 0)
  {
    const TariffCode* tc = paxTypeFare.getRuleTariffCode(fareDisplayTrx);
    if (tc)
      return *tc;
  }
  return EMPTY_STRING();
}

//---------------------------------------------------------------------------
// FareDisplayResponseUtil::getFareDescription()
//---------------------------------------------------------------------------
std::string
FareDisplayResponseUtil::getFareDescription(FareDisplayTrx& fareDisplayTrx,
                                            PaxTypeFare& paxTypeFare)
{
  const FareTypeMatrix* matrix = fareDisplayTrx.dataHandle().getFareTypeMatrix(
      paxTypeFare.fcaFareType(), fareDisplayTrx.travelDate());
  if (matrix != nullptr)
    return matrix->description();
  return EMPTY_STRING();
}

//---------------------------------------------------------------------------
// FareDisplayResponseUtil::getEffDate()
//---------------------------------------------------------------------------
void
FareDisplayResponseUtil::getEffDate(FareDisplayTrx& fareDisplayTrx,
                                    PaxTypeFare& paxTypeFare,
                                    FareDisplayInfo& fareDisplayInfo,
                                    DateTime& effDate)
{
  if (fareDisplayInfo.travelInfoRD().empty())
    effDate = paxTypeFare.effectiveDate();
  else
  {
    TravelInfo* ti = fareDisplayInfo.travelInfoRD().front();

    if (ti->earliestTvlStartDate().isValid())
      effDate = ti->earliestTvlStartDate();
    else
      effDate = paxTypeFare.effectiveDate();
  }
}

void
FareDisplayResponseUtil::getDiscDate(FareDisplayTrx& fareDisplayTrx,
                                     PaxTypeFare& paxTypeFare,
                                     FareDisplayInfo& fareDisplayInfo,
                                     DateTime& discDate)
{
  if (fareDisplayInfo.travelInfoRD().empty())
  {
    if (fareDisplayTrx.dataHandle().isHistorical())
      discDate = paxTypeFare.fare()->discDate();
    else if (paxTypeFare.expirationDate().isValid())
      discDate = paxTypeFare.expirationDate();
  }
  else
  {
    TravelInfo* ti = fareDisplayInfo.travelInfoRD().front();

    if (ti->latestTvlStartDate().isValid())
      discDate = ti->latestTvlStartDate();
    else if (ti->stopDate().isValid())
      discDate = ti->stopDate();
    else if (fareDisplayTrx.dataHandle().isHistorical())
      discDate = paxTypeFare.fare()->discDate();
    else if (paxTypeFare.expirationDate().isValid())
      discDate = paxTypeFare.expirationDate();
  }
}

// -----------------------------------------------------------------------
// @MethodName  FareDisplayResponseUtil::convertFare()
//              Convert Fare from published currency to NUC
//              Then convert Fare from NUC to currency of specified Fare.
// -----------------------------------------------------------------------
void
FareDisplayResponseUtil::convertFare(FareDisplayTrx& fareDisplayTrx,
                                     PaxTypeFare& paxTypeFare,
                                     MoneyAmount& fareAmount,
                                     const CurrencyCode& displayCurrency,
                                     NUCCollectionResults& returnedNucResults)
{
  // Convert the addon fare from published currency to NUCs
  NUCCurrencyConverter nucConverter;
  Money amountNUC(0, NUC);
  Money amount(fareAmount, displayCurrency);

  NUCCollectionResults nucResults;
  nucResults.collect() = true;

  CurrencyConversionRequest curConvReq(amountNUC,
                                       amount,
                                       fareDisplayTrx.getRequest()->ticketingDT(),
                                       *(fareDisplayTrx.getRequest()),
                                       fareDisplayTrx.dataHandle(),
                                       paxTypeFare.isInternational(),
                                       CurrencyConversionRequest::FAREDISPLAY);

  bool convertRC = nucConverter.convert(curConvReq, &nucResults);
  if (convertRC)
  {
    fareAmount = amountNUC.value();
    returnedNucResults.exchangeRate() = nucResults.exchangeRate();
    returnedNucResults.exchangeRateNoDec() = nucResults.exchangeRateNoDec();
    returnedNucResults.roundingFactor() = nucResults.roundingFactor();
    returnedNucResults.roundingRule() = nucResults.roundingRule();
  }

  // Convert the NUC fare to the currency of the constructed fare
  Money amountSpecified(0, paxTypeFare.currency());
  nucResults.collect() = true;

  CurrencyConversionRequest curConvReq2(amountSpecified,
                                        amountNUC,
                                        fareDisplayTrx.getRequest()->ticketingDT(),
                                        *(fareDisplayTrx.getRequest()),
                                        fareDisplayTrx.dataHandle(),
                                        paxTypeFare.isInternational(),
                                        CurrencyConversionRequest::FAREDISPLAY,
                                        false,
                                        fareDisplayTrx.getOptions(),
                                        true);

  convertRC = nucConverter.convert(curConvReq2, &nucResults);
  if (convertRC)
  {
    fareAmount = amountSpecified.value();
  }
}

bool
FareDisplayResponseUtil::routingNumberToNumeric(const std::string& str, int16_t& rn)
{
  const char* cstr = str.c_str();
  while (*cstr && isspace(*cstr))
    ++cstr;

  while (*cstr)
  {
    if (!isdigit(*cstr))
      return false;
    ++cstr;
  }

  rn = int16_t(atoi(str.c_str()));
  return true;
}

std::string
FareDisplayResponseUtil::routingNumberToString(const std::string& src)
{
  const char* cstr = src.c_str();
  while (*cstr && (*cstr == '0' || *cstr == ' '))
    ++cstr;

  if (!(*cstr)) // src="0000"
    return "0";
  return cstr;
}

std::string
FareDisplayResponseUtil::routingNumberToStringFormat(const std::string& src,
                                                     std::ios_base::fmtflags ff)
{
  ostringstream oss;
  oss.setf(ff, std::ios::adjustfield);
  oss.setf(std::ios::fixed, std::ios::floatfield);

  const char* cstr = src.c_str();
  while (*cstr && (*cstr == '0' || *cstr == ' '))
    ++cstr;

  if (!(*cstr)) // src=="0000"
    oss << std::setfill(' ') << std::setw(4) << 0;
  else
    oss << std::setfill(' ') << std::setw(4) << cstr;

  return oss.str();
}

std::string
FareDisplayResponseUtil::routingNumberToStringFormat(uint16_t rn, std::ios_base::fmtflags ff)
{
  ostringstream oss;
  oss.setf(ff, std::ios::adjustfield);
  oss.setf(std::ios::fixed, std::ios::floatfield);
  oss << std::setfill(' ') << std::setw(4) << rn;
  return oss.str();
}

void
FareDisplayResponseUtil::displayMapDirectionalityInfo(FareDisplayTrx& trx,
                                                      const RoutingInfo& rtgInfo)
{
  if (!TrxUtil::isFullMapRoutingActivated(trx))
  {
    return;
  }

  if (!rtgInfo.routing() || rtgInfo.routing()->routing() == MILEAGE_ROUTING ||
      rtgInfo.routing()->rmaps().empty())
    return;

  switch (rtgInfo.routing()->directionalInd())
  {
  case MAPDIR_L2R:
    trx.response() << "MAP CONSTRUCTED LEFT TO RIGHT" << std::endl;
    break;
  case MAPDIR_BOTH:
    trx.response() << "MAP CONSTRUCTED LEFT TO RIGHT AND RIGHT TO LEFT" << std::endl;
    break;
  case MAPDIR_IGNOREIND:
  default:
    break;
  }
}
