//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "DataModel/FlexFares/GroupsDataAnalyzer.h"


namespace tse
{

namespace flexFares
{

void
GroupsDataAnalyzer::analyzeGroupAndPutResultsTo(const GroupId index,
                                                const GroupAttrs& groupAttrs,
                                                TotalAttrs& storage)
{
  TariffType tariffType = TariffType::Mixed;

  for (const std::string& corpId : groupAttrs.corpIds)
  {
    storage.addCorpId(corpId, index);
  }
  for (const std::string& accCode : groupAttrs.accCodes)
  {
    storage.addAccCode(accCode, index);
  }
  if (groupAttrs.corpIds.empty() && groupAttrs.accCodes.empty())
    storage.setMatchEmptyAccCode(true);

  if (groupAttrs.arePublicFaresRequired())
  {
    storage.addGroup<PUBLIC_FARES>(index);
    if ((storage.getTariffType() == TariffType::Unknown) ||
        (storage.getTariffType() == TariffType::Published))
      tariffType = TariffType::Published;
  }

  if (groupAttrs.arePrivateFaresRequired())
  {
    storage.addGroup<PRIVATE_FARES>(index);
    if ((storage.getTariffType() == TariffType::Unknown) ||
        (storage.getTariffType() == TariffType::Private))
      tariffType = TariffType::Private;
  }

  storage.setTariffType(tariffType);

  if (groupAttrs.isNoAdvancePurchaseRequired())
    storage.addGroup<NO_ADVANCE_PURCHASE>(index);

  if (groupAttrs.isNoPenaltiesRequired())
    storage.addGroup<NO_PENALTIES>(index);

  if (groupAttrs.isNoMinMaxStayRequired())
    storage.addGroup<NO_MIN_MAX_STAY>(index);
}

} // flexFares

} // tse
