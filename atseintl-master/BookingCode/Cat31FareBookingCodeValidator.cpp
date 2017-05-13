//----------------------------------------------------------------------------
//  Copyright Sabre 2015
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

#include "BookingCode/Cat31FareBookingCodeValidator.h"
#include "Common/ClassOfService.h"
#include "Common/Foreach.h"
#include "Common/Logger.h"
#include "Common/RBDByCabinUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexBaseTrx.h"
#include "DBAccess/Cabin.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/FareMarketRuleController.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Util/BranchPrediction.h"

#include <boost/foreach.hpp>

namespace tse
{
namespace
{
Logger
logger("atseintl.BookingCode.Cat31FareBookingCodeValidator");

bool
HasFlownAirSeg(const PaxTypeFare& paxTypeFare)
{
  for (const TravelSeg* tvlSeg : paxTypeFare.fareMarket()->travelSeg())
  {
    if (!tvlSeg->unflown() && dynamic_cast<const AirSeg*>(tvlSeg))
    {
      return true;
    }
  }
  return false;
}
}

class Cat31FareBookingCodeValidator::FareAndCabin
{
public:
  FareAndCabin(const PaxTypeFare& paxTypeFare, const PricingTrx& trx);

  const PaxTypeFare& paxTypeFare() const { return _paxTypeFare; }
  const std::vector<ClassOfService>& classOfService() const { return _classOfService; }

private:
  const PaxTypeFare& _paxTypeFare;
  std::vector<ClassOfService> _classOfService;
};

Cat31FareBookingCodeValidator::FareAndCabin::FareAndCabin(const PaxTypeFare& paxTypeFare,
                                                          const PricingTrx& trx)
  : _paxTypeFare(paxTypeFare)
{
  std::vector<BookingCode> bkgCodes;
  if (!paxTypeFare.getPrimeBookingCode(bkgCodes))
  {
    const FBRPaxTypeFareRuleData* fbrPTFBaseFare =
        paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);

    if (fbrPTFBaseFare && !fbrPTFBaseFare->isSpecifiedFare())
      fbrPTFBaseFare->getBaseFarePrimeBookingCode(bkgCodes);
  }

  if (bkgCodes.empty() || !HasFlownAirSeg(paxTypeFare))
    return;

  _classOfService.reserve(bkgCodes.size());

  if(TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx))
  {
    for (const BookingCode& bkgCode : bkgCodes)
    {
      ClassOfService cs;
      cs.bookingCode() = bkgCode;
      _classOfService.push_back(cs);
    }
    RBDByCabinUtil rbdCabin(const_cast<PricingTrx&>(trx), RBD_CAT31);
    rbdCabin.getCabinsByRbd(paxTypeFare, _classOfService);
  }
  else
  {
    for (const BookingCode& bkgCode : bkgCodes)
    {
      ClassOfService cs;
      cs.bookingCode() = bkgCode;
      const Cabin* cabin = trx.dataHandle().getCabin(paxTypeFare.fareMarket()->governingCarrier(),
                                                     cs.bookingCode(),
                                                     paxTypeFare.fareMarket()->travelDate());
      if (cabin)
      {
        cs.cabin() = cabin->cabin();
      }
      else
      {
        cs.cabin().setInvalidClass();
        LOG4CXX_ERROR(
            logger,
            "Cat31FareBookingCodeValidator::FareAndCabin::FareAndCabin() CABIN TABLE ERROR:"
                << "GOV CXR: " << paxTypeFare.fareMarket()->governingCarrier() << " "
                << cs.bookingCode());
      }
      _classOfService.push_back(cs);
    }
  }
}

namespace
{
typedef Cat31FareBookingCodeValidator::FareAndCabin FareAndCabin;
bool
IsNormal(const std::vector<const PaxTypeFare*>& fares)
{
  for (const PaxTypeFare* fare : fares)
  {
    if (!fare->isNormal())
      return false;
  }
  return !fares.empty();
}

bool
IsAnyFlownFareNormal(const FarePath& farePath)
{
  for (const PricingUnit* pricingUnit : farePath.pricingUnit())
  {
    for (const FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      if (!fareUsage->travelSeg().front()->unflown() && fareUsage->isPaxTypeFareNormal())
        return true;
    }
  }
  return false;
}

const PaxTypeFare*
FindFlownFareForSeg(const PricingUnit& pricingUnit, const AirSeg& wantedAirSeg)
{
  for (const FareUsage* fareUsage : pricingUnit.fareUsage())
  {
    for (const TravelSeg* travelSeg : fareUsage->paxTypeFare()->fareMarket()->travelSeg())
    {
      if (travelSeg->unflown())
        break;

      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);

      if (airSeg && airSeg->origAirport() == wantedAirSeg.origAirport() &&
          airSeg->destAirport() == wantedAirSeg.destAirport() &&
          airSeg->flightNumber() == wantedAirSeg.flightNumber() &&
          airSeg->carrier() == wantedAirSeg.carrier())
      {
        return fareUsage->paxTypeFare();
      }
    }
  }
  return nullptr;
}

bool
ValidateCategories(PricingTrx& trx,
                   Itin& itin,
                   PaxTypeFare& ptf,
                   const std::vector<uint16_t>& categories)
{
  if (categories.empty())
    return true;

  std::vector<uint16_t> fmCategories;

  for (uint16_t cat : categories)
  {
    if (ptf.isCategoryProcessed(cat))
    {
      if (!ptf.isCategoryValid(cat))
        return false;
    }
    else
    {
      fmCategories.push_back(cat);
    }
  }

  if (!fmCategories.empty())
  {
    RuleControllerWithChancelor<FareMarketRuleController> ruleController(
        DynamicValidation, fmCategories, &trx);
    return ruleController.validate(trx, itin, ptf);
  }

  return true;
}

bool
FareValidForHierarchy(PricingTrx& trx, Itin& itin, PaxTypeFare& ptf)
{
  if (!trx.matchFareRetrievalDate(ptf))
    return false;

  if (ptf.isValidNoBookingCode())
    return true;

  if (!ptf.fare()->isValid())
    return false;

  if (ptf.fare()->isIndustry())
  {
    IndustryFare* indf = static_cast<IndustryFare*>(ptf.fare());
    if (!indf->validForPricing())
      return false;
  }

  if (!ptf.isNoDataMissing())
    return false;

  if (ptf.isNotValidForCP())
    return false;

  if (ptf.failedFareGroup())
    return false;

  if (!ptf.isRoutingProcessed())
    return false;

  if (!ptf.isRoutingValid())
    return false;

  if (!ptf.isCategoryValid(1))
    return false;

  if (!ptf.isCategoryValid(15))
    return false;

  std::vector<uint16_t> categories;
  categories.push_back(RuleConst::SEASONAL_RULE); // cat 3
  categories.push_back(RuleConst::ACCOMPANIED_PSG_RULE); // cat 13
  categories.push_back(RuleConst::TRAVEL_RESTRICTIONS_RULE); // cat 14
  return ValidateCategories(trx, itin, ptf, categories);
}

std::vector<const PaxTypeFare*>
BuildRbdHierarchy(PricingTrx& trx, const FareMarket& mkt, Itin& itin)
{
  std::vector<const PaxTypeFare*> rbdHierarchy;
  REVERSE_FOREACH (PaxTypeFare* ptf, mkt.allPaxTypeFare())
  {
    if (FareValidForHierarchy(trx, itin, *ptf))
    {
      rbdHierarchy.push_back(ptf);
    }
  }
  return rbdHierarchy;
}

std::unique_ptr<FareAndCabin>
FindSamePrevFare(const RexBaseTrx& trx, const FareMarket& mkt)
{
  for (PricingUnit* pu : trx.exchangeItin().front()->farePath().front()->pricingUnit())
  {
    for (FareUsage* fuExc : pu->fareUsage())
    {
      if (mkt.boardMultiCity() == fuExc->paxTypeFare()->fareMarket()->boardMultiCity() &&
          mkt.offMultiCity() == fuExc->paxTypeFare()->fareMarket()->offMultiCity())
      {
        return std::unique_ptr<FareAndCabin>(new FareAndCabin(*fuExc->paxTypeFare(), trx));
      }
    }
  }
  return {};
}

void
DiffFareBreak(RexBaseTrx& trx,
              const FareMarket& mkt,
              Itin& itin,
              std::vector<FareAndCabin>& prevFareAndCabins,
              std::vector<const PaxTypeFare*>& rbdHierarchy,
              bool& cat31PrevFareNormal)
{
  // when fare break change, we need to collect all previous fares
  // which that are applicable for the flown segments of the new fare market

  const FarePath& prevFarePath = *trx.exchangeItin().front()->farePath().front();

  std::vector<const PaxTypeFare*> prevFares;

  for (const TravelSeg* travelSeg : mkt.travelSeg())
  {
    if (travelSeg->unflown())
      break;

    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);
    if (!airSeg)
      continue;

    for (const PricingUnit* prevPricingUnit : prevFarePath.pricingUnit())
    {
      if (const PaxTypeFare* prevFare = FindFlownFareForSeg(*prevPricingUnit, *airSeg))
      {
        if (std::find(prevFares.begin(), prevFares.end(), prevFare) == prevFares.end())
        {
          prevFares.push_back(prevFare);
        }
        break;
      }
    }
  }

  if (IsNormal(prevFares))
  {
    cat31PrevFareNormal = true;
  }
  else
  {
    for (const PaxTypeFare* prevFare : prevFares)
    {
      prevFareAndCabins.push_back(FareAndCabin(*prevFare, trx));
    }

    // build RBD hierarchy
    rbdHierarchy = BuildRbdHierarchy(trx, mkt, itin);
  }
}

bool
OnlyOneFlownSeg(const RexBaseTrx& trx)
{
  bool foundOne = false;
  const FarePath& prevFarePath = *trx.exchangeItin().front()->farePath().front();
  for (const PricingUnit* prevPricingUnit : prevFarePath.pricingUnit())
  {
    for (const FareUsage* prevFareUsage : prevPricingUnit->fareUsage())
    {
      for (const TravelSeg* travelSeg : prevFareUsage->travelSeg())
      {
        if (!travelSeg->unflown())
        {
          if (foundOne)
            return false;
          else
            foundOne = true;
        }
      }
    }
  }
  return foundOne;
}

bool
PartialOrFullFlown(const std::vector<TravelSeg*>& tvlSegVector)
{
  for (const TravelSeg* tvlSeg : tvlSegVector)
  {
    if (!tvlSeg->unflown())
      return true;
  }
  return false;
}

inline bool
InUsCa(const Itin& itin)
{
  return (itin.geoTravelType() == GeoTravelType::Domestic || itin.geoTravelType() == GeoTravelType::Transborder);
}

std::vector<BookingCode>
GetPrimeBookingCodes(const PaxTypeFare& ptf)
{
  std::vector<BookingCode> bkgCodes;
  if (!ptf.getPrimeBookingCode(bkgCodes))
  {
    const FBRPaxTypeFareRuleData* fbrPTFBaseFare = ptf.getFbrRuleData(RuleConst::FARE_BY_RULE);

    if (fbrPTFBaseFare && !fbrPTFBaseFare->isSpecifiedFare())
      fbrPTFBaseFare->getBaseFarePrimeBookingCode(bkgCodes);
  }
  return bkgCodes;
}

const PaxTypeFare*
FindFare(const std::vector<const PaxTypeFare*>& fares,
         const BookingCode& bookingCode,
         Directionality directionality)
{
  REVERSE_FOREACH (const PaxTypeFare* rbhFare, fares)
  {
    if (rbhFare->directionality() == directionality)
    {
      const std::vector<BookingCode> bkgCodes = GetPrimeBookingCodes(*rbhFare);
      if (std::find(bkgCodes.begin(), bkgCodes.end(), bookingCode) != bkgCodes.end())
      {
        return rbhFare;
      }
    }
  }
  return nullptr;
}

const PaxTypeFare*
FindEntryPoint(const PaxTypeFare& ptf,
               const std::vector<FareAndCabin>& prevFares,
               const std::vector<const PaxTypeFare*>& rbdHierarchy,
               bool primaryCarrierOnly)
{
  const PaxTypeFare* entryPoint = nullptr;
  if (!rbdHierarchy.empty())
  {
    for (const FareAndCabin& prevFare : prevFares)
    {
      if (prevFare.paxTypeFare().isNormal())
        continue;

      if (primaryCarrierOnly)
      {
        if (prevFare.paxTypeFare().fareMarket()->governingCarrier() !=
            ptf.fareMarket()->governingCarrier())
          continue;
      }

      for (const ClassOfService& prevCos : prevFare.classOfService())
      {
        const PaxTypeFare* fare =
            FindFare(rbdHierarchy, prevCos.bookingCode(), prevFare.paxTypeFare().directionality());

        if (fare && (!entryPoint || fare->nucFareAmount() > entryPoint->nucFareAmount()))
        {
          entryPoint = fare;
        }
      }
    }
  }
  return entryPoint;
}

bool
ContainsSameCabin(const FareAndCabin& prevFare, const FareAndCabin& curFare)
{
  for (const ClassOfService& curCos : curFare.classOfService())
  {
    for (const ClassOfService& prevCos : prevFare.classOfService())
    {
      if (curCos.cabin() == prevCos.cabin())
        return true;
    }
  }
  return false;
}

bool
CurrRbdEqualToOrHigherThanPrev(const FareAndCabin& prevFare, const ClassOfService& curCos)
{
  for (const ClassOfService& prevCos : prevFare.classOfService())
  {
    // keep in mind that cabin enum is defined from-higher-to-lower class
    if (curCos.cabin() > prevCos.cabin())
      return false;
  }
  return true;
}

bool
ContainsEqualOrHigherCabin(const FareAndCabin& prevFare, const FareAndCabin& curFare)
{
  // atleast one of current fare RBDs must be equal or
  // higher than the previous fare's RBDs
  for (const ClassOfService& curCos : curFare.classOfService())
  {
    if (CurrRbdEqualToOrHigherThanPrev(prevFare, curCos))
      return true;
  }
  return false;
}

bool
ValidateEntryPoint(const FareAndCabin& curFare,
                   const PaxTypeFare* entryPoint,
                   const std::vector<FareAndCabin>& prevFares)
{
  if (entryPoint)
  {
    return (curFare.paxTypeFare().nucFareAmount() >= entryPoint->nucFareAmount());
  }
  else
  {
    // in the event entry points could not be found
    // make sure the fare cabin of the replacing fare
    // is equal or higher than all previous fares

    for (const FareAndCabin& prevFare : prevFares)
    {
      if (!ContainsEqualOrHigherCabin(prevFare, curFare))
      {
        return false;
      }
    }
    return true;
  }
}

void
PrintRbdHierarchy(DiagCollector& dc, const std::vector<const PaxTypeFare*>& rbdHierarchy)
{
  dc << "------------------------------------------------------------- \n";
  dc << "HIERARCHY TABLE : \n";
  dc << " FARE CLASS       AMOUNT  CUR O/R  EFF DATE   PRIME BKG \n";
  dc << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n";

  for (const PaxTypeFare* ptf : rbdHierarchy)
  {
    dc.setf(std::ios::left, std::ios::adjustfield);

    // display fare type flags
    dc << std::setw(2) << dc.cnvFlags(*ptf);

    // display fare class  or fare basis code
    std::string fareClass = ptf->fareClass().c_str();
    if (fareClass.size() > 12)
      fareClass = fareClass.substr(0, 12) + "*"; // Cross-of-lorraine?
    dc << std::setw(13) << fareClass << " ";

    // display fare amount and currency
    dc << std::setw(9) << Money(ptf->nucFareAmount(), "NUC") << "  ";

    // display one way roundtrip
    if (ptf->owrt() == ONE_WAY_MAY_BE_DOUBLED)
      dc << std::setw(2) << "X";
    else if (ptf->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
      dc << std::setw(2) << "R";
    else if (ptf->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
      dc << std::setw(2) << "O";
    else
      dc << std::setw(2) << " ";
    dc << "  ";

    // effective date
    dc << ptf->effectiveDate().dateToString(YYYYMMDD, "-") << " ";

    // prime booking codes
    const std::vector<BookingCode> bkgCodes = GetPrimeBookingCodes(*ptf);
    if (bkgCodes.empty())
      dc << std::setw(1) << "*";

    for (uint16_t i = 0; i < bkgCodes.size(); i++)
    {
      dc << std::setw(1) << bkgCodes[i] << " ";
      if (i == 6)
      {
        dc << std::setw(1) << "*";
        dc << " ";
        break;
      }
    }
    dc << " \n";
  }
}

void
PrintRbdCabDiag(DiagCollector& dc, const FareAndCabin& fare)
{
  for (const ClassOfService& cos : fare.classOfService())
  {
    dc << cos.bookingCode()[0] << "/" << cos.cabin() << " ";
  }
}

void
PrintAirSegDiag(DiagCollector* diag, const AirSeg& airSeg, bool passCabinCheck)
{
  if (!diag || diag->diagnosticType() != Diagnostic411)
    return;

  DiagCollector& dc = *diag;
  dc << "  " << airSeg.pnrSegment() << ". ";
  dc << airSeg.carrier();
  dc << std::setw(4) << airSeg.flightNumber();
  dc << airSeg.getBookingCode();
  dc << " " << airSeg.origAirport() << " " << airSeg.destAirport();
  dc << " CAB: " << airSeg.bookedCabin();
  if (airSeg.unflown())
    dc << " UNFLOWN ";
  else
    dc << " FLOWN   ";

  if (passCabinCheck)
    dc << " CABIN CHECK PASS ";
  else
    dc << " CABIN CHECK FAIL ";

  dc << " \n";
}

void
PrintCabinDiag(DiagCollector* diag, const FareAndCabin& curFare, bool passCabinCheck)
{
  if (!diag || diag->diagnosticType() != Diagnostic411)
    return;

  DiagCollector& dc = *diag;
  dc << "  " << curFare.paxTypeFare().fareClass() << " ";
  dc << std::setw(9) << Money(curFare.paxTypeFare().nucFareAmount(), "NUC") << " ";
  dc << " PRIME RBD: ";
  PrintRbdCabDiag(dc, curFare);
  if (passCabinCheck)
    dc << "-PASS CABIN ";
  else
    dc << "-FAIL CABIN ";
  dc << " \n";
}

void
PrintEntryPointDiag(DiagCollector* diag,
                    const PaxTypeFare* entryPoint,
                    bool passed,
                    const std::vector<FareAndCabin> prevFares)
{
  if (!diag || diag->diagnosticType() != Diagnostic411)
    return;

  std::string fareClass;
  DiagCollector& dc = *diag;
  dc << "  RBD HIERARCHY ENTRY POINT: ";

  if (entryPoint)
  {
    dc << " \n";
    dc << "    ";
    dc.setf(std::ios::left, std::ios::adjustfield);

    // display fare type flags
    dc << std::setw(2) << dc.cnvFlags(*entryPoint);

    // display fare class  or fare basis code
    fareClass = entryPoint->fareClass().c_str();
    if (fareClass.size() > 12)
      fareClass = fareClass.substr(0, 12) + "*"; // Cross-of-lorraine?
    dc << std::setw(13) << fareClass << " ";

    // display fare amount and currency
    dc << std::setw(9) << Money(entryPoint->nucFareAmount(), "NUC") << "  ";

    // display one way roundtrip
    if (entryPoint->owrt() == ONE_WAY_MAY_BE_DOUBLED)
      dc << std::setw(2) << "X";
    else if (entryPoint->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
      dc << std::setw(2) << "R";
    else if (entryPoint->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
      dc << std::setw(2) << "O";
    else
      dc << std::setw(2) << " ";
    dc << "  ";

    // effective date
    dc << entryPoint->effectiveDate().dateToString(YYYYMMDD, "-") << " ";

    if (entryPoint->isNormal())
      dc << "NL ";
    else
      dc << "SP ";

    const std::vector<BookingCode> bkgCodes = GetPrimeBookingCodes(*entryPoint);

    if (bkgCodes.empty())
      dc << std::setw(1) << "*";

    for (uint16_t i = 0; i < bkgCodes.size(); i++)
    {
      dc << std::setw(1) << bkgCodes[i];
      if (i == 4)
      {
        dc << std::setw(1) << "*";
        dc << " ";
        break;
      }
    }
    dc << "\n";

    if (passed)
      dc << "  AMOUNT HIGHER/EQUAL -RBD HIERARCHY PASS \n";
    else
      dc << "  LOWER AMOUNT -RBD HIERARCHY FAIL \n";
  }
  else
  {
    dc << "*** NOT FOUND *** \n";
    dc << "  PREVIOUS FARES AND FARE CABINS: \n";

    for (const FareAndCabin& prevFare : prevFares)
    {
      dc << "    ";
      fareClass = prevFare.paxTypeFare().fareClass().c_str();
      if (fareClass.size() > 12)
        fareClass = fareClass.substr(0, 12) + "*"; // Cross-of-lorraine?
      dc << std::setw(13) << fareClass << " ";
      dc << "CABIN: ";
      for (const ClassOfService& classOfService : prevFare.classOfService())
      {
        dc << classOfService.cabin();
      }
      dc << " \n";
    }
    dc << " \n";

    if (passed)
      dc << "  NO ENTRY POINT -HIGHER/EQUAL FARE CABIN -PASS \n";
    else
      dc << "  NO ENTRY POINT -LOWER FARE CABIN -FAIL \n";
  }
}

bool
FareRbdCabinHigherThanBooked(DiagCollector* diag,
                             const Cat31FareBookingCodeValidator::FareAndCabin& curFare,
                             const AirSeg& airSeg)
{
  bool cabinPass = false;

  for (const ClassOfService& curCos : curFare.classOfService())
  {
    if (curCos.cabin() <= airSeg.bookedCabin())
    {
      cabinPass = true;
      break;
    }
  }
  PrintAirSegDiag(diag, airSeg, cabinPass);
  return cabinPass;
}

bool
CabinSameAsPrev(const std::vector<FareAndCabin> prevFares,
                const FareAndCabin& curFare,
                DiagCollector* diag)
{
  bool passCabin = false;

  for (const FareAndCabin& prevFare : prevFares)
  {
    passCabin = ContainsSameCabin(prevFare, curFare);
    if (!passCabin)
      break;
  }

  PrintCabinDiag(diag, curFare, passCabin);

  return passCabin;
}

bool
CabinEqualToOrHigherThanPrev(const std::vector<FareAndCabin> prevFares,
                             const FareAndCabin& curFare,
                             DiagCollector* diag)
{
  bool passCabin = false;

  for (const FareAndCabin& prevFare : prevFares)
  {
    passCabin = ContainsEqualOrHigherCabin(prevFare, curFare);
    if (!passCabin)
      break;
  }

  PrintCabinDiag(diag, curFare, passCabin);

  return passCabin;
}

bool
HasFlownSectorWithCarier(const std::vector<TravelSeg*>& travelSegs, const CarrierCode& carrierCode)
{
  for (const TravelSeg* tvlSeg : travelSegs)
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(tvlSeg);
    if (airSeg && !airSeg->unflown() && airSeg->carrier() == carrierCode)
    {
      return true;
    }
  }
  return false;
}

} // namespace

Cat31FareBookingCodeValidator::Cat31FareBookingCodeValidator(PricingTrx& trx,
                                                             const FareMarket& mkt,
                                                             Itin& itin)
  : _trx(trx),
    _mkt(mkt),
    _itin(itin),
    _partialFullFlown(false),
    _onlyOneFlownSeg(false),
    _cat31PrevFareNormal(false)
{
  if (LIKELY(!RexBaseTrx::isRexTrxAndNewItin(_trx)))
    return;

  if (PartialOrFullFlown(_mkt.travelSeg()))
    _partialFullFlown = true;
  else
    return;

  RexBaseTrx& rexBaseTrx = dynamic_cast<RexBaseTrx&>(trx);

  _prevCat31Fare = FindSamePrevFare(rexBaseTrx, _mkt);

  if (!_prevCat31Fare)
  {
    DiffFareBreak(rexBaseTrx, _mkt, itin, _prevFares, _rbdHierarchy, _cat31PrevFareNormal);
    _onlyOneFlownSeg = OnlyOneFlownSeg(rexBaseTrx);
  }
}

Cat31FareBookingCodeValidator::~Cat31FareBookingCodeValidator()
{
}

bool
Cat31FareBookingCodeValidator::isActive() const
{
  return _partialFullFlown;
}

Cat31FareBookingCodeValidator::Result
Cat31FareBookingCodeValidator::validateCat31(const PaxTypeFare& ptf,
                                             const FarePath* farePath,
                                             DiagCollector* diag) const
{
  Result result = SKIPPED;

  if (LIKELY(!_partialFullFlown || (!_prevCat31Fare && _prevFares.empty()) ||
              (_prevCat31Fare && _prevCat31Fare->paxTypeFare().isNormal() && ptf.isNormal())))
  {
    return result;
  }

  if (_prevCat31Fare)
    result = validateSameFareBreak(ptf, *_prevCat31Fare, diag);
  else if (InUsCa(_itin))
    result = validateUsCa(ptf, diag);
  else if (ptf.isNormal())
    result = validateInternationalNormal(ptf, diag);
  else if (_onlyOneFlownSeg)
    result = validateInternationalSpecial(ptf, diag);
  else if (farePath)
  {
    if (IsAnyFlownFareNormal(*farePath))
      result = validateInternationalNormal(ptf, diag);
    else
      result = validateInternationalSpecial(ptf, diag);
  }
  else
    result = POSTPONED_TO_PHASE2;

  if (UNLIKELY(diag && diag->diagnosticType() == Diagnostic411))
    printResultDiag(*diag, ptf, result);

  return result;
}

bool
Cat31FareBookingCodeValidator::validateCat33(const PaxTypeFare& ptf, DiagCollector* diag) const
{
  if (LIKELY(!_partialFullFlown || _trx.excTrxType() != PricingTrx::AF_EXC_TRX))
    return true;

  if (_prevCat31Fare)
  {
    FareAndCabin curFare(ptf, _trx);
    if (!ContainsSameCabin(*_prevCat31Fare, curFare))
    {
      PrintCabinDiag(diag, curFare, false);
      return false;
    }
  }
  else if (!_prevFares.empty())
  {
    FareAndCabin curFare(ptf, _trx);
    if (!CabinSameAsPrev(_prevFares, curFare, diag))
    {
      return false;
    }
  }

  return true;
}

void
Cat31FareBookingCodeValidator::printDiagHeader(DiagCollector& dc, const FareUsage* fareUsage) const
{
  if (!RexBaseTrx::isRexTrxAndNewItin(_trx) ||
      !(dc.diagnosticType() == Diagnostic400 || dc.diagnosticType() == Diagnostic411))
    return;

  const RexBaseTrx& rexBaseTrx = dynamic_cast<const RexBaseTrx&>(_trx);

  dc << "************************************************************* \n";
  dc << "CAT 3" << ((_trx.excTrxType() == PricingTrx::AR_EXC_TRX) ? "1" : "3")
     << " RBD PROCESSING - FARE RETRIEVAL DATE - "
     << _trx.ticketingDate().dateToString(YYYYMMDD, "-") << " \n";

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "PREVIOUS FARES   :";
  if (fareUsage != nullptr)
    dc << "          *** PU LEVEL PROCESS ***";
  dc << " \n";
  for (const PricingUnit* pu : rexBaseTrx.exchangeItin().front()->farePath().front()->pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      printFareDiag(dc, *fu);
    }
  }

  dc << "NEW ITIN FLIGHTS :  \n";
  for (const TravelSeg* tvlSeg : _mkt.travelSeg())
  {
    if (const AirSeg* airSeg = dynamic_cast<const AirSeg*>(tvlSeg))
    {
      dc << airSeg->pnrSegment() << ". ";
      dc << airSeg->carrier();
      dc << std::setw(4) << airSeg->flightNumber();
      dc << airSeg->getBookingCode();
      dc << " " << airSeg->origAirport() << " " << airSeg->destAirport();
      dc << " CAB: " << airSeg->bookedCabin();
      dc << (airSeg->unflown() ? " UNFLOWN " : " FLOWN ");
      dc << " \n";
    }
    else
    {
      dc << "--- ARUNK --- \n";
    }
  }

  if (!_partialFullFlown)
  {
    dc << " \n";
    dc << "UNFLOWN FARE MARKET " << _mkt.boardMultiCity() << "-" << _mkt.offMultiCity()
       << " -STANDARD RBD APPLY \n";
    return;
  }

  if (!_prevCat31Fare)
  {
    if (!_prevFares.empty())
    {
      dc << " \n";
      dc << "FARE MARKET " << _mkt.boardMultiCity() << "-" << _mkt.offMultiCity()
         << " -FARE BREAK CHANGE -RBD HIERARCHY APPLY \n";
      if (dc.diagnosticType() == Diagnostic400)
        PrintRbdHierarchy(dc, _rbdHierarchy);
    }
    else
    {
      dc << " \n";
      if (_cat31PrevFareNormal)
        dc << "ALL PREVIOUS MATCHED FARES NORMAL -STANDARD RBD APPLY \n";
      else
        dc << "NO MATCH PREVIOUS FARES -STANDARD RBD APPLY \n";
    }
  }
}

void
Cat31FareBookingCodeValidator::printResultDiag(DiagCollector& dc,
                                               const PaxTypeFare& ptf,
                                               Result result) const
{
  if (!_partialFullFlown)
    return;

  dc << "  CAT 3" << ((_trx.excTrxType() == PricingTrx::AR_EXC_TRX) ? "1" : "3") << " RBD - ";

  if (_prevCat31Fare)
  {
    if (result == PASSED)
    {
      if (_mkt.isFlown())
        dc << "PASS FARE-ALL FLOWN ";
      else
        dc << "SKIP FLOWN SEGS ";
      dc << std::setw(9) << Money(ptf.nucFareAmount(), "NUC") << " \n";
    }
    else if (result == FAILED)
    {
      if (ptf.nucFareAmount() < _prevCat31Fare->paxTypeFare().nucFareAmount())
      {
        dc << "FAIL LOWER FARE AMT ";
        dc << std::setw(9) << Money(ptf.nucFareAmount(), "NUC") << " \n";
      }
      else
      {
        dc << "FAIL CABIN CHECK"
           << " \n \n";
      }
    }
    else if (result == SKIPPED)
    {
      dc << "STANDARD RBD APPLY \n";
    }
    else
    {
      dc << "POSTPONED TO PHASE 2 - FARE PATH REQUIRED \n";
    }
  }
  else
  {
    if (result == PASSED)
    {
      if (_mkt.isFlown())
      {
        if (!InUsCa(_itin) && ptf.isNormal())
          dc << "PASS FARE -CABIN PASS -ALL FLOWN \n";
        else
          dc << "PASS FARE -RBD HIERARCHY PASS -ALL FLOWN \n";
      }
      else
      {
        dc << "SKIP FLOWN SEGS \n";
      }
    }
    else if (result == FAILED)
    {
      dc << "FAIL FARE -RBD HIERARCHY FAIL \n";
    }
    else if (result == SKIPPED)
    {
      dc << "STANDARD RBD APPLY \n";
    }
    else
    {
      dc << "POSTPONED TO PHASE 2 - FARE PATH REQUIRED \n";
    }
  }
}

Cat31FareBookingCodeValidator::Result
Cat31FareBookingCodeValidator::validateSameFareBreak(const PaxTypeFare& ptf,
                                                     const FareAndCabin& prevFare,
                                                     DiagCollector* diag) const
{
  if (ptf.nucFareAmount() < prevFare.paxTypeFare().nucFareAmount())
  {
    return FAILED;
  }

  if (!prevFare.paxTypeFare().isNormal())
  {
    FareAndCabin curFare(ptf, _trx);
    const bool passed = ContainsEqualOrHigherCabin(prevFare, curFare);
    PrintCabinDiag(diag, curFare, passed);
    if (!passed)
    {
      return FAILED;
    }
  }

  return PASSED;
}

Cat31FareBookingCodeValidator::Result
Cat31FareBookingCodeValidator::validateUsCa(const PaxTypeFare& ptf, DiagCollector* diag) const
{
  // check cabin
  FareAndCabin curFare(ptf, _trx);
  if (!CabinEqualToOrHigherThanPrev(_prevFares, curFare, diag))
  {
    return FAILED;
  }

  // find entry points
  const PaxTypeFare* entryPoint = FindEntryPoint(ptf, _prevFares, _rbdHierarchy, false);
  const bool passed = ValidateEntryPoint(curFare, entryPoint, _prevFares);

  PrintEntryPointDiag(diag, entryPoint, passed, _prevFares);

  return passed ? PASSED : FAILED;
}

Cat31FareBookingCodeValidator::Result
Cat31FareBookingCodeValidator::validateInternationalNormal(const PaxTypeFare& ptf,
                                                           DiagCollector* diag) const
{
  FareAndCabin curFare(ptf, _trx);

  bool passCabin = true;
  for (const TravelSeg* tvlSeg : _mkt.travelSeg())
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(tvlSeg);
    if (!airSeg)
      continue;
    if (airSeg->unflown())
      continue;
    if (!FareRbdCabinHigherThanBooked(diag, curFare, *airSeg))
    {
      passCabin = false;
      break;
    }
  }
  PrintCabinDiag(diag, curFare, passCabin);

  return passCabin ? PASSED : SKIPPED;
}

Cat31FareBookingCodeValidator::Result
Cat31FareBookingCodeValidator::validateInternationalSpecial(const PaxTypeFare& ptf,
                                                            DiagCollector* diag) const
{
  FareAndCabin curFare(ptf, _trx);

  // check primary/secondary flown sector with carrier == current fare gov cxr
  if (HasFlownSectorWithCarier(_mkt.travelSeg(), ptf.fareMarket()->governingCarrier()))
  {
    // check cabin
    if (!CabinEqualToOrHigherThanPrev(_prevFares, curFare, diag))
    {
      return FAILED;
    }

    // find entry points
    const PaxTypeFare* entryPoint =
        FindEntryPoint(curFare.paxTypeFare(), _prevFares, _rbdHierarchy, true);

    const bool passed = ValidateEntryPoint(curFare, entryPoint, _prevFares);

    PrintEntryPointDiag(diag, entryPoint, passed, _prevFares);

    if (!passed)
    {
      return FAILED;
    }
  }

  // check secondary flown sector
  bool passCabin = true;
  for (const TravelSeg* tvlSeg : _mkt.travelSeg())
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(tvlSeg);
    if (!airSeg)
      continue;
    if (airSeg->unflown())
      continue;
    if (airSeg->carrier() == ptf.fareMarket()->governingCarrier())
      continue;
    if (!FareRbdCabinHigherThanBooked(diag, curFare, *airSeg))
    {
      passCabin = false;
      break;
    }
  }

  PrintCabinDiag(diag, curFare, passCabin);

  return passCabin ? PASSED : FAILED;
}

void
Cat31FareBookingCodeValidator::printFareDiag(DiagCollector& dc, const FareUsage& fareUsage) const
{
  dc << std::setw(14) << fareUsage.paxTypeFare()->fareClass();
  dc << " " << fareUsage.paxTypeFare()->fareMarket()->boardMultiCity() << "-"
     << fareUsage.paxTypeFare()->fareMarket()->governingCarrier() << "-"
     << fareUsage.paxTypeFare()->fareMarket()->offMultiCity();

  dc << std::setw(9) << Money(fareUsage.paxTypeFare()->nucFareAmount(), "NUC") << " ";
  if (fareUsage.isPaxTypeFareNormal())
    dc << "NL ";
  else
    dc << "SP ";

  if (_prevCat31Fare && fareUsage.paxTypeFare() == &_prevCat31Fare->paxTypeFare())
  {
    dc << "RBD/CAB: ";
    PrintRbdCabDiag(dc, *_prevCat31Fare);
  }

  for (const FareAndCabin& prevFare : _prevFares)
  {
    if (&prevFare.paxTypeFare() == fareUsage.paxTypeFare())
    {
      dc << "RBD/CAB: ";
      PrintRbdCabDiag(dc, prevFare);
    }
  }

  dc << " \n";
}

} // namespace tse
