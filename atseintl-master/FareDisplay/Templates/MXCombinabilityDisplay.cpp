//-------------------------------------------------------------------
//
//  File:        MXCombinabilityDisplay.h
//  Author:      Doug Batchelor
//  Created:     Jan 10, 2006
//  Description: A class to provide the processing
//               for displaying combinability MX info.
//
//  Updates:
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//
//////////////////////////////////////////////////////////////////////

#include "FareDisplay/Templates/MXCombinabilityDisplay.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Rules/RuleUtil.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace tse
{
static Logger
logger("atseintl.FareDisplay.MXCombinabilityDisplay");

static const char NOT_APPLICABLE = 'X';

MXCombinabilityDisplay::MXCombinabilityDisplay() {}

MXCombinabilityDisplay::~MXCombinabilityDisplay() {}

bool
MXCombinabilityDisplay::getMXInfo(FareDisplayTrx& trx,
                                  PaxTypeFare& paxTypeFare,
                                  bool isLocationSwapped,
                                  std::string& rep)
{

  // isLocationSwapped = false;
  rep = "*********** COMBINABILITY CATEGORY CONTROL RECORD *************\n";

  CombinabilityRuleInfo* pCat10 = nullptr;

  pCat10 = RuleUtil::getCombinabilityRuleInfo(trx, paxTypeFare, isLocationSwapped);

  // Fill the output string with combinability info.
  displayMXInfo(paxTypeFare, pCat10, rep);

  return true;
}

void
MXCombinabilityDisplay::displayMXInfo(const PaxTypeFare& ptf,
                                      const CombinabilityRuleInfo* cri,
                                      std::string& rep)
{
  std::ostringstream os;
  bool contin = true;

  os.setf(std::ios::left, std::ios::adjustfield);
  os << "FROM-" << std::setw(4) << ptf.origin() << "TO-" << std::setw(4) << ptf.destination()
     << "CARRIER-" << std::setw(4) << ptf.carrier() << "RULE " << std::setw(4) << ptf.ruleNumber()
     << "-" << std::setw(7) << ptf.tcrRuleTariffCode() << "\n"
     << "FARE BASIS-" << std::setw(13) << ptf.createFareBasis(nullptr) << std::setw(2)
     << (ptf.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED ? "RT" : "OW") << "-" << std::setw(5)
     << ptf.fcaFareType() << std::setw(5) << ((ptf.isNormal()) ? "NORMAL FARE" : "SPECIAL FARE")
     << "\n\n";

  if (cri == nullptr)
  {
    os << "            ******** NO RECORD 2 CATEGORY 10 ********\n";
    contin = false;
  }
  else
  {
    if (cri->applInd() == NOT_APPLICABLE)
    {
      os << "            ****** REC 2 CAT 10 NOT APPLICABLE ******\n ";
      contin = false;
    }
  }
  if (!contin)
  {
    rep += os.str();
    return;
  }

  os.setf(std::ios::left, std::ios::adjustfield);
  os << "CXR-" << std::setw(4) << cri->carrierCode() << "VND-" << std::setw(5) << cri->vendorCode()
     << "RULE-" << std::setw(6) << cri->ruleNumber() << "TARIFF-" << std::setw(5)
     << cri->tariffNumber() << "R2 FARE CLASS-" << std::setw(8) << cri->fareClass() << "\n\n"
     << "SEQ      S  N  **************** ********O  R  FT  JNT     C GL\n"
     << "NBR      T  A  T LOC1   T LOC2  FRE S D R  I  NT  CXR     S DI\n" << std::setw(9)
     << cri->sequenceNumber() << "      ";

  if (cri->loc1().locType() != 0)
  {
    os.setf(std::ios::left, std::ios::adjustfield);
    os << std::setw(2) << cri->loc1().locType() << std::setw(7) << cri->loc1().loc();
  }
  else
    os << "         ";

  if (cri->loc2().locType() != 0)
  {
    os.setf(std::ios::left, std::ios::adjustfield);
    os << std::setw(2) << cri->loc2().locType() << std::setw(6) << cri->loc2().loc();
  }
  else
    os << "         ";

  os.setf(std::ios::left, std::ios::adjustfield);
  os << std::setw(4) << cri->fareType() << std::setw(2) << cri->seasonType() << std::setw(2)
     << cri->dowType() << std::setw(3) << cri->owrt() << std::setw(3) << cri->routingAppl()
     << std::setw(4) << cri->footNote1() << std::setw(7) << cri->jointCarrierTblItemNo() << "\n"
     << "SAME PTS - " << std::setw(2) << cri->samepointstblItemNo() << "\n\n"
     << "            P   S   O   D   C    T    F                  A\n"
     << "            E   O   /   O   X    /    /    SRC           P\n"
     << "            R   J   D   J   R    R    T    TRF    RULE   L\n"
     << "  101-OJ        " << std::setw(4) << cri->sojInd() << std::setw(4)
     << cri->sojorigIndestInd() << std::setw(4) << cri->dojInd() << std::setw(2)
     << cri->dojCarrierRestInd() << std::setw(3) << cri->dojSameCarrierInd() << std::setw(2)
     << cri->dojTariffRuleRestInd() << std::setw(3) << cri->dojSameRuleTariffInd() << std::setw(2)
     << cri->dojFareClassTypeRestInd() << std::setw(3) << cri->dojSameFareInd() << "\n"
     << "  102-CT2   " << std::setw(16) << cri->ct2Ind() << std::setw(2) << cri->ct2CarrierRestInd()
     << std::setw(3) << cri->ct2SameCarrierInd() << std::setw(2) << cri->ct2TariffRuleRestInd()
     << std::setw(3) << cri->ct2SameRuleTariffInd() << std::setw(2)
     << cri->ct2FareClassTypeRestInd() << std::setw(3) << cri->ct2SameFareInd() << "\n"
     << "  103-CT2P  " << std::setw(16) << cri->ct2plusInd() << std::setw(2)
     << cri->ct2plusCarrierRestInd() << std::setw(3) << cri->ct2plusSameCarrierInd() << std::setw(2)
     << cri->ct2plusTariffRuleRestInd() << std::setw(3) << cri->ct2plusSameRuleTariffInd()
     << std::setw(2) << cri->ct2plusFareClassTypeRestInd() << std::setw(3)
     << cri->ct2plusSameFareInd() << "\n"
     << "  104-END   " << std::setw(16) << cri->eoeInd() << std::setw(2) << cri->eoeCarrierRestInd()
     << std::setw(3) << cri->eoeSameCarrierInd() << std::setw(2) << cri->eoeTariffRuleRestInd()
     << std::setw(3) << cri->eoeSameRuleTariffInd() << std::setw(2)
     << cri->eoeFareClassTypeRestInd() << std::setw(3) << cri->eoeSameFareInd() << "\n\n"
     << "          V         S S S  O T  N SD 3  E E  P  TO  I O  D \n"
     << "      CAT N ITEM    C R F  D X  E OO F  O E  R  RT  N T  I \n"
     << "TABLE LST D NBR     R T T  T T  G JJ B  E I  D  PH  B B  R \n";

  rep += os.str();
}
}
