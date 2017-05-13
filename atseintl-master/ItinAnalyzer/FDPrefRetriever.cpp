//----------------------------------------------------------------------------
//  File: FDPrefRetriever.cpp
//
//  Author: Jeff Hoffman
//  Created:      03/20/2007
//  Description:  This takes cares of getting header message text based on header msg table
//  Copyright Sabre 2007
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#include "ItinAnalyzer/FDPrefRetriever.h"

#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/FareDisplayPrefSeg.h"

namespace tse
{
FDPrefRetriever::FDPrefRetriever(FareDisplayTrx& trx) : FDCustomerRetriever(trx) {}

bool
FDPrefRetriever::retrieve()
{
  _trx.getOptions()->fareDisplayPref() = nullptr;
  retrieveOne();
  return true;
}

bool
FDPrefRetriever::retrieveData(const Indicator& userApplType,
                              const UserApplCode& userAppl,
                              const Indicator& pseudoCityType,
                              const PseudoCityCode& pseudoCity,
                              const TJRGroup& tjrGroup)
{
  const std::vector<FareDisplayPref*>& prefList = _trx.dataHandle().getFareDisplayPref(
      userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup);

  if (!prefList.empty())
  {
    _trx.getOptions()->fareDisplayPref() = prefList.front();
    getPrefSeg(userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup);
    return true;
  }
  return false;
}

void
FDPrefRetriever::getPrefSeg(const Indicator& userApplType,
                            const UserApplCode& userAppl,
                            const Indicator& pseudoCityType,
                            const PseudoCityCode& pseudoCity,
                            const TJRGroup& tjrGroup)
{
  const std::vector<FareDisplayPrefSeg*>& fdPrefSegList = _trx.dataHandle().getFareDisplayPrefSeg(
      userApplType, userAppl, pseudoCityType, pseudoCity, tjrGroup);
  if (fdPrefSegList.size() > 0)
  {
    std::vector<FareDisplayPrefSeg*>::const_iterator i = fdPrefSegList.begin();
    std::vector<FareDisplayPrefSeg*>::const_iterator end = fdPrefSegList.end();

    for (; i != end; ++i)
    {
      _trx.getOptions()->surchargeTypes().push_back((*i)->surchargeType());
    }
    _trx.getOptions()->applySurcharges() = YES;
  }
}
}
