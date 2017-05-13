//----------------------------------------------------------------------------
//  File: InclusionCodeRetriever.cpp
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

#include "ItinAnalyzer/InclusionCodeRetriever.h"

#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"

namespace tse
{
InclusionCodeRetriever::InclusionCodeRetriever(FareDisplayTrx& trx)
  : FDCustomerRetriever(trx), _inclusionCode(trx.getRequest()->inclusionCode()), _inclCdRec(nullptr)
{
}

bool
InclusionCodeRetriever::retrieve()
{
  retrieveOne();
  return true;
}

FareDisplayInclCd*
InclusionCodeRetriever::fetch()
{
  retrieve();
  return _inclCdRec;
}

bool
InclusionCodeRetriever::retrieveData(const Indicator& userApplType,
                                     const UserApplCode& userAppl,
                                     const Indicator& pseudoCityType,
                                     const PseudoCityCode& pseudoCity,
                                     const TJRGroup& tjrGroup)
{
  const std::vector<FareDisplayInclCd*>& inclusionCodes = _trx.dataHandle().getFareDisplayInclCd(
      userApplType, userAppl, pseudoCityType, pseudoCity, _inclusionCode);

  if (inclusionCodes.empty())
    return false;

  _inclCdRec = inclusionCodes.front();

  return (_inclCdRec != nullptr);
}
}
