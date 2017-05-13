#include "Fares/SalesRestrictionByNation.h"

#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Billing.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/SalesNationRestr.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/Diagnostic.h"
#include "Util/BranchPrediction.h"

using namespace std;

namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrActivation);

namespace
{
log4cxx::LoggerPtr
_logger(log4cxx::Logger::getLogger("atseintl.Fares.SalesRestrictionByNation"));
}

bool
SalesRestrictionByNation::isRestricted(Itin& itin, FareMarket& fareMarket, PricingTrx& trx)
{
  if (fareMarket.primarySector() == nullptr)
  {
    return false;
  }

  DCFactory* factory = DCFactory::instance();
  DiagCollector* diag = nullptr;
  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic371))
  {
    diag = factory->create(trx);
    diag->enable(Diagnostic371);

    if (diag != nullptr)
    {
      (*diag) << " \nFARE MARKET:\n"
              << " " << FareMarketUtil::getDisplayString(fareMarket) << "\n"
              << " GOVCXR/" << fareMarket.governingCarrier() << " TKTCXR/"
              << itin.ticketingCarrier() << " VALCXR/" << itin.validatingCarrier();

      std::string gd;
      globalDirectionToStr(gd, fareMarket.getGlobalDirection());
      (*diag) << " GI/" << gd << "\n"
              << " PARTITION/" << trx.billing()->partitionID() << " SALELOC/"
              << TrxUtil::saleLoc(trx)->loc() << " TKTLOC/" << TrxUtil::ticketingLoc(trx)->loc()
              << "\n";
    }
  }

  const DateTime& date = trx.adjustedTravelDate(fareMarket.primarySector()->departureDT());

  std::set<NationCode> allNations;
  allNations.insert(fareMarket.origin()->nation());

  if ((fareMarket.geoTravelType() != GeoTravelType::Domestic) && (fareMarket.geoTravelType() != GeoTravelType::ForeignDomestic))
  {
    const std::vector<TravelSeg*>& travelSegs = fareMarket.travelSeg();
    std::vector<TravelSeg*>::const_iterator i = travelSegs.begin();
    for (; i != travelSegs.end(); ++i)
    {
      allNations.insert((*i)->destination()->nation());
    }
  }

  std::set<CarrierCode> restrictedCxrs;
  std::set<NationCode>::iterator nationIter = allNations.begin();
  for (; nationIter != allNations.end(); ++nationIter)
  {
    const vector<SalesNationRestr*>& restr =
      trx.dataHandle().getSalesNationRestr(*nationIter, date);

    if (!fallback::fallbackValidatingCxrActivation(&trx))
    {
      bool isCxrRestricted = false;;
      restrictedCxrs.clear();
      for (SalesNationRestr* restriction : restr)
      {
        bool tempIsRest = anyRestriction(*restriction, itin, fareMarket, trx, restrictedCxrs);
        isCxrRestricted ^= tempIsRest;

        if (UNLIKELY(tempIsRest && diag))
        {
          LOG4CXX_DEBUG(_logger, "Matched SalesRestrictionByNation #" << restriction->seqNo());
          (*diag) << " RESULT: FAILED ITEM " << restriction->seqNo() << " FOR NATION "
            << restriction->nation() << "\n";
          displayRestrItem(*restriction, *diag);
        }

        if (isCxrRestricted && (fareMarket.validatingCarriers().empty() ||
              fareMarket.validatingCarriers().size() == restrictedCxrs.size()))
          break;
      }

      if (UNLIKELY(isCxrRestricted))
      {
        bool res = false;
        if (!restrictedCxrs.empty() && fareMarket.validatingCarriers().size() > 1)
        {
          for (const CarrierCode& cxr : restrictedCxrs)
          {
            auto it = std::find(fareMarket.validatingCarriers().begin(),
                fareMarket.validatingCarriers().end(), cxr);
            if (it != fareMarket.validatingCarriers().end())
              fareMarket.validatingCarriers().erase(it);
          }

          if (fareMarket.validatingCarriers().empty())
          {
            itin.salesNationRestr() = true; // Itin may restricted.
            res = true; // Restriction - fail the fare market
          }

          if (UNLIKELY(diag))
          {
            *diag << "VALIDATING CXR REMOVED FROM FM: ";
            for (const CarrierCode& cxr : restrictedCxrs)
              *diag << cxr << " ";
            *diag << std::endl;
          }
        }
        else
        {
          itin.salesNationRestr() = true; // Itin may restricted.
          res = true; // Restriction - fail the fare market
        }

        if (UNLIKELY(diag))
        {
          diag->flushMsg();
        }

        return res;
      }
    }
    else
    {
      vector<SalesNationRestr*>::const_iterator restriction = restr.begin();
      vector<SalesNationRestr*>::const_iterator restrEnd = restr.end();
      for (; restriction != restrEnd; ++restriction)
      {
        if (anyRestriction(**restriction, itin, fareMarket, trx, restrictedCxrs))
        {
          LOG4CXX_DEBUG(_logger, "Matched SalesRestrictionByNation #" << (*restriction)->seqNo());

          itin.salesNationRestr() = true; // Itin may restricted.

          if (UNLIKELY(diag))
          {
            (*diag) << " RESULT: FAILED ITEM " << (*restriction)->seqNo() << " FOR NATION "
              << (*restriction)->nation() << "\n";
            displayRestrItem(**restriction, *diag);

            diag->flushMsg();
          }
          return true; // Restriction - fail the fare market
        }
      }
    }
  }

  if (UNLIKELY(diag))
  {
    (*diag) << " RESULT: PASSED\n\n";

    diag->flushMsg();
  }

  return false;
}

bool
SalesRestrictionByNation::anyRestriction(const SalesNationRestr& restriction,
                                         const Itin& itin,
                                         FareMarket& fareMarket,
                                         PricingTrx& trx,
                                         std::set<CarrierCode>& restrictedCxrs)
{
  if (UNLIKELY(!matchUser(restriction.userApplType(), restriction.userAppl(), trx)))
    return false;

  GeoTravelType geoTvlType = itin.geoTravelType();

  bool isValidatingCxrRestricted = (!fallback::fallbackValidatingCxrActivation(&trx)) ?
      matchCarrier(itin, restriction.tktgCxrs(), restriction.exceptTicketingCxr(),
        fareMarket.validatingCarriers(), restrictedCxrs) :
      matchCarrier(restriction.tktgCxrs(), restriction.exceptTicketingCxr(),
          (itin.validatingCarrier().empty() ? itin.ticketingCarrier() : itin.validatingCarrier()));

  bool restricted =
      isValidatingCxrRestricted &&
      matchCarrier(restriction.govCxrs(), restriction.exceptGoverningCxr(), fareMarket.governingCarrier()) &&
      matchDirectionality(trx, restriction, itin, fareMarket) &&
      matchGlobalDirection(restriction.globalDir(), fareMarket.getGlobalDirection()) &&
      matchTravelType(restriction.travelType(), itin.geoTravelType()) &&
      matchVia(trx, restriction.viaLoc(), fareMarket) &&
      matchCurrency(restriction.curRstrs(), fareMarket) && matchSITILoc(trx,
                                                                        restriction.posExceptInd(),
                                                                        restriction.posLoc(),
                                                                        *TrxUtil::saleLoc(trx),
                                                                        geoTvlType) &&
      matchSITILoc(trx,
                   restriction.poiExceptInd(),
                   restriction.poiLoc(),
                   *TrxUtil::ticketingLoc(trx),
                   geoTvlType);

  if (restricted)
  {
    switch (restriction.restriction())
    {
    case 'P': // To test only
    case 'X': // Do not Quote/Price
      printText(const_cast<SalesNationRestr&>(restriction), trx);
      break;
    case 'T': // Price with text
      printText(const_cast<SalesNationRestr&>(restriction), trx);
      restricted = false;
      break;
    default:
      // Do nothing
      break;
    }
  }
  return restricted;
}

/**
 * Returns true if restriction applies for given user.
 */
bool
SalesRestrictionByNation::matchUser(const Indicator& userApplType,
                                    const UserApplCode& userAppl,
                                    PricingTrx& trx)
{
  // @TODO userApplType matching - no sure how to match it

  // Match userAppl
  if (LIKELY(userAppl.empty()))
    return true;
  else
    return (userAppl == trx.billing()->partitionID()); // For Airline partition
}

bool
SalesRestrictionByNation::matchCarrier(const Itin& itin,
                                       const std::vector<CarrierCode>& restrictedCxrsByRule,
                                       const Indicator& exceptCxr,
                                       const std::vector<CarrierCode>& validatingCxrs,
                                       std::set<CarrierCode>& restrictedCxrs)
{
  if (restrictedCxrsByRule.empty())
    return true;

  if (validatingCxrs.empty())
    return matchCarrier(restrictedCxrsByRule, exceptCxr,
        (itin.validatingCarrier().empty() ? itin.ticketingCarrier() : itin.validatingCarrier()));

  for (const CarrierCode& valCxr : validatingCxrs)
  {
    const bool isFoundInList = std::find(restrictedCxrsByRule.begin(),
        restrictedCxrsByRule.end(), valCxr) != restrictedCxrsByRule.end();

    // Except the carrier or restrict the carrier
    if ((isFoundInList && exceptCxr != 'Y') || (!isFoundInList && exceptCxr == 'Y'))
      restrictedCxrs.insert(valCxr);
  }

  // Not found in the restrictedCxrs
  return !restrictedCxrs.empty();
}

bool
SalesRestrictionByNation::matchCarrier(const std::vector<CarrierCode>& cxrs,
                                       const Indicator& exceptCxr,
                                       const CarrierCode& cxr)
{
  if (cxrs.empty())
    return true;

  std::vector<CarrierCode>::const_iterator carrier = cxrs.begin();
  std::vector<CarrierCode>::const_iterator end = cxrs.end();

  for (; carrier != end; ++carrier)
  {
    if (*carrier == cxr)
    {
      return (exceptCxr != 'Y'); // Except the carrier or restrict the carrier
    }
  }

  // No found in the cxrs
  return (exceptCxr == 'Y');
}

bool
SalesRestrictionByNation::matchDirectionality(PricingTrx& trx,
                                              const SalesNationRestr& restriction,
                                              const Itin& itin,
                                              const FareMarket& fareMarket)
{
  bool exceptLoc = (restriction.exceptLoc() == 'Y');
  bool retVal = true;

  switch (restriction.directionality())
  {
  case BETWEEN: // between of FM
    retVal = matchBetween(trx, restriction.loc1(), restriction.loc2(), fareMarket);
    break;
  case FROM: // from of FM
    retVal = matchFrom(trx,
                       restriction.loc1(),
                       restriction.loc2(),
                       *fareMarket.origin(),
                       *fareMarket.destination(),
                       fareMarket.geoTravelType());
    break;
  case WITHIN: // within of FM
    retVal =
        matchWithin(trx, restriction.loc1(), fareMarket.travelSeg(), fareMarket.geoTravelType());
    break;
  case ORIGIN: // origin of Itin
    retVal = matchLoc(
        trx, restriction.loc1(), *(itin.travelSeg().front()->origin()), itin.geoTravelType());
    break;
  case TERMINATE: // terminate of Itin
    retVal = matchLoc(
        trx, restriction.loc2(), *(itin.travelSeg().back()->destination()), itin.geoTravelType());
    break;
  default:
    break;
  }

  return (exceptLoc ? (!retVal) : retVal);
}

bool
SalesRestrictionByNation::matchGlobalDirection(const GlobalDirection& restrictionGlobalDir,
                                               const GlobalDirection& globalDirection)
{
  if (restrictionGlobalDir >= GlobalDirection::AF && restrictionGlobalDir < GlobalDirection::XX)
    return (restrictionGlobalDir == globalDirection);

  return true;
}

bool
SalesRestrictionByNation::matchTravelType(const Indicator& travelType,
                                          const GeoTravelType& itinTravelType)
{
  switch (travelType)
  {
  case 'D':
    return ((itinTravelType == GeoTravelType::Domestic) || (itinTravelType == GeoTravelType::ForeignDomestic));
  case 'I':
    return ((itinTravelType == GeoTravelType::International) || (itinTravelType == GeoTravelType::Transborder));
  default:
    return true;
  }
}

bool
SalesRestrictionByNation::matchVia(PricingTrx& trx, const LocKey& loc, const FareMarket& fareMarket)
{
  if (loc.loc().empty())
    return true;

  const std::vector<TravelSeg*>& travelSegs = fareMarket.travelSeg();
  if (travelSegs.empty())
    return false;

  GeoTravelType geoTvlType = fareMarket.geoTravelType();

  std::vector<TravelSeg*>::const_iterator i = travelSegs.begin();
  std::vector<TravelSeg*>::const_iterator end = travelSegs.end();

  for (; i != end; ++i)
  {
    if ((i != travelSegs.begin()) && LocUtil::isInLoc(*((*i)->origin()),
                                                      loc.locType(),
                                                      loc.loc(),
                                                      Vendor::SABRE,
                                                      MANUAL,
                                                      LocUtil::OTHER,
                                                      geoTvlType,
                                                      EMPTY_STRING(),
                                                      trx.getRequest()->ticketingDT()))
      return true;

    if ((i != (travelSegs.end() - 1)) && LocUtil::isInLoc(*((*i)->destination()),
                                                          loc.locType(),
                                                          loc.loc(),
                                                          Vendor::SABRE,
                                                          MANUAL,
                                                          LocUtil::OTHER,
                                                          geoTvlType,
                                                          EMPTY_STRING(),
                                                          trx.getRequest()->ticketingDT()))
      return true;

    const std::vector<const Loc*>& hiddenStops = (*i)->hiddenStops();
    std::vector<const Loc*>::const_iterator hiddenIter = hiddenStops.begin();
    for (; hiddenIter != hiddenStops.end(); ++hiddenIter)
    {
      if (LocUtil::isInLoc(**hiddenIter,
                           loc.locType(),
                           loc.loc(),
                           Vendor::SABRE,
                           MANUAL,
                           LocUtil::OTHER,
                           geoTvlType,
                           EMPTY_STRING(),
                           trx.getRequest()->ticketingDT()))
        return true;
    }
  }

  return false;
}

bool
SalesRestrictionByNation::matchCurrency(const std::vector<CurrencyCode>& curRstrs,
                                        const FareMarket& fareMarket)
{
  if (curRstrs.empty())
    return true;

  ConstCurrencyIter itCur = fareMarket.currencies().begin();
  ConstCurrencyIter endCur = fareMarket.currencies().end();
  for (; itCur != endCur; ++itCur)
  {
    const FareMarketCurrencyKey& ccKey = *itCur;
    if (std::find(curRstrs.begin(), curRstrs.end(), ccKey.currencyCode()) != curRstrs.end())
      return true;
  }

  return false;
}

bool
SalesRestrictionByNation::matchBetween(PricingTrx& trx,
                                       const LocKey& loc1,
                                       const LocKey& loc2,
                                       const FareMarket& fareMarket)
{
  GeoTravelType geoTvlType = fareMarket.geoTravelType();

  const std::vector<TravelSeg*> travelSegs = fareMarket.travelSeg();
  std::vector<TravelSeg*>::const_iterator i = travelSegs.begin();
  for (; i != travelSegs.end(); ++i)
  {
    TravelSeg& travelSeg = **i;
    const Loc& origin = *travelSeg.origin();
    const Loc& destination = *travelSeg.destination();

    if (!loc2.loc().empty())
    {
      // between loc1 and loc2
      if ((LocUtil::isInLoc(origin,
                            loc1.locType(),
                            loc1.loc(),
                            Vendor::SABRE,
                            MANUAL,
                            LocUtil::OTHER,
                            geoTvlType,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()) &&
           LocUtil::isInLoc(destination,
                            loc2.locType(),
                            loc2.loc(),
                            Vendor::SABRE,
                            MANUAL,
                            LocUtil::OTHER,
                            geoTvlType,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT())) ||
          (LocUtil::isInLoc(destination,
                            loc1.locType(),
                            loc1.loc(),
                            Vendor::SABRE,
                            MANUAL,
                            LocUtil::OTHER,
                            geoTvlType,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT()) &&
           LocUtil::isInLoc(origin,
                            loc2.locType(),
                            loc2.loc(),
                            Vendor::SABRE,
                            MANUAL,
                            LocUtil::OTHER,
                            geoTvlType,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT())))
        return true;
    }
    else
    {
      // between loc1 and anywhere
      if (LocUtil::isInLoc(origin,
                           loc1.locType(),
                           loc1.loc(),
                           Vendor::SABRE,
                           MANUAL,
                           LocUtil::OTHER,
                           geoTvlType,
                           EMPTY_STRING(),
                           trx.getRequest()->ticketingDT()) ||
          LocUtil::isInLoc(destination,
                           loc1.locType(),
                           loc1.loc(),
                           Vendor::SABRE,
                           MANUAL,
                           LocUtil::OTHER,
                           geoTvlType,
                           EMPTY_STRING(),
                           trx.getRequest()->ticketingDT()))
        return true;

      const std::vector<const Loc*>& hiddenStops = travelSeg.hiddenStops();
      std::vector<const Loc*>::const_iterator hiddenIter = hiddenStops.begin();
      for (; hiddenIter != hiddenStops.end(); ++hiddenIter)
      {
        if (LocUtil::isInLoc(**hiddenIter,
                             loc1.locType(),
                             loc1.loc(),
                             Vendor::SABRE,
                             MANUAL,
                             LocUtil::OTHER,
                             geoTvlType,
                             EMPTY_STRING(),
                             trx.getRequest()->ticketingDT()))
          return true;
      }
    }
  }

  return false;
}

bool
SalesRestrictionByNation::matchFrom(PricingTrx& trx,
                                    const LocKey& loc1,
                                    const LocKey& loc2,
                                    const Loc& origin,
                                    const Loc& destination,
                                    GeoTravelType geoTvlType)
{
  if (!loc2.loc().empty())
  {
    // from loc1 to loc2
    return (LocUtil::isInLoc(origin,
                             loc1.locType(),
                             loc1.loc(),
                             Vendor::SABRE,
                             MANUAL,
                             LocUtil::OTHER,
                             geoTvlType,
                             EMPTY_STRING(),
                             trx.getRequest()->ticketingDT()) &&
            LocUtil::isInLoc(destination,
                             loc2.locType(),
                             loc2.loc(),
                             Vendor::SABRE,
                             MANUAL,
                             LocUtil::OTHER,
                             geoTvlType,
                             EMPTY_STRING(),
                             trx.getRequest()->ticketingDT()));
  }
  else
  {
    // from loc1 to anywhere
    return LocUtil::isInLoc(origin,
                            loc1.locType(),
                            loc1.loc(),
                            Vendor::SABRE,
                            MANUAL,
                            LocUtil::OTHER,
                            geoTvlType,
                            EMPTY_STRING(),
                            trx.getRequest()->ticketingDT());
  }
}

bool
SalesRestrictionByNation::matchWithin(PricingTrx& trx,
                                      const LocKey& loc,
                                      const std::vector<TravelSeg*>& travelSegs,
                                      GeoTravelType geoTvlType)
{
  if (travelSegs.empty())
    return false;

  std::vector<TravelSeg*>::const_iterator i = travelSegs.begin();
  std::vector<TravelSeg*>::const_iterator end = travelSegs.end();

  for (; i != end; ++i)
  {
    if (!LocUtil::isInLoc(*((*i)->origin()),
                          loc.locType(),
                          loc.loc(),
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::OTHER,
                          geoTvlType,
                          EMPTY_STRING(),
                          trx.getRequest()->ticketingDT()))
      return false;
  }

  if (!LocUtil::isInLoc(*(travelSegs.back()->destination()),
                        loc.locType(),
                        loc.loc(),
                        Vendor::SABRE,
                        MANUAL,
                        LocUtil::OTHER,
                        geoTvlType,
                        EMPTY_STRING(),
                        trx.getRequest()->ticketingDT()))
    return false;

  return true;
}

bool
SalesRestrictionByNation::matchSITILoc(PricingTrx& trx,
                                       const Indicator& exceptLoc,
                                       const LocKey& locKey,
                                       const Loc& loc,
                                       GeoTravelType geoTvlType)
{
  bool retVal = LocUtil::isInLoc(loc,
                                 locKey.locType(),
                                 locKey.loc(),
                                 Vendor::SABRE,
                                 MANUAL,
                                 LocUtil::OTHER,
                                 geoTvlType,
                                 EMPTY_STRING(),
                                 trx.getRequest()->ticketingDT());
  return (exceptLoc == 'Y') ? (!retVal) : retVal;
}

bool
SalesRestrictionByNation::matchLoc(PricingTrx& trx,
                                   const LocKey& locKey,
                                   const Loc& loc,
                                   GeoTravelType geoTvlType)
{
  return LocUtil::isInLoc(loc,
                          locKey.locType(),
                          locKey.loc(),
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::OTHER,
                          geoTvlType,
                          EMPTY_STRING(),
                          trx.getRequest()->ticketingDT());
}

void
SalesRestrictionByNation::printText(SalesNationRestr& restriction, PricingTrx& trx)
{
  vector<string>::iterator text = restriction.txtSegs().begin();
  vector<string>::iterator end = restriction.txtSegs().end();

  std::string restrMsg;
  for (; text != end; ++text)
  {
    restrMsg += *text;
    restrMsg += "\n";
  }

  // lint -e{530}
  if (trx.status().find(restrMsg) == string::npos) // Do not save duplicates
    trx.status() += restrMsg;
}

void
SalesRestrictionByNation::displayRestrItem(SalesNationRestr& restr, DiagCollector& diag)
{
  diag << "-------------------------------------------------\n"
       << "SEQ NO " << restr.seqNo() << "\n"
       << "NATION - " << restr.nation() << "\n"
       << "RESTRICTION - " << restr.restriction() << "\n"
       << "VENDOR - " << restr.vendor() << "\n"
       << "USER APPL TYPE - " << restr.userApplType() << "\n"
       << "USER APPL - " << restr.userAppl() << "\n"
       << "EXCEPT TICKETING CARRIER - " << restr.exceptTicketingCxr();

  const std::vector<CarrierCode>& tktgCxrs = restr.tktgCxrs();
  for (const auto& tktgCxr : tktgCxrs)
  {
    diag << tktgCxr << " ";
  }
  diag << " \n";

  diag << "EXCEPT GOVERNING CARRIER - " << restr.exceptGoverningCxr();
  const std::vector<CarrierCode>& govCxrs = restr.govCxrs();
  for (const auto& govCxr : govCxrs)
  {
    diag << govCxr << " ";
  }
  diag << " \n";

  std::string gd;
  globalDirectionToStr(gd, restr.globalDir());
  diag << "GLOBAL DIRECTION - " << gd << "\n"
       << "TRAVEL TYPE - " << restr.travelType() << "\n"
       << "POS EXCEPT IND - " << restr.posExceptInd() << "\n"
       << "POS LOC - " << restr.posLoc().locType() << " " << restr.posLoc().loc() << "\n"
       << "POI EXCEPT IND - " << restr.poiExceptInd() << "\n"
       << "POI LOC - " << restr.poiLoc().locType() << " " << restr.poiLoc().loc() << "\n"
       << "EXCEPT LOC - " << restr.exceptLoc() << "\n"
       << "DIRECTIONALITY - ";

  switch (restr.directionality())
  {
  case FROM:
    diag << "F";
    break;
  case TO:
    diag << "T";
    break;
  case BETWEEN:
    diag << "B";
    break;
  case WITHIN:
    diag << "W";
    break;
  case ORIGIN:
    diag << "O";
    break;
  case TERMINATE:
    diag << "X";
    break;
  default:
    break;
  }

  diag << " \n"
       << "LOC 1 - " << restr.loc1().locType() << " " << restr.loc1().loc() << "\n"
       << "LOC 2 - " << restr.loc2().locType() << " " << restr.loc2().loc() << "\n"
       << "VIA LOC - " << restr.viaLoc().locType() << " " << restr.viaLoc().loc() << "\n"
       << "CURRENCY RESTRI APPL - " << restr.curRestrAppl();

  const std::vector<CurrencyCode>& curRstrs = restr.curRstrs();
  for (const auto& curRstr : curRstrs)
  {
    diag << curRstr << " ";
  }
  diag << " \n";

  diag << "TEXT MESSAGE - ";
  const std::vector<std::string>& txtSegs = restr.txtSegs();
  for (const auto& txtSeg : txtSegs)
  {
    diag << txtSeg << "\n";
  }
  diag << "-------------------------------------------------\n";
}
}
