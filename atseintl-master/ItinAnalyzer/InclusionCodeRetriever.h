//----------------------------------------------------------------------------
//  File: InclusionCodeRetriever.h
//
//  Author: Jeff Hoffman
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

#include "Common/TseStringTypes.h"
#include "Common/FDCustomerRetriever.h"

namespace tse
{
class FareDisplayInclCd;

class InclusionCodeRetriever : public FDCustomerRetriever
{
  friend class InclCdRetrieverTest;

public:
  InclusionCodeRetriever(FareDisplayTrx& trx);
  virtual ~InclusionCodeRetriever() {}

  // does everything with all needed records
  virtual bool retrieve() override;
  FareDisplayInclCd* fetch();

private:
  InclusionCode& _inclusionCode;
  FareDisplayInclCd* _inclCdRec;

  // does everything for one record
  virtual bool retrieveData(const Indicator& userApplType,
                            const UserApplCode& userAppl,
                            const Indicator& pCCType,
                            const PseudoCityCode& pCC,
                            const TJRGroup& tjrGroup) override;

  virtual bool retrieveData(const TJRGroup& tjrGroup) override { return false; };

}; // End of Class InclusionCodeRetriever

} // end of namespace

