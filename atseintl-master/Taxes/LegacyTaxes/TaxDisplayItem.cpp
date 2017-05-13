// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxCodeGenText.h"
#include "DBAccess/TaxNation.h"
#include "DBAccess/Nation.h"
#include "DBAccess/DataHandle.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseEnums.h"
#include "Common/TseConsts.h"

using namespace tse;
using namespace std;

void
TaxDisplayItem::buildTaxDisplayItem(TaxTrx& taxTrx,
                                    TaxCodeReg& taxCodeReg,
                                    const TaxNation& taxNation)
{
  _taxCodeReg = &taxCodeReg;
  _taxCode = taxCodeReg.taxCode();
  _roundingUnit = taxNation.roundingUnit();
  _roundingNoDec = taxNation.roundingUnitNodec();
  _roundingRule = taxNation.roundingRule();
  _message = taxNation.message();

  if (!taxCodeReg.taxCodeGenTexts().empty())
  {
    if (!taxCodeReg.taxCodeGenTexts().front()->txtMsgs().empty())
      _taxDescription = taxCodeReg.taxCodeGenTexts().front()->txtMsgs().front();
  }

  const std::vector<Nation*> allNationList =
      taxTrx.dataHandle().getAllNation(taxTrx.getRequest()->ticketingDT());

  std::vector<Nation*>::const_iterator nationsListIter = allNationList.begin();
  std::vector<Nation*>::const_iterator nationsListEnd = allNationList.end();

  if (allNationList.empty())
  {
    return;
  }

  for (; nationsListIter != nationsListEnd; nationsListIter++)
  {
    if (taxNation.nation() != (*nationsListIter)->nation())
      continue;

    _taxNation = (*nationsListIter)->description();
    break;
  }
}
