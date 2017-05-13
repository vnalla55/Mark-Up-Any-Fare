//----------------------------------------------------------------------------
//  File: FDCustomerRetriever.h
//
//  Author: Partha Kumar Chakraborti
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class FareDisplayTrx;

class FDCustomerRetriever
{
public:
  FDCustomerRetriever(FareDisplayTrx& trx);
  virtual ~FDCustomerRetriever();
  virtual bool retrieve() = 0;

  static const Indicator TYPE_NONE;
  static const UserApplCode USER_NONE;
  static const PseudoCityCode PCC_NONE;
  static const TJRGroup TJR_NONE;

protected:
  FareDisplayTrx& _trx;
  UserApplCode _userMulti;
  UserApplCode _userCrs;
  PseudoCityCode _pccAgent;
  PseudoCityCode _pccHome;
  TJRGroup _tjrGroup;

  // scans matching recs and does retriveData on them
  // does only first match or all matches based on allRecs
  bool retrieve(bool allRecs);
  bool retrieveOne() { return retrieve(false); }
  bool retrieveAll() { return retrieve(true); }

  // the default data access uses all keys
  virtual bool retrieveData(const Indicator& userApplType,
                            const UserApplCode& userAppl,
                            const Indicator& pseudoCityType = TYPE_NONE,
                            const PseudoCityCode& pseudoCity = PCC_NONE,
                            const TJRGroup& tjrGroup = TJR_NONE) = 0;

  // stub and/or redefine these in subclasses that do not use all keys
  virtual bool retrieveDefault()
  {
    return retrieveData(TYPE_NONE, USER_NONE);
  };

  virtual bool retrieveData(const Indicator& pccType, const PseudoCityCode& pccCode)
  {
    return retrieveData(TYPE_NONE, USER_NONE, pccType, pccCode);
  };

  virtual bool retrieveData(const TJRGroup& tjrGroup);
};
}
