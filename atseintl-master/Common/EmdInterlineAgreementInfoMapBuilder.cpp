/*
 * EmdInterlineAgreementInfoMapBuilder.cpp
 *
 *  Created on: Jun 16, 2015
 *      Author: SG0216859
 */

#include "Common/EmdInterlineAgreementInfoMapBuilder.h"
#include "Common/TseConsts.h"
#include "DBAccess/EmdInterlineAgreementInfo.h"
#include "DBAccess/DataHandle.h"


namespace tse
{

bool
EmdInterlineAgreementInfoMapBuilder::populateRecords(const NationCode& nation, const CrsCode& gds, const CarrierCode& carrier,
                                                     DataHandle& dataHandle, EmdInterlineAgreementInfoMap& emdInfoMap)
{
  EmdInterlineAgreementInfoVector eiaList;
  const EmdInterlineAgreementInfoVector& allCountriesEiaList =
    dataHandle.getEmdInterlineAgreements(NATION_ALL, gds, carrier);

  if(!allCountriesEiaList.empty())
    eiaList.insert(eiaList.end(), allCountriesEiaList.begin(), allCountriesEiaList.end());

  const EmdInterlineAgreementInfoVector& specificCountryEiaList =
    dataHandle.getEmdInterlineAgreements(nation, gds, carrier);

  if(!specificCountryEiaList.empty())
    eiaList.insert(eiaList.end(), specificCountryEiaList.begin(), specificCountryEiaList.end());

  emdInfoMap.insert(EmdInterlineAgreementInfoMap::value_type(carrier, eiaList));

  if(eiaList.empty())
    return false;

  return true;
}


} /* namespace tse */
