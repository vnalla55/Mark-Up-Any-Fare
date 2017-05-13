//-------------------------------------------------------------------
//
//  File:        AccTvlDetailIn.cpp
//  Created:     Feb 27, 2006
//  Authors:     Simon Li
//
//  Updates:
//
//  Description:
//
//  Copyright Sabre 2006
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

#include "Common/AccTvlDetailIn.h"

#include "Common/PaxTypeUtil.h"
#include "DBAccess/DataHandle.h"

namespace tse
{
bool
AccTvlDetailIn::restoreAccTvlDetail(DataHandle& dataHandle,
                                    const std::vector<const std::string*>& accTvlData,
                                    std::vector<const AccTvlFarePath*>& farePaths)
{
  std::vector<const std::string*>::const_iterator accTvlDataI = accTvlData.begin();
  const std::vector<const std::string*>::const_iterator accTvlDataIEnd = accTvlData.end();

  for (; accTvlDataI != accTvlDataIEnd; accTvlDataI++)
  {
    AccTvlFarePath* farePath = nullptr;
    dataHandle.get(farePath);

    if (!farePath)
      return false;

    if (restoreAccTvlDetail(dataHandle, **accTvlDataI, *farePath))
    {
      farePaths.push_back(farePath);
    }
    else
    {
      return false;
    }
  }
  return true;
}

bool
AccTvlDetailIn::restoreAccTvlDetail(DataHandle& dataHandle,
                                    const std::string& accTvlDataStr,
                                    AccTvlFarePath& farePath)
{
  if (accTvlDataStr.empty())
    return false;

  AccTvlDetailReader detailReader(accTvlDataStr);

  return restoreAccTvlDetail(detailReader, dataHandle, farePath);
}

bool
AccTvlDetailIn::restoreAccTvlDetail(AccTvlDetailReader& inputObj,
                                    DataHandle& dataHandle,
                                    AccTvlFarePath& farePath)
{
  try
  {
    inputObj >> farePath.truePaxType();

    if (!restoreAccTvlDetail(inputObj, farePath.paxType()))
      return false;

    PaxTypeUtil::getVendorCode(dataHandle, farePath.paxType());

    // restore detail required for accompanied travel restriction
    // for all fare usages
    uint16_t ptfOrder = 0;
    int numOfPaxTypeFares = 0;

    inputObj >> numOfPaxTypeFares;

    for (; ptfOrder < numOfPaxTypeFares; ptfOrder++)
    {
      SimplePaxTypeFare* paxTypeFare = nullptr;
      dataHandle.get(paxTypeFare);

      if (!paxTypeFare)
      {
        return false;
      }

      paxTypeFare->setPaxType(farePath.truePaxType());
      paxTypeFare->paxNumber() = farePath.paxType().number();

      if (!restoreAccTvlDetail(inputObj, dataHandle, *paxTypeFare))
      {
        return false;
      }
      farePath.paxTypeFares().push_back(paxTypeFare);
    }
  }
  catch (...) { return false; }

  return true;
}

bool
AccTvlDetailIn::restoreAccTvlDetail(AccTvlDetailReader& inputObj, PaxType& paxType)
{
  inputObj >> paxType.number() >> paxType.paxType() >> paxType.age();

  return true;
}

bool
AccTvlDetailIn::restoreAccTvlDetail(AccTvlDetailReader& inputObj,
                                    DataHandle& dataHandle,
                                    SimplePaxTypeFare& paxTypeFare)
{
  inputObj >> paxTypeFare.origSegNo() >> paxTypeFare.destSegNo() >> paxTypeFare.cabin() >>
      paxTypeFare.fareClass();

  inputObj >> paxTypeFare.ruleNumber() >> paxTypeFare.tcrRuleTariff() >> paxTypeFare.bookingCode();

  char ruleHead = AccTvlDetailIn::RuleHead;
  while (inputObj.getting(ruleHead))
  {
    // there is more rule on this PaxTypeFare
    SimpleAccTvlRule* simpleAccTvlRule = nullptr;
    dataHandle.get(simpleAccTvlRule);
    if (!simpleAccTvlRule)
      return false;

    if (!restoreAccTvlDetail(inputObj, dataHandle, *simpleAccTvlRule))
      return false;

    paxTypeFare.simpleAccTvlRule().push_back(simpleAccTvlRule);
  }

  return true;
}

bool
AccTvlDetailIn::restoreAccTvlDetail(AccTvlDetailReader& inputObj,
                                    DataHandle& dataHandle,
                                    SimpleAccTvlRule& simpleAccTvlRule)
{
  uint16_t numOfOpt = 0;

  std::string flagStr;
  inputObj >> flagStr;
  uint8_t flag = uint8_t(atoi(flagStr.c_str()));

  simpleAccTvlRule._chkList.set(flag);

  if (simpleAccTvlRule.reqChkNumPsg())
  {
    inputObj >> simpleAccTvlRule.numOfAccPsg;
  }
  else
  {
    simpleAccTvlRule.numOfAccPsg = 0; // no number check
  }

  inputObj >> numOfOpt;

  if (numOfOpt == 0)
  {
    return false;
  }

  while (numOfOpt)
  {
    SimpleAccTvlRule::AccTvlOpt* accTvlOption = nullptr;
    dataHandle.get(accTvlOption);
    if (!accTvlOption)
      return false;

    if (!restoreAccTvlDetail(inputObj, dataHandle, *accTvlOption))
    {
      return false;
    }

    simpleAccTvlRule._accTvlOptions.push_back(accTvlOption);
    numOfOpt--;
  }
  return true;
}

bool
AccTvlDetailIn::restoreAccTvlDetail(AccTvlDetailReader& inputObj,
                                    DataHandle& dataHandle,
                                    SimpleAccTvlRule::AccTvlOpt& accTvlOpt)
{
  uint16_t numOfFareClassBkgCds = 0;
  inputObj >> numOfFareClassBkgCds;

  for (; numOfFareClassBkgCds > 0; numOfFareClassBkgCds--)
  {
    std::string* fareClassBkgCd = nullptr;
    dataHandle.get(fareClassBkgCd);

    if (!fareClassBkgCd)
      return false;

    inputObj >> (*fareClassBkgCd);

    accTvlOpt._fareClassBkgCds.push_back(fareClassBkgCd);
  }

  uint16_t numOfAccPaxType = 0;
  inputObj >> numOfAccPaxType;

  for (; numOfAccPaxType > 0; numOfAccPaxType--)
  {
    PaxTypeCode* paxType = nullptr;
    dataHandle.get(paxType);

    if (!paxType)
      return false;

    inputObj >> (*paxType);

    accTvlOpt._paxTypes.push_back(paxType);
  }

  return true;
}
}
