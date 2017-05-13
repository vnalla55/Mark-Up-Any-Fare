//-------------------------------------------------------------------
//
//  File:        IndustryFareController.h
//  Created:     June 29, 2004
//  Authors:     Mark Kasprowicz
//
//  Description: Industry fare factory
//
//  Updates:
//
//  Copyright Sabre 2004
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

#include "Fares/IndustryFareController.h"

#include "Common/FareCalcUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/IndustryFareAppl.h"
#include "DBAccess/IndustryFareBasisMod.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DCFactory.h"
#include "Fares/FareController.h"
#include "Rules/RuleUtil.h"

#include <algorithm>
#include <sstream>
#include <vector>

using namespace std;

namespace tse
{
// const CarrierCode IndustryFareController::INDUSTRY_CARRIER  = "YY";
const Indicator IndustryFareController::DO_NOT_CHANGE_FARE_BASIS = 'Y';
const Indicator IndustryFareController::EXCLUDE_ALL_PRICING_FARE_QUOTE = '1';
const Indicator IndustryFareController::EXCLUDE_ALL_PRICING = '2';
const Indicator IndustryFareController::ALLOW_WHEN_NO_CARRIER = '3';
const Indicator IndustryFareController::ALLOW_WHEN_NO_CARRIER_OF_TYPE = '4';
const Indicator IndustryFareController::ALLOW_ALL = '5';

FALLBACK_DECL(fallbackFixFNFailedFares);

namespace
{
log4cxx::LoggerPtr _logger = log4cxx::Logger::getLogger("atseintl.Fares.IndustryFareController");
}

IndustryFareController::IndustryFareController(PricingTrx& trx,
                                               Itin& itin,
                                               FareMarket& fareMarket,
                                               bool processMultilateral,
                                               bool processPureYY)
  : FareController(trx, itin, fareMarket),
    _diag(DCFactory::instance()->create(_trx)),
    _processMultilateral(processMultilateral),
    _processPureYY(processPureYY)
{
  // We dont want multilateral fares for the YY carrier.
  if (_processMultilateral && (_fareMarket.governingCarrier() == INDUSTRY_CARRIER))
    _processMultilateral = false;

  // Init _userApplType and _userAppl
  FareCalcUtil::getUserAppl(_trx, *trx.getRequest()->ticketingAgent(), _userApplType, _userAppl);
}

IndustryFareController::~IndustryFareController()
{
  if (LIKELY(_diag != nullptr))
  {
    _diag->flushMsg();
    _diag = nullptr;
  }

  _addOnFares = nullptr;
}

bool
IndustryFareController::process(const std::vector<Fare*>* addOnFaresVec)
{
  // Check FareMarket diag parm
  if (isIndustryFareDiagnostic(_trx.diagnostic().diagnosticType()))
  {
    if (!_trx.diagnostic().shouldDisplay(_fareMarket))
      return true;
  }

  _addOnFares = addOnFaresVec;

  vector<Fare*> fares;

  _diag->enable(Diagnostic271, Diagnostic272, Diagnostic277);
  diagHeader();

  _diag->enable(Diagnostic273);
  diag273(_fareMarket);

  // Get all YY fares
  findFares(INDUSTRY_CARRIER, fares);

  size_t addOnFares = 0;

  if ((_addOnFares != nullptr) && !_addOnFares->empty())
  {
    addOnFares = _addOnFares->size();
  }

  if (_logger->isInfoEnabled())
  {
    std::ostringstream oss;

    oss << "IndustryFareController - " << fares.size() << " total fares and " << addOnFares
        << " additional fares for " << FareMarketUtil::getDisplayString(_fareMarket) << " market";

    _logger->info(oss.str());
  }

  // This isnt an error, there might not be any records
  if (fares.empty() && (addOnFares == 0))
  {
    _diag->enable(Diagnostic271);
    *_diag << "NO MULTILATERAL FARES FOUND FOR FARE MARKET" << endl;

    _diag->enable(Diagnostic272);
    *_diag << "NO YY FARES FOUND FOR FARE MARKET" << endl;

    _diag->enable(Diagnostic273, Diagnostic277);
    *_diag << "NO INDUSTRY FARES FOUND FOR FARE MARKET" << endl;

    LOG4CXX_DEBUG(_logger, "IndustryFareController - findFares returned no records")
    return true;
  }

  vector<const IndustryFareAppl*> empty;

  // Now get all the application records
  // Get the Multilateral records - 'M' industryFareAppl
  const vector<const IndustryFareAppl*>& allMultilateralAppls =
      (_processMultilateral
           ? _trx.dataHandle().getIndustryFareAppl(
                 MULTILATERAL, _fareMarket.governingCarrier(), _travelDate)
           : empty);

  const vector<const IndustryFareAppl*>& allIndustryAppls =
      (_processPureYY
           ? _trx.dataHandle().getIndustryFareAppl(
                 INDUSTRY, _fareMarket.governingCarrier(), _travelDate)
           : empty);
  vector<const IndustryFareAppl*> filteredMultilateralAppls;
  vector<const IndustryFareAppl*> filteredIndustryAppls;
  const vector<const IndustryFareAppl*>& multilateralAppls =
      filterByVendorForCat31(allMultilateralAppls, filteredMultilateralAppls);
  const vector<const IndustryFareAppl*>& industryAppls =
      filterByVendorForCat31(allIndustryAppls, filteredIndustryAppls);

  // Add a 'blank YY to the end as a catchall so we always create a fare
  if (LIKELY(_processPureYY))
  {
    IndustryFareAppl* blankAppl = nullptr;
    _trx.dataHandle().get(blankAppl);

    if (LIKELY(blankAppl != nullptr))
    {
      blankAppl->seqNo() = 0;
      blankAppl->fareTariff() = -1;
      blankAppl->globalDir() = GlobalDirection::ZZ;
      blankAppl->owrt() = ' ';
      blankAppl->selectionType() = INDUSTRY;
      blankAppl->yyFareAppl() = ALLOW_ALL;
      blankAppl->directionality() = ' ';
      blankAppl->userApplType() = ' ';
      blankAppl->userAppl() = "";
      blankAppl->loc1().locType() = LOCTYPE_NONE;
      blankAppl->loc2().locType() = LOCTYPE_NONE;

      const_cast<vector<const IndustryFareAppl*>&>(industryAppls).push_back(blankAppl);
    }
  }

  if (UNLIKELY(IS_DEBUG_ENABLED(_logger)))
  {
    std::ostringstream oss;
    oss << "IndustryFareController - matchFares getIndustryFareAppl returned "
        << multilateralAppls.size() << " multilateral and " << industryAppls.size()
        << " industry records";
    _logger->debug(oss.str());
  }

  // First find all the YY fares

  vector<PaxTypeFare*> indFares;

  // We already checked is fares was empty
  matchFares(fares, multilateralAppls, industryAppls, indFares);

  const vector<PaxTypeFare*>* specifiedFares = nullptr;
  // If there arent any specified fares carrier pref doesnt apply
  if (!indFares.empty())
  {
    const CarrierPreference* cp =
        _trx.dataHandle().getCarrierPreference(EMPTY_STRING(), _travelDate);

    if (LIKELY(cp != nullptr && cp->applyspecoveraddon() == 'Y'))
    {
      specifiedFares = &indFares;
    }
  }

  // Now find all the YY AddOn fares
  vector<PaxTypeFare*> addOnIndFares;
  if (_addOnFares != nullptr && !_addOnFares->empty())
  {
    matchFares(*_addOnFares, multilateralAppls, industryAppls, addOnIndFares, specifiedFares);
  }

  // Now create all the pax type fare objects
  bool faresCreated = (indFares.size() > 0 || addOnIndFares.size() > 0);

  if (!indFares.empty())
    validateAndAddPaxTypeFares(indFares);

  if (!addOnIndFares.empty())
    validateAndAddPaxTypeFares(addOnIndFares);

  if (!faresCreated)
  {
    _diag->enable(Diagnostic271);
    *_diag << "NO MATCH ON MULTILATERAL FARES" << endl;

    _diag->enable(Diagnostic272);
    *_diag << "NO MATCH ON INDUSTRY FARES" << endl;

    _diag->enable(Diagnostic277);
    *_diag << "NO MATCH ON FARES" << endl;
  }

  if (LIKELY(_diag != nullptr))
    _diag->flush();

  return true;
}

IndustryFare*
IndustryFareController::matchFare(const PaxTypeFare& ptFare,
                                  const vector<const IndustryFareAppl*>& fareAppls,
                                  const std::vector<PaxTypeFare*>* specifiedFares)
{
  vector<const IndustryFareAppl*>::const_iterator k = fareAppls.begin();
  vector<const IndustryFareAppl*>::const_iterator l = fareAppls.end();

  MatchResult matchResult;
  _matchFareTypeOfTag2CxrFare = false;

  for (; k != l; ++k)
  {
    const IndustryFareAppl* appl = *k;

    if (UNLIKELY(_trx.diagnostic().diagnosticType() == Diagnostic273))
      diag273(*appl);

    IndustryFareAppl::ExceptAppl* except = nullptr;
    matchResult = matchFare(ptFare, *appl, except);

    // Now for minimum fares we create the items we previously didnt create,
    // but we mark them invalid for pricing
    if (matchResult == MATCH_CREATE_FARE || matchResult == MATCH_DONT_CREATE_FARE)
    {
      // Check carrier preference
      if (specifiedFares != nullptr)
      {
        const ConstructedFareInfo* cfi = ptFare.fare()->constructedFareInfo();

        if (LIKELY(cfi != nullptr))
        {
          if (isCFDInResponse(*specifiedFares, *cfi))
          {
            // This is a dupe, dont use it
            diag273("FAILED CARRIER PREFERENCE");
            return nullptr;
          }
        }
      }

      const IndustryFareBasisMod* basisMod = nullptr;

      // We need the fareBasisMod for multilateral fares only
      if (LIKELY(appl->selectionType() == INDUSTRY))
      {
        // TODO get values from except
        Indicator userApplType = appl->userApplType();
        UserApplCode userAppl = appl->userAppl();

        if (except != nullptr)
        {
          // If an exception was found the values come from there
          userApplType = except->userApplType();
          userAppl = except->userAppl();
        }

        const vector<const IndustryFareBasisMod*>& basisModList =
            _trx.dataHandle().getIndustryFareBasisMod(
                _fareMarket.governingCarrier(), userApplType, userAppl, _travelDate);

        if (!basisModList.empty())
        {
          basisMod = basisModList.front();
        }
      }

      IndustryFare* indFare = nullptr;
      _trx.dataHandle().get(indFare);
      // lint --e{413}

      bool validForPricing = (matchResult == MATCH_CREATE_FARE);

      if (UNLIKELY(!indFare->initialize(*ptFare.fare(), *appl, except, _trx.dataHandle(), validForPricing)))
      {
        LOG4CXX_ERROR(
            _logger,
            "IndustryFareController - Found an appl match but unable to initialize IndustryFare")
        return nullptr;
      }

      // Moved to IndustryFare::initialize
      // indFare->carrier() = appl->carrier();

      if (LIKELY(_trx.diagnostic().diagnosticType() != Diagnostic273))
        diagFare(ptFare, appl);

      if (UNLIKELY((_trx.diagnostic().diagnosticType() == Diagnostic271) ||
                   (_trx.diagnostic().diagnosticType() == Diagnostic272)))
      {
        diagFareAppl(*appl, except, basisMod);
      }

      // For industry fares always change the fare basis code unless
      // explicitly told not to
      if ((INDUSTRY == appl->selectionType()) &&
          ((nullptr == basisMod) || (DO_NOT_CHANGE_FARE_BASIS != basisMod->doNotChangeFareBasisInd())))
      {
        indFare->changeFareClass() = true;
      }

      // For industry fare application option#4, the industry fare with tag 1 that
      // the carrier fare with tag 2 and same fare type is found, cannot be set as invalid fare now.
      // The indicator will be set and the PU Fare selection will check this indicator.
      // For RT, CT and OJ PU, PO can ignore this fare, but for OW, PU will select this fare.
      if (UNLIKELY(validForPricing && _matchFareTypeOfTag2CxrFare))
      {
        indFare->matchFareTypeOfTag2CxrFare() = true;
      }

      return indFare;

    } // We didnt find a match, check the next fare
  }

  return nullptr;
}

IndustryFareController::MatchResult
IndustryFareController::matchFare(const PaxTypeFare& paxTypeFare,
                                  const IndustryFareAppl& appl,
                                  IndustryFareAppl::ExceptAppl*& except)
{
  string reason;

  if (!matchIndAppl(paxTypeFare, appl, reason))
  {
    if (IS_DEBUG_ENABLED(_logger))
    {
      std::ostringstream oss;
      oss << "matchIndApp failed on " << reason;
      _logger->debug(oss.str());
    }

    diag273(reason.c_str());
    return NO_MATCH;
  }

  // If this is a multilateral fare we're done
  if (UNLIKELY(appl.selectionType() == MULTILATERAL))
  {
    LOG4CXX_DEBUG(_logger, "IndustryFareController - matchFare success for Multilateral fare")
    diag273("MATCH CREATE");
    return MATCH_CREATE_FARE;
  }

  except = nullptr;
  if (!appl.except().empty())
  {
    // Try all the exception records
    std::vector<IndustryFareAppl::ExceptAppl>::const_iterator i = appl.except().begin();
    std::vector<IndustryFareAppl::ExceptAppl>::const_iterator j = appl.except().end();

    for (; i != j; ++i)
    {
      const IndustryFareAppl::ExceptAppl& exceptAppl = *i;

      diag273(exceptAppl);
      if (!matchIndExceptAppl(paxTypeFare, exceptAppl, reason))
      {
        diag273(reason.c_str());
      }
      else
      {
        except = const_cast<IndustryFareAppl::ExceptAppl*>(&exceptAppl);
        diag273("EXCEPTION MATCH");
        break;
      }
    }
  }

  Indicator yyFareAppl = 0;
  if (except != nullptr)
    yyFareAppl = except->yyFareAppl();
  else
    yyFareAppl = appl.yyFareAppl();

  // Ok if we got this far we need to validate if we can 'use' this fare
  if (yyFareAppl == ALLOW_ALL)
  {
    LOG4CXX_DEBUG(_logger, "IndustryFareController - matchFare success for ALLOW_ALL fare")
    diag273("MATCH CREATE");
    return MATCH_CREATE_FARE;
  }
  // If fare is set only to be displayed in fare quote, but not to price.
  // Therefore, for FQ sets _validForPricing to be true in order to pass or display this fare.
  if (UNLIKELY(isFdTrx() && (yyFareAppl == EXCLUDE_ALL_PRICING)))
  {
    LOG4CXX_DEBUG(_logger, "IndustryFareController - matchFare success for ALLOW_ALL fare")
    diag273("MATCH CREATE");
    return MATCH_CREATE_FARE;
  }

  if ((yyFareAppl == EXCLUDE_ALL_PRICING_FARE_QUOTE) || (yyFareAppl == EXCLUDE_ALL_PRICING))
  {
    if (UNLIKELY(IS_DEBUG_ENABLED(_logger)))
    {
      std::ostringstream oss;
      oss << "IndustryFareController - matchFare failed yyFareAppl='" << yyFareAppl << "'";
      _logger->debug(oss.str());
    }
    diag273("MATCH CREATE INVALID FOR PRICING");
    return MATCH_DONT_CREATE_FARE;
  }

  if (yyFareAppl == ALLOW_WHEN_NO_CARRIER)
  {
    if (govCarrierHasFares(paxTypeFare, yyFareAppl))
    {
      if (UNLIKELY(IS_DEBUG_ENABLED(_logger)))
      {
        std::ostringstream oss;
        oss << "IndustryFareController - matchFare failed yyFareAppl='" << yyFareAppl
            << "' and carrier fares exist";
        _logger->debug(oss.str());
      }
      diag273("MATCH CREATE INVALID FOR PRICING");
      return MATCH_DONT_CREATE_FARE;
    }
  }

  if (yyFareAppl == ALLOW_WHEN_NO_CARRIER_OF_TYPE)
  {
    if (UNLIKELY(paxTypeFare.isFareClassAppMissing()))
    {
      if (UNLIKELY(IS_DEBUG_ENABLED(_logger)))
      {
        std::ostringstream oss;
        oss << "IndustryFareController - matchFare failed yyFareAppl='" << yyFareAppl
            << "' and PaxTypeFare is missing fareClassApp";
        _logger->debug(oss.str());
      }
      return NO_MATCH;
    }

    if (govCarrierHasFares(paxTypeFare, yyFareAppl))
    {
      if (UNLIKELY(IS_DEBUG_ENABLED(_logger)))
      {
        std::ostringstream oss;
        oss << "IndustryFareController - matchFare failed yyFareAppl='" << yyFareAppl
            << "' and carrier fares exist for type '" << paxTypeFare.fareClassAppInfo()->_fareType
            << "'";
        _logger->debug(oss.str());
      }
      diag273("MATCH CREATE INVALID FOR PRICING");
      return MATCH_DONT_CREATE_FARE;
    }
  }

  LOG4CXX_DEBUG(_logger, "IndustryFareController - matchFare sucess")

  diag273("MATCH CREATE");
  return MATCH_CREATE_FARE;
}

void
IndustryFareController::diagHeader()
{
  if (LIKELY(!_diag || !_diag->isActive()))
    return;

  *_diag << " " << endl;
  *_diag << "*************************************************************" << endl;

  // If we dont have travel segments, we cant output this line
  if (_fareMarket.travelSeg().size() == 0)
    return;

  *_diag << FareMarketUtil::getFullDisplayString(_fareMarket) << endl;

  *_diag << "*************************************************************" << endl;

  DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();

  *_diag << " " << endl;

  *_diag << "  GI V RULE RTNG   FARE CLS   TRF O O CUR FAR   CNV";

  if (diagType == Diagnostic277)
    *_diag << "    CXR SEQ";

  *_diag << endl;

  *_diag << "                              NUM R I     TPE   AMT" << endl;
  *_diag << endl;

  *_diag << "- -- - ---- ---- ------------ --- - - --- --- --------";

  if (diagType == Diagnostic277)
    *_diag << " --- ---";

  *_diag << endl;
}

void
IndustryFareController::diagFare(const PaxTypeFare& paxTypeFare, const IndustryFareAppl* appl)
{
  if (LIKELY(!_diag || !_diag->isActive()))
    return;

  DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();

  const Fare& fare = *paxTypeFare.fare();

  _diag->setf(ios::left, ios::adjustfield);

  *_diag << std::setw(2) << _diag->cnvFlags(paxTypeFare) << setw(2)
         << globalDirectionString(paxTypeFare.globalDirection()) << ' ' << setw(1)
         << fare.vendor()[0] << ' ' << setw(4) << fare.ruleNumber() << ' ' << setw(4)
         << fare.routingNumber() << ' ' << setw(12) << fare.fareClass() << ' ' << setw(3)
         << fare.tariffCrossRefInfo()->ruleTariff() << ' ' << owrtChar(fare.owrt()) << ' '
         << directionalityChar(fare.directionality()) << ' ';

  *_diag << setw(3) << fare.currency() << " ";

  *_diag << setw(3) << paxTypeFare.fareClassAppInfo()->_fareType << " ";

  _diag->setf(ios::right, ios::adjustfield);

  *_diag << setw(8) << fare.nucFareAmount();

  _diag->setf(ios::left, ios::adjustfield);

  if ((diagType == Diagnostic277) && appl)
  {
    string cxr;

    if (appl->selectionType() == MULTILATERAL)
      cxr = appl->carrier();
    else
      cxr = INDUSTRY_CARRIER;
    *_diag << " " << setw(3) << cxr << " " << setw(3) << appl->seqNo();
  }
  *_diag << endl;
}

void
IndustryFareController::diagFareAppl(const IndustryFareAppl& appl,
                                     const IndustryFareAppl::ExceptAppl* except,
                                     const IndustryFareBasisMod* basisMod)
{
  if (LIKELY(!_diag || !_diag->isActive()))
    return;

  *_diag << "IFA "
         << "TYPE-" << appl.selectionType() << " CXR-" << appl.carrier() << " SEQ-" << appl.seqNo()
         << " V-" << appl.vendor() << " YYA-" << yyFareApplString(appl.yyFareAppl()) << endl;

  if (appl.loc1().locType() != LOCTYPE_NONE)
    *_diag << "LOC1-" << appl.loc1().locType() << "-" << appl.loc1().loc();

  if (appl.loc2().locType() != LOCTYPE_NONE)
    *_diag << "LOC2-" << appl.loc2().locType() << "-" << appl.loc2().loc();

  if (appl.directionality() != ' ')
    *_diag << " DIR-" << appl.directionality();

  std::string gd = globalDirectionString(appl.globalDir());
  if (!gd.empty())
    *_diag << " GD-" << gd;

  if (appl.fareTariff() != -1)
    *_diag << " FTRF-" << appl.fareTariff();

  if (!appl.rule().empty())
    *_diag << " RULE-" << appl.rule();

  if (!appl.routing().empty())
    *_diag << "RTNG-" << appl.routing();

  if (!appl.footNote().empty())
    *_diag << " FTN-" << appl.footNote();

  if (!appl.cur().empty())
    *_diag << " CUR-" << appl.cur();

  if (!appl.fareClass().empty())
    *_diag << " FC-" << appl.fareClass();

  if (!appl.fareType().empty())
    *_diag << " FT-" << appl.fareType();

  if (appl.owrt() != ' ')
    *_diag << " OWRT-" << owrtChar(appl.owrt());

  *_diag << endl;

  if (except != nullptr)
  { // This will always be in the IFA segment so needs a newline
    *_diag << endl << "IFE "
           << " CXR-" << except->exceptCarrier() << " V-" << except->vendor()
           << yyFareApplString(except->yyFareAppl()) << endl;

    if (except->loc1().locType() != LOCTYPE_NONE)
      *_diag << "LOC1-" << except->loc1().locType() << "-" << except->loc1().loc();

    if (except->loc2().locType() != LOCTYPE_NONE)
      *_diag << "LOC2-" << except->loc2().locType() << "-" << except->loc2().loc();

    if (except->directionality() != ' ')
      *_diag << " DIR-" << except->directionality();

    std::string gde = globalDirectionString(except->globalDir());
    if (!gde.empty())
      *_diag << " GD-" << gde;

    if (except->fareTariff() != -1)
      *_diag << " FTRF-" << except->fareTariff();

    if (!except->rule().empty())
      *_diag << " RULE-" << except->rule();

    if (!except->routing().empty())
      *_diag << "RTNG-" << except->routing();

    if (!except->footNote().empty())
      *_diag << " FTN-" << except->footNote();

    if (!except->cur().empty())
      *_diag << " CUR-" << except->cur();

    if (!except->fareClass().empty())
      *_diag << " FC-" << except->fareClass();

    if (!except->fareType().empty())
      *_diag << " FT-" << except->fareType();

    if (except->owrt() != ' ')
      *_diag << " OWRT-" << owrtChar(except->owrt());

    *_diag << endl;
  }

  if (appl.selectionType() == INDUSTRY)
  {
    if (basisMod != nullptr)
    {
      *_diag << "OVERRIDE INFO: CXR-" << basisMod->carrier() << " USER APPL-"
             << basisMod->userAppl() << endl;
    }

    if ((basisMod != nullptr) && (basisMod->doNotChangeFareBasisInd() == DO_NOT_CHANGE_FARE_BASIS))
    {
      *_diag << "DO NOT CHANGE 1ST CHAR FARE BASIS TO MATCH CARRIER BOOKING CODE" << endl;
    }
    else
    {
      *_diag << "CHANGE 1ST CHAR FARE BASIS TO MATCH CARRIER BOOKING CODE" << endl;
    }
  }

  *_diag << " " << endl;
}

bool
IndustryFareController::match(const Fare& fare, const PaxTypeFare& yyFare) const
{
  if (fare.isIndustry())
    return false;

  if (fare.carrier() != _fareMarket.governingCarrier())
    return false;

  if (fare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
    return false;

  if ((fare.directionality() == FROM && yyFare.fare()->directionality() == TO) ||
      (fare.directionality() == TO && yyFare.fare()->directionality() == FROM))
    return false;

  return true;
}

bool
IndustryFareController::initialMatchOWRT(const Indicator& owrt1, const Indicator& owrt2)
{
  if ((owrt1 == ONE_WAY_MAYNOT_BE_DOUBLED &&
       owrt2 != ONE_WAY_MAYNOT_BE_DOUBLED) ||
      (owrt1 != ONE_WAY_MAYNOT_BE_DOUBLED &&
       owrt2 == ONE_WAY_MAYNOT_BE_DOUBLED))
    return false;

  return true;
}

bool
IndustryFareController::govCarrierHasFares(const PaxTypeFare& yyFare, const Indicator& yyFareAppl)
{
  if (LIKELY(_trx.isFootNotePrevalidationEnabled()))
  {
    for (PaxTypeFare* ptFare : _fareMarket.allPaxTypeFare())
    {
      if (!match(*(ptFare->fare()), yyFare))
        continue;

      if (yyFareAppl == ALLOW_WHEN_NO_CARRIER)
        return true;
      else
      {
        if (!initialMatchOWRT(ptFare->owrt(), yyFare.owrt()))
          continue;

        if (RuleUtil::matchFareType(yyFare.fcaFareType(), ptFare->fcaFareType()))
        {
          if (ptFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED &&
              yyFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
          {
            _matchFareTypeOfTag2CxrFare = true;
            continue;
          }
          return true;
        }
      }
    }

    if (LIKELY(_trx.isFootNotePrevalidationAllowed()))
    {
      if (fallback::fallbackFixFNFailedFares(&_trx))
      {
        for (auto fareMark : _fareMarket.footNoteFailedFares())
        {
          Fare* fare = fareMark.first;

          if (!match(*fare, yyFare))
            continue;

          if (yyFareAppl == ALLOW_WHEN_NO_CARRIER)
            return true;

          if (!initialMatchOWRT(fare->owrt(), yyFare.owrt()))
            continue;

          std::vector<PaxTypeFare*> ptFares;
          resolveFareClassApp(*fare, ptFares);

          _fareMarket.footNoteFailedPaxTypeFares().insert(_fareMarket.footNoteFailedPaxTypeFares().end(), ptFares.begin(), ptFares.end());
          fareMark.second = true;

          for (auto ptFare : ptFares)
          {
             if (RuleUtil::matchFareType(yyFare.fcaFareType(), ptFare->fcaFareType()))
             {
               if (ptFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED &&
                   yyFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
               {
                 _matchFareTypeOfTag2CxrFare = true;
                 continue;
               }
               return true;
             }
          }
        }
      }
      else
      {
        for (auto& fareMark : _fareMarket.footNoteFailedFares())
        {
          Fare* fare = fareMark.first;

          if (!match(*fare, yyFare))
            continue;

          if (yyFareAppl == ALLOW_WHEN_NO_CARRIER)
            return true;

          if (!initialMatchOWRT(fare->owrt(), yyFare.owrt()))
            continue;

          std::vector<PaxTypeFare*> ptFares;
          resolveFareClassApp(*fare, ptFares);

          _fareMarket.footNoteFailedPaxTypeFares().insert(_fareMarket.footNoteFailedPaxTypeFares().end(), ptFares.begin(), ptFares.end());
          fareMark.second = true;

          for (auto ptFare : ptFares)
          {
             if (RuleUtil::matchFareType(yyFare.fcaFareType(), ptFare->fcaFareType()))
             {
               if (ptFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED &&
                   yyFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
               {
                 _matchFareTypeOfTag2CxrFare = true;
                 continue;
               }
               return true;
             }
          }
        }
      }
    }
  }
  else
  {
    vector<PaxTypeFare*>::iterator i = _fareMarket.allPaxTypeFare().begin();
    vector<PaxTypeFare*>::iterator j = _fareMarket.allPaxTypeFare().end();

    PaxTypeFare* ptFare;

    CarrierCode& govCarrier = _fareMarket.governingCarrier();

    for (; i != j; ++i)
    {
      ptFare = *i;

      if (ptFare->fare()->isIndustry())
        continue;

      if (ptFare->carrier() != govCarrier)
        continue;

      if (ptFare->tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
        continue;

      if ((ptFare->fare()->directionality() == FROM && yyFare.fare()->directionality() == TO) ||
          (ptFare->fare()->directionality() == TO && yyFare.fare()->directionality() == FROM))
        continue;

      if (yyFareAppl == ALLOW_WHEN_NO_CARRIER)
        return true;
      else
      {
        if ((ptFare->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED &&
             yyFare.owrt() != ONE_WAY_MAYNOT_BE_DOUBLED) ||
            (ptFare->owrt() != ONE_WAY_MAYNOT_BE_DOUBLED &&
             yyFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED))
          continue;

        if (RuleUtil::matchFareType(yyFare.fcaFareType(), ptFare->fcaFareType()))
        {
          if (ptFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED &&
              yyFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
          {
            _matchFareTypeOfTag2CxrFare = true;
            continue;
          }
          return true;
        }
      }
    }
  }
  return false;
}

void
IndustryFareController::diag273(const FareMarket& fareMarket)
{
  if (LIKELY(_diag == nullptr))
    return;

  if (LIKELY(_trx.diagnostic().diagnosticType() != Diagnostic273 || !_diag->isActive()))
    return;

  // Output FM line
  *_diag << "****************************************************************" << endl;
  *_diag << FareMarketUtil::getFullDisplayString(fareMarket) << endl;
  *_diag << "****************************************************************" << endl;
}

void
IndustryFareController::diag273(const PaxTypeFare& paxTypeFare, const bool resolveOk)
{
  const FareClassAppInfo* fcai = nullptr;
  if (LIKELY(!paxTypeFare.isFareClassAppMissing()))
    fcai = paxTypeFare.fareClassAppInfo();

  const FareClassAppSegInfo* fcasi = nullptr;
  if (LIKELY(!paxTypeFare.isFareClassAppSegMissing()))
    fcasi = paxTypeFare.fareClassAppSegInfo();

  diag273(*paxTypeFare.fare(), fcai, fcasi, true, resolveOk);
}

void
IndustryFareController::diag273(const Fare& fare,
                                const FareClassAppInfo* fcai,
                                const FareClassAppSegInfo* fcasi,
                                const bool globalOk,
                                const bool resolveOk)
{
  _show273 = false;

  if (LIKELY(_diag == nullptr))
    return;

  if (LIKELY(_trx.diagnostic().diagnosticType() != Diagnostic273 || !_diag->isActive()))
    return;

  _show273 = true;

  _diag->setf(std::ios::left, std::ios::adjustfield);

  *_diag << ' ' << endl;

  *_diag << "----------------------------------------------------------------" << endl;

  *_diag << "VEND-" << fare.vendor() // 5+4 =  9
         << " FCLS-" << fare.fareClass() // 6+8   14
         << " FTRF-" << fare.fareTariff() // 6+6   12
         << " RULE-" << fare.ruleNumber(); // 6+4   10

  *_diag << " FTYP-";
  if (fcai != nullptr)
    *_diag << fcai->_fareType; // 6+3    9
  else
    *_diag << "***";

  *_diag << " DIR-" << directionalityChar(fare.directionality()) // 5+1    6
         << endl; // Total 60

  *_diag << "GD-" << globalDirectionString(fare.globalDirection()) // 3+2 =  5
         << " LOC1-" << fare.origin() // 6+5   11
         << " LOC2-" << fare.destination() // 6+5   11
         << " FN1-" << fare.footNote1() // 5+2    7
         << " FN2-" << fare.footNote2() // 5+2    7
         << " CUR-" << fare.currency() // 5+3    8
         << endl; // Total 49

  *_diag << "RTNG-" << fare.routingNumber() // 5+4 = 9
         << " OWRT-" << owrtChar(fare.owrt()) // 6+1   7
         << endl; // Total 16

  if (!globalOk)
  {
    *_diag << "FAILED GLOBAL DIRECTION" << endl;
  }

  if (!resolveOk)
  {
    *_diag << "FAILED RESOLVE - ";

    if (fcai == nullptr)
      *_diag << "FARECLASSAPP MISSING" << endl;
    else if (fcasi == nullptr)
      *_diag << "FARECLASSAPPSEG MISSING" << endl;
  }
}

void
IndustryFareController::diag273(const IndustryFareAppl& indFareAppl)
{
  if (_trx.diagnostic().diagnosticType() != Diagnostic273 || !_diag || !_diag->isActive())
    return;

  if (!_show273)
    return;

  *_diag << ' ' << endl;
  *_diag << "IFA ";

  // Output Industry Fare Appl minus the status
  if (!indFareAppl.vendor().empty())
    *_diag << "VEND-" << indFareAppl.vendor() << ' ';

  if (!indFareAppl.fareClass().empty())
    *_diag << "FCLS-" << indFareAppl.fareClass() << ' ';

  if (indFareAppl.fareTariff() != -1)
    *_diag << "FTRF-" << indFareAppl.fareTariff() << ' ';

  if (!indFareAppl.rule().empty())
    *_diag << "RULE-" << indFareAppl.rule() << ' ';

  if (!indFareAppl.fareType().empty())
    *_diag << "FTYP-" << indFareAppl.fareType() << ' ';

  if (indFareAppl.globalDir() != GlobalDirection::XX &&
      indFareAppl.globalDir() != GlobalDirection::ZZ)
    *_diag << "GD-" << globalDirectionString(indFareAppl.globalDir()) << ' ';

  char dir = directionalityChar(indFareAppl.directionality());
  if (dir != ' ')
    *_diag << "DIR-" << dir << ' ';

  if (!indFareAppl.loc1().loc().empty() && indFareAppl.loc1().locType() != ' ' &&
      indFareAppl.loc1().locType() != 0)
    *_diag << "LOC1-" << indFareAppl.loc1().locType() << ":" << indFareAppl.loc1().loc() << ' ';

  if (!indFareAppl.loc2().loc().empty() && indFareAppl.loc2().locType() != ' ' &&
      indFareAppl.loc2().locType() != 0)
    *_diag << "LOC2-" << indFareAppl.loc2().locType() << ":" << indFareAppl.loc2().loc() << ' ';

  if (!indFareAppl.footNote().empty())
    *_diag << "FN-" << indFareAppl.footNote() << ' ';

  if (!indFareAppl.cur().empty())
    *_diag << "CUR-" << indFareAppl.cur() << ' ';

  if (!indFareAppl.routing().empty())
    *_diag << "RTNG-" << indFareAppl.routing() << ' ';

  char owrt = owrtChar(indFareAppl.owrt());
  if (owrt != ' ')
    *_diag << "OWRT-" << owrt << ' ';

  if (!indFareAppl.carrier().empty())
    *_diag << "CXR-" << indFareAppl.carrier() << ' ';

  if (indFareAppl.seqNo() > 0)
    *_diag << "SEQ-" << indFareAppl.seqNo() << ' ';

  *_diag << "TYPE-" << indFareAppl.selectionType() << ' ';

  *_diag << endl;

  *_diag << "YYAPPL-" << yyFareApplString(indFareAppl.yyFareAppl()) << ' ';
}

void
IndustryFareController::diag273(const IndustryFareAppl::ExceptAppl& except)
{
  if (LIKELY(_diag == nullptr))
    return;

  if (LIKELY(_trx.diagnostic().diagnosticType() != Diagnostic273 || !_diag->isActive()))
    return;

  if (!_show273)
    return;

  *_diag << ' ' << endl;
  *_diag << "IFE ";

  // Output Industry Fare Appl minus the status
  if (!except.vendor().empty())
    *_diag << "VEND-" << except.vendor() << ' ';

  if (!except.fareClass().empty())
    *_diag << "FCLS-" << except.fareClass() << ' ';

  if (except.fareTariff() != -1)
    *_diag << "FTRF-" << except.fareTariff() << ' ';

  if (!except.rule().empty())
    *_diag << "RULE-" << except.rule() << ' ';

  if (!except.fareType().empty())
    *_diag << "FTYP-" << except.fareType() << ' ';

  if (except.globalDir() != GlobalDirection::XX && except.globalDir() != GlobalDirection::ZZ)
    *_diag << "GD-" << globalDirectionString(except.globalDir()) << ' ';

  char dir = directionalityChar(except.directionality());
  if (dir != ' ')
    *_diag << "DIR-" << dir << ' ';

  if (!except.loc1().loc().empty() && except.loc1().locType() != ' ' &&
      except.loc1().locType() != 0)
    *_diag << "LOC1-" << except.loc1().locType() << ":" << except.loc1().loc() << ' ';

  if (!except.loc2().loc().empty() && except.loc2().locType() != ' ' &&
      except.loc2().locType() != 0)
    *_diag << "LOC2-" << except.loc2().locType() << ":" << except.loc2().loc() << ' ';

  if (!except.footNote().empty())
    *_diag << "FN-" << except.footNote() << ' ';

  if (!except.cur().empty())
    *_diag << "CUR-" << except.cur() << ' ';

  if (!except.routing().empty())
    *_diag << "RTNG-" << except.routing() << ' ';

  char owrt = owrtChar(except.owrt());
  if (owrt != ' ')
    *_diag << "OWRT-" << owrt << ' ';

  if (!except.exceptCarrier().empty())
    *_diag << "CXR-" << except.exceptCarrier() << ' ';

  if (except.itemNo() >= 0) // These start at 0 not 1
    *_diag << "ITEM-" << except.itemNo() << ' ';

  *_diag << endl;
  *_diag << "YYAPPL-" << yyFareApplString(except.yyFareAppl()) << ' ';
}

void
IndustryFareController::diag273(const char* msg)
{
  if (LIKELY(_diag == nullptr))
    return;

  if (LIKELY(_trx.diagnostic().diagnosticType() != Diagnostic273 || !_diag->isActive()))
    return;

  if (!_show273)
    return;

  *_diag << "RESULT-" << msg << endl;
}

char
IndustryFareController::directionalityChar(const Indicator& dir)
{
  if (dir == FROM)
    return 'O';
  else if (dir == TO)
    return 'I';
  else
    return ' ';
}

char
IndustryFareController::owrtChar(const Indicator& owrt)
{
  if (owrt == ONE_WAY_MAY_BE_DOUBLED)
    return 'X';
  else if (owrt == ROUND_TRIP_MAYNOT_BE_HALVED)
    return 'R';
  else if (owrt == ONE_WAY_MAYNOT_BE_DOUBLED)
    return 'O';
  else
    return ' ';
}

string
IndustryFareController::globalDirectionString(const GlobalDirection& globalDir)
{
  string globalDirStr;
  globalDirectionToStr(globalDirStr, globalDir);

  return globalDirStr;
}

const char*
IndustryFareController::yyFareApplString(const Indicator& appl)
{
  switch (appl)
  {
  case EXCLUDE_ALL_PRICING_FARE_QUOTE:
    return "EXCL ALL YY FARES";

  case EXCLUDE_ALL_PRICING:
    return "EXCL ALL YY FARES";

  case ALLOW_WHEN_NO_CARRIER:
    return "ALLOW YY WHEN NO CXR";

  case ALLOW_WHEN_NO_CARRIER_OF_TYPE:
    return "ALLOW YY WHEN NO CXR TYPE";

  case ALLOW_ALL:
    return "ALLOW ALL YY FARES";

  default:
    break;
  }

  return "";
}

bool
IndustryFareController::matchFares(const std::vector<Fare*>& fares,
                                   const std::vector<const IndustryFareAppl*>& multiFareAppls,
                                   const std::vector<const IndustryFareAppl*>& indFareAppls,
                                   std::vector<PaxTypeFare*>& matchedFares,
                                   const std::vector<PaxTypeFare*>* specifiedFares)
{
  vector<Fare*>::const_iterator i = fares.begin();
  vector<Fare*>::const_iterator j = fares.end();

  const GlobalDirection& goodGlobal = _fareMarket.getGlobalDirection();

  FareClassCode matchClass;

  if (isIndustryFareDiagnostic(_trx.diagnostic().diagnosticType()))
  {
    matchClass = _trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);
  }

  for (; i != j; ++i)
  {
    Fare* fare = *i;
    if (UNLIKELY(fare == nullptr))
      continue;

    if (!matchClass.empty())
    {
      if (!RuleUtil::matchFareClass(fare->fareClass().c_str(), matchClass.c_str()))
        continue;
    }

    GlobalDirection gd = fare->globalDirection();
    // if( gd != XX && gd != ZZ && gd != goodGlobal)
    if (goodGlobal != GlobalDirection::XX && goodGlobal != GlobalDirection::ZZ && gd != goodGlobal)
    {
      _diag->enable(Diagnostic273);
      diag273(*fare, nullptr, nullptr, false, true);
      continue;
    }

    vector<PaxTypeFare*> ptFares;

    if (UNLIKELY(!resolveFareClassApp(*fare, ptFares)))
    {
      LOG4CXX_DEBUG(_logger, "IndustryFareController - resolveFareClassApp failed, skipping fare")

      diag273(*fare, nullptr, nullptr, true, false);
      continue;
    }

    LOG4CXX_DEBUG(_logger,
                  "IndustryFareController - resolveFareClassApp for fare"
                      << " V:" << fare->vendor() << " C:" << fare->carrier()
                      << " T:" << fare->fareTariff() << " R:" << fare->ruleNumber() << " returned "
                      << ptFares.size() << " PaxTypeFares");

    bool fareIsMulti = false;

    // SITA fares have the Multilateral flag set properly, ATP fares must check the rule
    // the ATP Fare object doesnt have the multiLateralInd() function so we cant call it
    if (fare->vendor() == SITA_VENDOR_CODE)
    {
      fareIsMulti = (fare->multiLateralInd() == YES);
    }
    else
    {
      fareIsMulti = _trx.dataHandle().isMultilateral(fare->vendor(),
                                                     fare->ruleNumber(),
                                                     fare->origin(),
                                                     fare->destination(),
                                                     _fareMarket.travelDate());

      if (UNLIKELY(IS_DEBUG_ENABLED(_logger)))
      {
        std::ostringstream oss;
        oss << "IndustryFareController - Call to dataHandle.isMultilateral for"
            << " vendor '" << fare->vendor() << "'"
            << ",rule '" << fare->ruleNumber() << "'"
            << ",origin '" << fare->origin() << "'"
            << ",dest '" << fare->destination() << "'"
            << ",ticketDate '" << _fareMarket.travelDate() << "'"
            << " returned '" << fareIsMulti << "'";
        _logger->debug(oss.str());
      }
    }

    vector<PaxTypeFare*>::iterator k = ptFares.begin();
    vector<PaxTypeFare*>::iterator l = ptFares.end();

    for (; k != l; ++k)
    {
      PaxTypeFare* newPTFare = *k;

      _diag->enable(Diagnostic273);
      diag273(*newPTFare, newPTFare->isValid());

      if (!newPTFare->isValid())
        continue;

      IndustryFare* indFare = nullptr;

      // First try to find a multilateral fare
      if (UNLIKELY(fareIsMulti && !multiFareAppls.empty()))
      {
        LOG4CXX_DEBUG(_logger, "IndustryFareController - Attempting to find multilateral fares")

        _diag->enable(Diagnostic271, Diagnostic273, Diagnostic277);
        indFare = matchFare(*newPTFare, multiFareAppls);
      }

      // If that fails try to find an industry fare
      if (LIKELY(!fareIsMulti && !indFareAppls.empty()))
      {
        LOG4CXX_DEBUG(_logger, "IndustryFareController - Attempting to find industry fares")

        _diag->enable(Diagnostic272, Diagnostic273, Diagnostic277);
        indFare = matchFare(*newPTFare, indFareAppls, specifiedFares);
      }

      if (indFare == nullptr)
      {
        LOG4CXX_DEBUG(_logger, "IndustryFareController - no matching appl found, skipping fare")
        continue;
      }

      newPTFare->setFare(indFare);
      // TODO push back pax type fares.. fix all interfaces!
      // CHANGE call above to createPaxTypeFares to validateAndAddPaxTypeFares()!!!

      matchedFares.push_back(newPTFare);
    }
  }

  return true;
}

bool
IndustryFareController::matchFareTariff(const TariffNumber& appl, const TariffNumber& fare)
{
  if (appl == -1)
    return true;

  return fare == appl;
}

bool
IndustryFareController::matchVendor(const VendorCode& appl,
                                    const Indicator userApplType,
                                    const std::string& userAppl,
                                    const VendorCode& fare)
{
  bool cxrMatch = (appl.empty() || appl == fare);

  bool crsMatch = ((userApplType == ' ' && userAppl.empty()) ||
                   (_userApplType == userApplType && _userAppl == userAppl));

  return (cxrMatch && crsMatch);
}

bool
IndustryFareController::matchDir(const Indicator& applDir,
                                 const LocKey& applLoc1,
                                 const LocKey& applLoc2,
                                 const LocCode& fareOrigin,
                                 const LocCode& fareDest)
{
  // If the direction is empty or both of the locs are empty it matches
  if ((' ' == applDir) ||
      ((applLoc1.locType() == LOCTYPE_NONE) && (applLoc2.locType() == LOCTYPE_NONE)))
    return true;

  GeoTravelType geoTvlType = _fareMarket.geoTravelType();

  if (applDir == FareController::APPL_DIR_WITHIN)
  {
    if (!LocUtil::isInLoc(fareOrigin,
                          applLoc1.locType(),
                          applLoc1.loc(),
                          Vendor::SABRE,
                          MANUAL,
                          geoTvlType,
                          LocUtil::OTHER,
                          _trx.getRequest()->ticketingDT()) ||
        !LocUtil::isInLoc(fareDest,
                          applLoc1.locType(),
                          applLoc1.loc(),
                          Vendor::SABRE,
                          MANUAL,
                          geoTvlType,
                          LocUtil::OTHER,
                          _trx.getRequest()->ticketingDT()))
    {
      return false;
    }

    return true;
  }
  else if (applDir == FareController::APPL_DIR_FROM)
  {
    if (!LocUtil::isInLoc(fareOrigin,
                          applLoc1.locType(),
                          applLoc1.loc(),
                          Vendor::SABRE,
                          MANUAL,
                          geoTvlType,
                          LocUtil::OTHER,
                          _trx.getRequest()->ticketingDT()) ||
        !LocUtil::isInLoc(fareDest,
                          applLoc2.locType(),
                          applLoc2.loc(),
                          Vendor::SABRE,
                          MANUAL,
                          geoTvlType,
                          LocUtil::OTHER,
                          _trx.getRequest()->ticketingDT()))
    {
      return false;
    }

    return true;
  }
  else if (LIKELY(applDir == FareController::APPL_DIR_BETWEEN))
  {
    if (!LocUtil::isInLoc(fareOrigin,
                          applLoc1.locType(),
                          applLoc1.loc(),
                          Vendor::SABRE,
                          MANUAL,
                          geoTvlType,
                          LocUtil::OTHER,
                          _trx.getRequest()->ticketingDT()) ||
        !LocUtil::isInLoc(fareDest,
                          applLoc2.locType(),
                          applLoc2.loc(),
                          Vendor::SABRE,
                          MANUAL,
                          geoTvlType,
                          LocUtil::OTHER,
                          _trx.getRequest()->ticketingDT()))
    {
      // Try swithed
      if (!LocUtil::isInLoc(fareOrigin,
                            applLoc2.locType(),
                            applLoc2.loc(),
                            Vendor::SABRE,
                            MANUAL,
                            geoTvlType,
                            LocUtil::OTHER,
                            _trx.getRequest()->ticketingDT()) ||
          !LocUtil::isInLoc(fareDest,
                            applLoc1.locType(),
                            applLoc1.loc(),
                            Vendor::SABRE,
                            MANUAL,
                            geoTvlType,
                            LocUtil::OTHER,
                            _trx.getRequest()->ticketingDT()))
      {
        return false;
      }
    }

    return true;
  }
  else if (applDir == FareController::APPL_DIR_FROM || applDir == FareController::APPL_DIR_ANY)
  {
    return true;
  }

  return false;
}

bool
IndustryFareController::matchRule(const RuleNumber& appl, const RuleNumber& fare)
{
  if (appl.empty())
    return true;

  return fare == appl;
}

bool
IndustryFareController::matchGlobal(const GlobalDirection& appl, const GlobalDirection& fare)
{
  if ((appl == GlobalDirection::ZZ) || (appl == ' '))
    return true;

  return fare == appl;
}

bool
IndustryFareController::matchCur(const CurrencyCode& appl, const CurrencyCode& fare)
{
  if (LIKELY(appl.empty()))
    return true;

  return fare == appl;
}

bool
IndustryFareController::matchFareType(const FareType& appl, const FareType& fare)
{
  if (appl.empty())
    return true;

  return RuleUtil::matchFareType(appl, fare);
}

bool
IndustryFareController::matchFareClass(const FareClassCode& appl, const FareClassCode& fare)
{
  if (appl.empty())
    return true;

  return RuleUtil::matchFareClass(appl.c_str(), fare.c_str());
}

bool
IndustryFareController::matchFootnote(const Footnote& appl,
                                      const Footnote& fare1,
                                      const Footnote& fare2)
{
  if (LIKELY(appl.empty()))
    return true;

  return ((appl == fare1) || (appl == fare2));
}

bool
IndustryFareController::matchRouting(const RoutingNumber& appl, const RoutingNumber& fare)
{
  if (appl.empty())
    return true;

  return (fare == appl);
}

bool
IndustryFareController::matchOWRT(const Indicator& appl, const Indicator& fare)
{
  if (LIKELY(appl == ' '))
    return true;

  return (fare == appl);
}

//-------------------------------------------------------------------
// matchIndAppl()  true = match
//              no check of appl field (negative match determined by caller)
//-------------------------------------------------------------------
bool
IndustryFareController::matchIndAppl(const PaxTypeFare& paxTypeFare,
                                     const IndustryFareAppl& appl,
                                     std::string& reason)
{
  reason = "";
  const Fare& fare = *paxTypeFare.fare();

  if (!IndustryFareController::matchVendor(
          appl.vendor(), appl.userApplType(), appl.userAppl(), fare.vendor()))
  {
    reason = "FAIL VENDOR";
    return false;
  }

  if (!IndustryFareController::matchFareTariff(appl.fareTariff(), fare.fareTariff()))
  {
    reason = "FAIL TARIFF";
    return false;
  }

  if (!IndustryFareController::matchRule(appl.rule(), fare.ruleNumber()))
  {
    reason = "FAIL RULE";
    return false;
  }

  if (!IndustryFareController::matchGlobal(appl.globalDir(), fare.globalDirection()))
  {
    reason = "FAIL GD";
    return false;
  }

  if (!IndustryFareController::matchDir(
          appl.directionality(), appl.loc1(), appl.loc2(), fare.origin(), fare.destination()))
  {
    reason = "FAIL LOC/DIR";
    return false;
  }

  if (UNLIKELY(!IndustryFareController::matchCur(appl.cur(), fare.currency())))
  {
    reason = "FAIL CURRENCY";
    return false;
  }

  if (!IndustryFareController::matchFareType(appl.fareType(),
                                             paxTypeFare.fareClassAppInfo()->_fareType))
  {
    reason = "FAIL FARETYPE";
    return false;
  }

  if (!IndustryFareController::matchFareClass(appl.fareClass(), fare.fareClass()))
  {
    reason = "FAIL FARECLASS";
    return false;
  }

  if (UNLIKELY(!IndustryFareController::matchFootnote(appl.footNote(), fare.footNote1(), fare.footNote2())))
  {
    reason = "FAIL FOOTNOTE";
    return false;
  }

  if (!IndustryFareController::matchRouting(appl.routing(), fare.routingNumber()))
  {

    reason = "FAIL ROUTING";
    return false;
  }

  if (UNLIKELY(!IndustryFareController::matchOWRT(appl.owrt(), fare.owrt())))
  {
    reason = "FAIL OWRT";
    return false;
  }

  LOG4CXX_DEBUG(_logger,
                "IndustryFareController - matchFare passed initial checks "
                    << "Carrier='" << fare.carrier() << "' Rule='" << fare.ruleNumber()
                    << "' Class='" << fare.fareClass());

  return true;
}

//-------------------------------------------------------------------
// matchIndExceptAppl()  true = match
//              no check of appl field (negative match determined by caller)
//-------------------------------------------------------------------
bool
IndustryFareController::matchIndExceptAppl(const PaxTypeFare& paxTypeFare,
                                           const IndustryFareAppl::ExceptAppl& except,
                                           std::string& reason)
{
  reason = "";
  const Fare& fare = *paxTypeFare.fare();

  if (UNLIKELY(except.exceptCarrier() != _fareMarket.governingCarrier()))
    return false;

  if (UNLIKELY(!IndustryFareController::matchFareTariff(except.fareTariff(), fare.fareTariff())))
  {
    reason = "FAIL TARIFF";
    return false;
  }

  if (!IndustryFareController::matchVendor(
          except.vendor(), except.userApplType(), except.userAppl(), fare.vendor()))
  {
    reason = "FAIL VENDOR";
    return false;
  }

  if (!IndustryFareController::matchDir(
          except.directionality(), except.loc1(), except.loc2(), fare.origin(), fare.destination()))
  {
    reason = "FAIL LOC/DIR";
    return false;
  }

  if (UNLIKELY(!IndustryFareController::matchGlobal(except.globalDir(), fare.globalDirection())))
  {
    reason = "FAIL GD";
    return false;
  }

  if (UNLIKELY(!IndustryFareController::matchRule(except.rule(), fare.ruleNumber())))
  {
    reason = "FAIL RULE";
    return false;
  }

  if (UNLIKELY(!IndustryFareController::matchRouting(except.routing(), fare.routingNumber())))
  {
    reason = "FAIL ROUTING";
    return false;
  }

  if (UNLIKELY(!IndustryFareController::matchFootnote(except.footNote(), fare.footNote1(), fare.footNote2())))
  {
    reason = "FAIL FOOTNOTE";
    return false;
  }

  if (UNLIKELY(!IndustryFareController::matchCur(except.cur(), fare.currency())))
  {
    reason = "FAIL CURRENCY";
    return false;
  }

  if (!IndustryFareController::matchFareClass(except.fareClass(), fare.fareClass()))
  {
    reason = "FAIL FARECLASS";
    return false;
  }

  if (!IndustryFareController::matchFareType(except.fareType(),
                                             paxTypeFare.fareClassAppInfo()->_fareType))
  {
    reason = "FAIL FARETYPE";
    return false;
  }

  if (UNLIKELY(!IndustryFareController::matchOWRT(except.owrt(), fare.owrt())))
  {
    reason = "FAIL OWRT";
    return false;
  }

  return true;
}

bool
IndustryFareController::isIndustryFareDiagnostic(const DiagnosticTypes& diagType)
{
  if (UNLIKELY((diagType == Diagnostic271) || (diagType == Diagnostic272) || (diagType == Diagnostic273) ||
      (diagType == Diagnostic277)))
    return true;

  return false;
}
}
