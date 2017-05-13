//----------------------------------------------------------------------------
//
//
//  File:     MatchRuleLevelExclTable.cpp
//  Created:  8/20/2004
//  Authors:  Quan Ta
//
//  Description:
//          Function object to match Minimum Fare Rule Level Exclusion Table.
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
//----------------------------------------------------------------------------

#include "MinFares/MatchRuleLevelExclTable.h"
#include "Common/FallbackUtil.h"
#include "Common/DiagMonitor.h"
#include "Common/FallbackUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "MinFares/MinFareFareSelection.h"
#include "MinFares/MinFareLogic.h"
#include "MinFares/MinimumFare.h"
#include "Rules/RuleUtil.h"

#include <vector>

namespace tse
{
 
// Since STL passes the Predicate by value instead of by reference,
// We are creating a simple wrapper class that when passed in will
// only create a copy of the MatchRuleLevelExclTable pointer,
// instead of a copy of the class.
class MatchRuleLevelExclTablePredicate
{
public:
  MatchRuleLevelExclTablePredicate(MatchRuleLevelExclTable* table) { exclTable = table; }
  bool operator()(const MinFareRuleLevelExcl* rule) const { return exclTable->operator()(rule); }
  // We use the deafult copy constructor, so don't make a private one here.
private:
  MatchRuleLevelExclTable* exclTable;
};

MatchRuleLevelExclTable::MatchRuleLevelExclTable(MinimumFareModule module,
                                                 PricingTrx& trx,
                                                 const Itin& itin,
                                                 const PaxTypeFare& paxTypeFare,
                                                 const DateTime& tvlDate,
                                                 const bool checkSameFareGroup)
  : _module(module),
    _trx(trx),
    _itin(itin),
    _paxTypeFare(paxTypeFare),
    _tvlDate(tvlDate),
    _diag(nullptr),
    _diagEnabled(false),
    _fareForCat17(&paxTypeFare),
    _compCat17ItemOnly(false),
    _matchedRuleItem(nullptr),
    _setvalue(0),
    _checkSameFareGroup(checkSameFareGroup)
{
}

MatchRuleLevelExclTable::~MatchRuleLevelExclTable() {}

bool
MatchRuleLevelExclTable::
operator()()
{
  DiagMonitor diag(_trx, Diagnostic709);
  _diag = &diag.diag();
  _diagEnabled = (_diag && _diag->isActive());

  // Retrieve the rules from table
  int32_t table996No = RuleUtil::getCat17Table996ItemNo(_trx, _paxTypeFare, _fareForCat17);
  if (UNLIKELY(_fareForCat17 == nullptr))
    return false;

  if (_fareForCat17 != &_paxTypeFare) // Cat17 from base fare
  {
    _compCat17ItemOnly = true;

    if (_diagEnabled)
    {
      diag << " \n*********** " << MinFareFareSelection::MinFareModuleName[_module]
           << " MINIMUM FARE RULE LEVEL EXCL TABLE ***********\n"
           << "CAT 17 FROM BASE FARE\n"
           << "VENDOR/" << _fareForCat17->vendor() << " TBL996/" << table996No << " CXR/"
           << _fareForCat17->fareMarket()->governingCarrier() << " RULETRF/"
           << _fareForCat17->tcrRuleTariff() << " APPLYSAMEGRP/"
           << (_checkSameFareGroup ? "YES" : "NO") << " \n";

      if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
        MinimumFare::printRexDatesAdjustment(*_diag, _trx.dataHandle().ticketDate());

      MinimumFare::printFareInfo(*_fareForCat17, *_diag, _module);
    }

    // for base fare
    const std::vector<MinFareRuleLevelExcl*>& ruleList =
        _trx.dataHandle().getMinFareRuleLevelExcl(_fareForCat17->vendor(),
                                                  table996No,
                                                  _fareForCat17->fareMarket()->governingCarrier(),
                                                  _fareForCat17->tcrRuleTariff(),
                                                  _tvlDate);

    MatchRuleLevelExclTablePredicate predicate(this);
    std::vector<MinFareRuleLevelExcl*>::const_iterator ruleIter =
        find_if(ruleList.begin(), ruleList.end(), predicate);

    if (ruleIter != ruleList.end())
    {
      _matchedRuleItem = *ruleIter;

      return true;
    }
    else
    {
      table996No = 0; // Reset it
    }
  }

  // for Original fare
  _compCat17ItemOnly = false;

  if (UNLIKELY(_diagEnabled))
  {
    diag << " \n*********** " << MinFareFareSelection::MinFareModuleName[_module]
         << " MINIMUM FARE RULE LEVEL EXCL TABLE ***********\n"
         << "VENDOR/" << _paxTypeFare.vendor() << " TBL996/" << table996No << " CXR/"
         << _paxTypeFare.fareMarket()->governingCarrier() << " RULETRF/"
         << _paxTypeFare.tcrRuleTariff() << " APPLYSAMEGRP/" << (_checkSameFareGroup ? "YES" : "NO")
         << " \n";

    if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
      MinimumFare::printRexDatesAdjustment(*_diag, _trx.dataHandle().ticketDate());

    MinimumFare::printFareInfo(_paxTypeFare, *_diag, _module);
  }

  const std::vector<MinFareRuleLevelExcl*>& ruleList =
      _trx.dataHandle().getMinFareRuleLevelExcl(_paxTypeFare.vendor(),
                                                table996No,
                                                _paxTypeFare.fareMarket()->governingCarrier(),
                                                _paxTypeFare.tcrRuleTariff(),
                                                _tvlDate);

  MatchRuleLevelExclTablePredicate predicate(this);

  std::vector<MinFareRuleLevelExcl*>::const_iterator ruleIter =
      find_if(ruleList.begin(), ruleList.end(), predicate);

  if (ruleIter != ruleList.end())
  {
    _matchedRuleItem = *ruleIter;

    return true;
  }

  return false;
}

bool
MatchRuleLevelExclTable::
operator()(const MinFareRuleLevelExcl* rule)
{
  DiagCollector& diag = *_diag;

  if (UNLIKELY(_diagEnabled))
  {
    _diag->setf(std::ios::left, std::ios::adjustfield);
    diag << " \nVENDOR/" << std::setw(4) << rule->vendor() << " GOVCXR/" << std::setw(2)
         << rule->governingCarrier() << " SEQNO/" << rule->seqNo() << " TEXTTBLITEMNO/"
         << rule->textTblItemNo() << " RULTRF/" << rule->ruleTariff() << "\n";
  }

  const PaxTypeFare* compFare = (_compCat17ItemOnly ? _fareForCat17 : &_paxTypeFare);

  // Check if the rule level excl indicator for the current module is blank
  if (UNLIKELY(MinFareLogic::getMinFareExclIndicator(_module, *rule) == MinimumFare::BLANK))
  {
    return false;
  }

  // Check Tariff Category
  if (UNLIKELY(rule->tariffCat() != MinimumFare::ANY_TARIFF && compFare->tcrTariffCat() != rule->tariffCat()))
  {
    if (_diagEnabled)
    {
      diag << "FARE - TARIFF CATEGORY NOT MATCH " << compFare->tcrTariffCat() << " : "
           << rule->tariffCat() << '\n';
    }

    return false;
  }

 // Check Rule and Footnote
  const std::vector<std::pair<RuleNumber, Footnote> >& ruleFootnotes = rule->ruleFootnotes();
  if (!ruleFootnotes.empty())
  {
    bool everMatch = false;
    for(const auto& element: ruleFootnotes)
    {
      if (matchFareRule(element.first, *compFare) &&
                   matchFootnote(element.second, *compFare))
      {
        everMatch = true;
        break;
      }
    }
    if (!everMatch)
    {
      if (UNLIKELY(_diagEnabled))
      {
        diag << "FARE - RULE NUMBER NOT MATCH " << compFare->ruleNumber();
        if (ruleFootnotes.size() > 0)
        {
          diag << " : ";
          for(const auto& elem: ruleFootnotes)
          {
            diag << elem.first << "  " << elem.second << std::endl;
          }
        }
        diag << '\n';
      }
      return false;
    }
  }
  
  // Check through/terminate geo
  if (rule->loc1().locType() != MinimumFare::BLANK || rule->loc2().locType() != MinimumFare::BLANK)
  {
    const Directionality& directionality = rule->directionality();
    const std::vector<TravelSeg*>& travelSegVec = compFare->fareMarket()->travelSeg();
    bool reverseOrigin = ((_paxTypeFare.directionality() == TO) ||
                          (_paxTypeFare.fareMarket()->direction() == FMDirection::INBOUND));

    if (!MinimumFare::matchGeo(
            _trx, directionality, rule->loc1(), rule->loc2(), travelSegVec, _itin, reverseOrigin))
    {
      if (_diagEnabled)
      {
        diag << "DIR: " << directionality << " LOC1: [" << rule->loc1().loc() << ":"
             << rule->loc1().locType() << "] LOC2: [" << rule->loc2().loc() << ":"
             << rule->loc2().locType() << "]\n";
        diag << "THROUGH/TERMINATE LOCATION NOT MATCH " << travelSegVec.front()->boardMultiCity()
             << "-" << travelSegVec.back()->offMultiCity() << '\n';
      }

      return false;
    }
  }

  // Check Fare Class/Generic
  const std::vector<FareClassCode>& fareClasses = rule->fareClasses();
  std::vector<FareClassCode>::const_iterator fareClassIter = fareClasses.begin();
  for (; fareClassIter != fareClasses.end(); fareClassIter++)
  {
    if (matchFareClass(*fareClassIter, *compFare))
      break;
  }
  if (fareClasses.size() > 0 && fareClassIter == fareClasses.end())
  {
    if (_diagEnabled)
    {
      diag << "FARE - FARE CLASS NOT MATCH " << compFare->fareClass();
      if (fareClasses.size() > 0)
      {
        diag << " : ";
        std::copy(fareClasses.begin(),
                  fareClasses.end(),
                  std::ostream_iterator<FareClassCode>(diag, " "));
      }
      diag << '\n';
    }
    return false;
  }

  // Check Fare Type/Generic
  const std::vector<FareType>& fareTypes = rule->fareTypes();
  std::vector<FareType>::const_iterator fareTypeIter = fareTypes.begin();
  for (; fareTypeIter != fareTypes.end(); fareTypeIter++)
  {
    if (matchFareType(*fareTypeIter, *compFare))
      break;
  }

  if (fareTypes.size() > 0 && fareTypeIter == fareTypes.end())
  {
    if (_diagEnabled)
    {
      diag << "FARE - FARE TYPE NOT MATCH " << compFare->fcaFareType();
      if (fareTypes.size() > 0)
      {
        diag << " : ";
        std::copy(fareTypes.begin(), fareTypes.end(), std::ostream_iterator<FareType>(diag, " "));
      }
      diag << '\n';
    }
    return false;
  }

  // Check MPM Indicator
  if (rule->mpmInd() == YES && // Item only apply to MPM fare
      compFare->isRouting())
  {
    if (_diagEnabled)
    {
      diag << "FARE IS ROUTING FARE NOT MATCH " << '\n';
    }
    return false;
  }

  // Check Routing Application
  if (rule->routingInd() == YES && // Item only apply to Routing fare
      !compFare->isRouting())
  {
    if (_diagEnabled)
    {
      diag << "FARE IS MILEAGE FARE NOT MATCH " << '\n';
    }
    return false;
  }

  // Check Routing Tariff
  if (((rule->routingTariff1() != MinimumFare::ANY_TARIFF) &&
       (rule->routingTariff1() != compFare->tcrRoutingTariff1())) ||
      ((rule->routingTariff2() != MinimumFare::ANY_TARIFF) &&
       (rule->routingTariff2() != compFare->tcrRoutingTariff2())))
  {
    if (_diagEnabled)
    {
      diag << "FARE - ROUTING TARIFF NOT MATCH " << compFare->tcrRoutingTariff1() << " "
           << compFare->tcrRoutingTariff2() << '\n';
    }

    return false;
  }

  // Check Routing
  RoutingNumber rn = compFare->fcaRoutingNumber().empty()?
                     compFare->routingNumber() : compFare->fcaRoutingNumber();

  if (!RuleUtil::compareRoutings(rule->routing(), rn))
  {
    if (_diagEnabled)
      *_diag << "FARE - ROUTING NUMBER NOT MATCH " << _paxTypeFare.fcaRoutingNumber() << '\n';
    return false;
  }

  // Check Global Direction
  if (rule->globalDir() != GlobalDirection::ZZ &&
      rule->globalDir() != compFare->fareMarket()->getGlobalDirection())
  {
    if (_diagEnabled)
    {
      diag << "FARE - GLOBAL NOT MATCH ";
      std::string gd;
      globalDirectionToStr(gd, compFare->fareMarket()->getGlobalDirection());
      diag << gd << '\n';
    }
    return false;
  }

  if (_checkSameFareGroup && !checkRuleLevelExclusionSameFareGroupCheck(*rule, *compFare))
  {
    if (_diagEnabled)
      diag << "SAMEFAREGROUP NOT MATCHED" << '\n';

    return false;
  }

  if (_diagEnabled)
  {
    diag << "MATCHED RULE LEVEL EXCLUSION ITEM " << '\n';
    displayRuleLevel(*rule);
  }

  return true;
}

bool
MatchRuleLevelExclTable::checkRuleLevelExclusionSameFareGroupCheck(
    const MinFareRuleLevelExcl& ruleLevelExcl, const PaxTypeFare& paxTypeFare)
{
  // Check "Exclusion Checks" for "Do not use for fare comp check"
  switch (_module)
  {
  case HIP:
    if (ruleLevelExcl.hipFareCompAppl() == MinimumFare::NO &&
        ruleLevelExcl.hipSameGroupAppl() == MinimumFare::PERMITTED)
      return passRuleLevelExclusionSameGroup(ruleLevelExcl, paxTypeFare, false);
    else
      return true;

  case BHC:
    if (ruleLevelExcl.backhaulFareCompAppl() == MinimumFare::NO &&
        ruleLevelExcl.backhaulSameGroupAppl() == MinimumFare::PERMITTED)
      return passRuleLevelExclusionSameGroup(ruleLevelExcl, paxTypeFare, false);
    else
      return true;

  case CTM:
    if (ruleLevelExcl.ctmFareCompAppl() == MinimumFare::NO &&
        ruleLevelExcl.ctmSameGroupAppl() == MinimumFare::PERMITTED)
      return passRuleLevelExclusionSameGroup(ruleLevelExcl, paxTypeFare, false);
    else
      return true;
  default:
    return true;
  }
}

void
MatchRuleLevelExclTable::displayRuleLevel(const MinFareRuleLevelExcl& ruleLevelExcl) const
{
  DiagCollector& diag = *_diag;
  diag << "RULE TARIFF CATEGORY - " << ruleLevelExcl.tariffCat() << '\n' << "RULE TARIFF NUMBER - "
       << ruleLevelExcl.ruleTariff() << '\n';
	   
  const auto& footnote = ruleLevelExcl.ruleFootnotes();
  for (const auto& element: footnote)
  {
    diag << "RULE NUMBER - " << element.first << '\n';
    diag << "FOOTNOTE - " << element.second << '\n';
  }
  
  const std::vector<FareClassCode>& fareClasses = ruleLevelExcl.fareClasses();
  std::vector<FareClassCode>::const_iterator fareClassIter = fareClasses.begin();
  for (; fareClassIter != fareClasses.end(); fareClassIter++)
  {
    diag << "FARE CLASS - " << *fareClassIter << '\n';
  }

  const std::vector<FareType>& fareTypes = ruleLevelExcl.fareTypes();
  std::vector<FareType>::const_iterator fareTypeIter = fareTypes.begin();
  for (; fareTypeIter != fareTypes.end(); fareTypeIter++)
  {
    diag << "FARE TYPE - " << *fareTypeIter << '\n';
  }

  diag << "ROUTING TARIFF1 - " << ruleLevelExcl.routingTariff1() << '\n';

  diag << "ROUTING TARIFF2 - " << ruleLevelExcl.routingTariff2() << '\n';

  diag << "ROUTING NUMBER = " << ruleLevelExcl.routing() << '\n';

  diag << "GLOBAL DIRECTION - ";
  std::string gd;
  globalDirectionToStr(gd, ruleLevelExcl.globalDir());
  diag << gd << '\n';

  diag << "EXCLUSION CHECKS\n"
       << "  HIP DONOTAPPLY - " << ruleLevelExcl.hipMinFareAppl() << " DONOTUSEFORCOMP - "
       << ruleLevelExcl.hipFareCompAppl() << " APPLYSAMEGRPCOMP - "
       << ruleLevelExcl.hipSameGroupAppl() << "\n"
       << "  CTM DONOTAPPLY - " << ruleLevelExcl.ctmMinFareAppl() << " DONOTUSEFORCOMP - "
       << ruleLevelExcl.ctmFareCompAppl() << " APPLYSAMEGRPCOMP - "
       << ruleLevelExcl.ctmSameGroupAppl() << "\n"
       << "  BHC DONOTAPPLY - " << ruleLevelExcl.backhaulMinFareAppl() << " DONOTUSEFORCOMP - "
       << ruleLevelExcl.backhaulFareCompAppl() << " APPLYSAMEGRPCOMP - "
       << ruleLevelExcl.backhaulSameGroupAppl() << "\n"
       << "  DMC DONOTAPPLY - " << ruleLevelExcl.dmcMinFareAppl() << " DONOTUSEFORCOMP - "
       << ruleLevelExcl.dmcFareCompAppl() << " APPLYSAMEGRPCOMP - "
       << ruleLevelExcl.dmcSameGroupAppl() << "\n"
       << "  COM DONOTAPPLY - " << ruleLevelExcl.comMinFareAppl() << " DONOTUSEFORCOMP - "
       << ruleLevelExcl.comFareCompAppl() << " APPLYSAMEGRPCOMP - "
       << ruleLevelExcl.comSameGroupAppl() << "\n"
       << "  COP DONOTAPPLY - " << ruleLevelExcl.copMinFareAppl() << " DONOTUSEFORCOMP - "
       << ruleLevelExcl.copFareCompAppl() << " APPLYSAMEGRPCOMP - "
       << ruleLevelExcl.copSameGroupAppl() << "\n"
       << "  CPM DONOTAPPLY - " << ruleLevelExcl.cpmMinFareAppl() << " DONOTUSEFORCOMP - "
       << ruleLevelExcl.cpmFareCompAppl() << " APPLYSAMEGRPCOMP - "
       << ruleLevelExcl.cpmSameGroupAppl() << "\n"
       << "  OSC               "
       << " DONOTUSEFORCOMP - " << ruleLevelExcl.oscFareCompAppl() << " APPLYSAMEGRPCOMP - "
       << ruleLevelExcl.oscSameGroupAppl() << "\n"
       << "  RSC               "
       << " DONOTUSEFORCOMP - " << ruleLevelExcl.rscFareCompAppl() << " APPLYSAMEGRPCOMP - "
       << ruleLevelExcl.rscSameGroupAppl() << "\n";

  if (ruleLevelExcl.hipSameGroupAppl() == PERMITTED ||
      ruleLevelExcl.ctmSameGroupAppl() == PERMITTED ||
      ruleLevelExcl.backhaulSameGroupAppl() == PERMITTED)
  {
    const std::vector<SetNumber>& fareSetS = ruleLevelExcl.fareSet();
    const std::vector<TariffNumber>& sameFareGroupTariff = ruleLevelExcl.sameFareGroupTariff();
    const std::vector<RuleNumber>& sameFareGroupRules = ruleLevelExcl.sameFareGroupRules();
    const std::vector<FareClassCode>& sameFareGroupFareClasses =
        ruleLevelExcl.sameFareGroupFareClasses();
    const std::vector<FareType>& sameFareGroupFareTypes = ruleLevelExcl.sameFareGroupFareTypes();

    for (size_t i = 0;
         i < fareSetS.size() && i < sameFareGroupTariff.size() && i < sameFareGroupRules.size() &&
             i < sameFareGroupFareClasses.size() && i < sameFareGroupFareTypes.size();
         i++)
    {
      diag << "GRP - "
           << "SET - " << std::setw(5) << fareSetS[i] << "TRF - " << std::setw(5)
           << sameFareGroupTariff[i] << "RULE -" << std::setw(5) << sameFareGroupRules[i] << "FT - "
           << std::setw(3) << sameFareGroupFareTypes[i] << "FCLS - " << std::setw(8)
           << sameFareGroupFareClasses[i] << "\n";
    }
  }
}

bool
MatchRuleLevelExclTable::passRuleLevelExclusionSameGroup(const MinFareRuleLevelExcl& ruleLevelExcl,
                                                         const PaxTypeFare& paxTypeFare,
                                                         bool isThruFare)
{

  const std::vector<SetNumber>& fareSetS = ruleLevelExcl.fareSet();
  const std::vector<TariffNumber>& sameFareGroupTariff = ruleLevelExcl.sameFareGroupTariff();
  const std::vector<RuleNumber>& sameFareGroupRules = ruleLevelExcl.sameFareGroupRules();
  const std::vector<FareClassCode>& sameFareGroupFareClasses =
      ruleLevelExcl.sameFareGroupFareClasses();
  const std::vector<FareType>& sameFareGroupFareTypes = ruleLevelExcl.sameFareGroupFareTypes();

  for (size_t i = 0; i < fareSetS.size(); i++)
  {
    if (fareSetS[i] >= 1) // for invalid child records do not go inside
    {
      if (isThruFare && _setvalue != fareSetS[i])
        continue;

      if (matchSameGroup(sameFareGroupFareClasses[i],
                         sameFareGroupFareTypes[i],
                         sameFareGroupRules[i],
                         sameFareGroupTariff[i],
                         paxTypeFare))
      {
        if (!isThruFare)
          _setvalue = fareSetS[i];
        return true;
      }
    }
  }

  return false;
}

bool
MatchRuleLevelExclTable::matchSameGroup(const FareClassCode& sameFareGroupFC,
                                        const FareType& sameFareGroupFT,
                                        const RuleNumber& sameFareGroupRN,
                                        const TariffNumber& sameFareGroupRT,
                                        const PaxTypeFare& paxTypeFare)
{
  return (
      matchFareClass(sameFareGroupFC, paxTypeFare) && matchFareType(sameFareGroupFT, paxTypeFare) &&
      matchFareRule(sameFareGroupRN, paxTypeFare) && matchRuleTariff(sameFareGroupRT, paxTypeFare));
}

bool
MatchRuleLevelExclTable::matchFareType(const FareType& sameFareGroupFT,
                                       const PaxTypeFare& paxTypeFare) const
{
  if (sameFareGroupFT.empty() || sameFareGroupFT == " " ||
      RuleUtil::matchFareType(sameFareGroupFT, paxTypeFare.fcaFareType()))
    return true;

  return false;
}

bool
MatchRuleLevelExclTable::matchFareClass(const FareClassCode& sameFareGroupFC,
                                        const PaxTypeFare& paxTypeFare) const
{
  static const FareClassCode blank(" ");
  if (sameFareGroupFC.empty() || sameFareGroupFC == blank ||
      RuleUtil::matchFareClass(sameFareGroupFC.c_str(), paxTypeFare.fareClass().c_str()))
    return true;

  return false;
}

bool
MatchRuleLevelExclTable::matchFareRule(const RuleNumber& sameFareGroupRN,
                                       const PaxTypeFare& paxTypeFare) const
{
  if (sameFareGroupRN.empty() || sameFareGroupRN == " " ||
      sameFareGroupRN == paxTypeFare.ruleNumber())
    return true;

  return false;
}

bool
MatchRuleLevelExclTable::matchFootnote(const Footnote& fn,
                                       const PaxTypeFare& paxTypeFare) const
{
  if (fn.empty() || fn == " " ||
      fn == paxTypeFare.footNote1() || fn == paxTypeFare.footNote2())
    return true;

  return false;
}

bool
MatchRuleLevelExclTable::matchRuleTariff(const TariffNumber& sameFareGroupRT,
                                         const PaxTypeFare& paxTypeFare) const
{
  if (sameFareGroupRT == MinimumFare::ANY_TARIFF || sameFareGroupRT == paxTypeFare.tcrRuleTariff())
    return true;

  return false;
}
}
