//----------------------------------------------------------------------------
//
//
//  File:     MatchDefaultLogicTable.cpp
//  Created:  8/20/2004
//  Authors:  Quan Ta
//
//  Description:
//          Function object to Match Minimum Fare Default Logic Table.
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

#include "MinFares/MatchDefaultLogicTable.h"

#include "Common/DiagMonitor.h"
#include "Common/TrxUtil.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MinFareDefaultLogic.h"
#include "MinFares/MinFareFareSelection.h"
#include "MinFares/MinimumFare.h"

namespace tse
{
MatchDefaultLogicTable::MatchDefaultLogicTable(MinimumFareModule module,
                                               PricingTrx& trx,
                                               const Itin& itin,
                                               const PaxTypeFare& paxTypeFare)
  : _module(module),
    _trx(trx),
    _itin(itin),
    _paxTypeFare(paxTypeFare),
    _diag(nullptr),
    _diagEnabled(false),
    _matchedDefaultItem(nullptr)
{
}

MatchDefaultLogicTable::~MatchDefaultLogicTable() {}

bool
MatchDefaultLogicTable::
operator()()
{
  DiagMonitor diagMonitor(_trx, Diagnostic709);
  _diag = &diagMonitor.diag();
  _diagEnabled = (_diag && _diag->isActive());

  std::vector<MinFareDefaultLogic*> defLogicTable = _trx.dataHandle().getMinFareDefaultLogic(
      _paxTypeFare.vendor(), _paxTypeFare.fareMarket()->governingCarrier());

  if (UNLIKELY(_diagEnabled))
  {
    *_diag << " \n********** " << MinFareFareSelection::MinFareModuleName[_module]
           << " MINIMUM FARE LOGIC APPLICATION TABLE **********\n"
           << "VENDOR/" << _paxTypeFare.vendor() << " CXR/"
           << _paxTypeFare.fareMarket()->governingCarrier() << " \n";

    if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
      MinimumFare::printRexDatesAdjustment(*_diag, _trx.dataHandle().ticketDate());

    MinimumFare::printFareInfo(_paxTypeFare, *_diag, _module);
  }

  std::vector<MinFareDefaultLogic*>::iterator ruleIter =
      std::find_if(defLogicTable.begin(), defLogicTable.end(), *this);

  // Per Carrie, we will try to match the rule item with matched Gov Cxr first,
  // and default to the match all cxr if no match.
  if (ruleIter != defLogicTable.end() &&
      ((*ruleIter)->governingCarrier().empty() || (*ruleIter)->governingCarrier() == " "))
  {
    std::vector<MinFareDefaultLogic*>::iterator govCxrRuleIter =
        std::find_if(ruleIter + 1, defLogicTable.end(), *this);

    if (LIKELY(govCxrRuleIter != defLogicTable.end()))
    {
      ruleIter = govCxrRuleIter;
    }
  }

  if (LIKELY(ruleIter != defLogicTable.end()))
  {
    _matchedDefaultItem = *ruleIter;
    return true;
  }

  return false;
}

bool
MatchDefaultLogicTable::
operator()(const MinFareDefaultLogic* appl) const
{
  const PaxTypeFare& paxTypeFare = _paxTypeFare;
  const MinFareDefaultLogic& defaultAppl = *appl;

  if (UNLIKELY(_diagEnabled))
    *_diag << " \nVENDOR/" << std::setw(4) << defaultAppl.vendor() << " GOVCXR/" << std::setw(2)
           << defaultAppl.governingCarrier() << " SEQNO/" << defaultAppl.seqNo() << "\n";

  // Check carrier code (this matching should be done from dbaccess/cache
  const FareMarket& fareMarket = *(paxTypeFare.fareMarket());
  if (UNLIKELY(!(defaultAppl.governingCarrier().empty()) &&
      fareMarket.governingCarrier() != defaultAppl.governingCarrier()))
  {
    if (_diagEnabled)
      *_diag << "FARE - GOVERNING CARRIER NOT MATCH " << fareMarket.governingCarrier() << '\n';
    return false;
  }

  // thru/terminate geo matching
  if (defaultAppl.loc1().locType() != MinimumFare::BLANK ||
      defaultAppl.loc2().locType() != MinimumFare::BLANK)
  {
    const Directionality& directionality = defaultAppl.directionalInd();
    const FareMarket& fareMarket = *(paxTypeFare.fareMarket());
    const std::vector<TravelSeg*>& travelSegVec = fareMarket.travelSeg();
    bool reverseOrigin =
        ((paxTypeFare.directionality() == TO) || (fareMarket.direction() == FMDirection::INBOUND));

    if (!MinimumFare::matchGeo(_trx,
                               directionality,
                               defaultAppl.loc1(),
                               defaultAppl.loc2(),
                               travelSegVec,
                               _itin,
                               reverseOrigin))
    {
      if (_diagEnabled)
        *_diag << "THROUGH/TERMINATE LOCATION NOT MATCH "
               << (*(travelSegVec.begin()))->origin()->loc() << "-"
               << (*(travelSegVec.end() - 1))->destination()->loc() << '\n';

      return false;
    }
  }

  if (UNLIKELY(_diagEnabled))
  {
    *_diag << "MATCHED DEFAULT LOGIC APPLICATION ITEM\n";
    displayDefaultAppl(defaultAppl);
  }

  return true;
}

void
MatchDefaultLogicTable::displayDefaultAppl(const MinFareDefaultLogic& application) const
{
  if (_diagEnabled)
  {
    if ((_module == HIP) || (_module == BHC) || (_module == CTM))
    {
      *_diag << "HIP/CTM/BHC DOMESTIC APPLICATION - \n"
             << "  APPL/EXCLUDE ALL DOMESTIC: " << application.domAppl() << "\n"
             << "  EXCEPT: " << application.domExceptInd()
             << "  LOC: " << application.domLoc().locType() << " " << application.domLoc().loc()
             << "\n"
             << "  EXCEPT FARE TYPE: " << application.domFareTypeExcept() << "\n"
             << "  THROUGH FARE TYPE GENERIC: ";

      std::vector<FareTypeAbbrev>::const_iterator domFareTypeI = application.fareTypes().begin();
      for (; domFareTypeI != application.fareTypes().end(); ++domFareTypeI)
      {
        *_diag << *domFareTypeI;
      }

      *_diag << " \n";
    }

    if (_module == HIP)
    {
      *_diag << "NORMAL HIP - \n"
             << "  DETAILED CHECK POINTS - \n"
             << "    FROM GROUP 1              TO GROUP 2 \n"
             << "    ORIG                 " << application.nmlHipOrigInd() << "    DEST            "
             << application.nmlHipDestInd() << "\n"
             << "    ORIG NATION          " << application.nmlHipOrigNationInd()
             << "    DEST NATION     " << application.nmlHipDestNationInd() << "\n"
             << "    INTERM               " << application.nmlHipFromInterInd()
             << "    INTERM          " << application.nmlHipToInterInd() << "\n"
             << "  ADDITIONAL CHECK POINTS - \n"
             << "    FROM GROUP 1              TO GROUP 2 \n"
             << "    ORIG NATION TKT      " << application.nmlHipOrigNationTktPt()
             << "    TKT             " << application.nmlHipTicketedPt() << "\n"
             << "    ORIG NATION STOPOVER " << application.nmlHipOrigNationStopPt()
             << "    STOPOVER        " << application.nmlHipStopoverPt() << "\n"
             << "  EXEMPT INTERM TO INTERM: " << application.nmlHipExemptInterToInter() << "\n"
             << "SPECIAL HIP - \n"
             << "  SPECIAL ONLY:     " << application.spclHipSpclOnlyInd()
             << "  INTERM LOC BY PASS NML: " << application.spclHipLoc().locType() << " "
             << application.spclHipLoc().loc() << "\n"
             << "  DETAILED CHECK POINTS - \n"
             << "    FROM GROUP 1              TO GROUP 2 \n"
             << "    ORIG                 " << application.spclHipOrigInd()
             << "    DEST            " << application.spclHipDestInd() << "\n"
             << "    ORIG NATION          " << application.spclHipOrigNationInd()
             << "    DEST NATION     " << application.spclHipDestNationInd() << "\n"
             << "    INTERM               " << application.spclHipFromInterInd()
             << "    INTERM          " << application.spclHipToInterInd() << "\n"
             << "  ADDITIONAL CHECK POINTS - \n"
             << "    FROM GROUP 1              TO GROUP 2 \n"
             << "    ORIG NATION TKT      " << application.spclHipOrigNationTktPt()
             << "    TKT             " << application.spclHipTicketedPt() << "\n"
             << "    ORIG NATION STOPOVER " << application.spclHipOrigNationStopPt()
             << "    STOPOVER        " << application.spclHipStopoverPt() << "\n"
             << "  EXEMPT INTERM TO INTERM: " << application.spclHipExemptInterToInter() << "\n";
    }

    *_diag << "SPECIAL FARE TYPE GROUP - \n";
    *_diag << "  PROCESS NAME: " << application.specialProcessName() << " \n";

    if (_module == CTM)
    {
      *_diag << "NORMAL CTM - \n"
             << "  DETAILED CHECK POINTS - \n"
             << "    FROM GROUP 1              TO GROUP 2 \n"
             << "    ORIG                 " << application.nmlCtmOrigInd() << "    INTERM          "
             << application.nmlCtmToInterInd() << "\n"
             << "                          "
             << "    DEST NATION     " << application.nmlCtmDestNationInd() << "\n"
             << "SPECIAL CTM - \n"
             << "  DETAILED CHECK POINTS - \n"
             << "    FROM GROUP 1              TO GROUP 2 \n"
             << "    ORIG                 " << application.spclCtmOrigInd()
             << "    INTERM          " << application.spclCtmToInterInd() << "\n"
             << "                          "
             << "    DEST NATION     " << application.spclCtmDestNationInd() << "\n";
    }

    if (_module == CPM)
    {
      *_diag << "CPM EXCLUSION - " << application.cpmExcl() << "\n";
    }
  }
}
}
