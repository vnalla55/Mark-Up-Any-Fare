//----------------------------------------------------------------------------
//  File: FDPrefRetriever.h
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

#include "Common/FDCustomerRetriever.h"

namespace tse
{

class FDPrefRetriever : public FDCustomerRetriever
{
  friend class FDPrefRetrieverTest;

public:
  FDPrefRetriever(FareDisplayTrx& trx);
  virtual ~FDPrefRetriever() {}

  // does everything with all needed records
  virtual bool retrieve() override;

private:
  // does everything for one record
  virtual bool retrieveData(const Indicator& userApplType,
                            const UserApplCode& userAppl,
                            const Indicator& pseudoCityType,
                            const PseudoCityCode& pseudoCity,
                            const TJRGroup& tjrGroup) override;

  void getPrefSeg(const Indicator& userApplType,
                  const UserApplCode& userAppl,
                  const Indicator& pseudoCityType,
                  const PseudoCityCode& pseudoCity,
                  const TJRGroup& tjrGroup);

}; // End of Class FDPrefRetriever

} // end of namespace

