/*
 * EmdInterlineAgreementInfoMapBuilder.h
 *
 *  Created on: Jun 16, 2015
 *      Author: SG0216859
 */

#pragma once

#include "Common/TseCodeTypes.h"

#include <map>
#include <vector>

namespace tse
{
class EmdInterlineAgreementInfo;
class DataHandle;

using EmdInterlineAgreementInfoVector = std::vector<EmdInterlineAgreementInfo*>;
using EmdInterlineAgreementInfoMap = std::map<CarrierCode, EmdInterlineAgreementInfoVector>;

struct EmdInterlineAgreementInfoMapBuilder
{
  //this method gets data from the database using dataHandle and puts it into EmdInterlineAgreementInfoMap
  static bool populateRecords(const NationCode& nation, const CrsCode& gds, const CarrierCode& carrier,     //filters
                              DataHandle& dataHandle,                                                       //input
                              EmdInterlineAgreementInfoMap& emdInfoMap);                                    //output
};


} /* namespace tse */
