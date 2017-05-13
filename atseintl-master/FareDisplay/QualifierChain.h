//////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//
//  File:           QualifierChain.h
//
//  Description:    Base class for Fare QualifierChain classes.
//
//  Updates:
//
//  Copyright Sabre 2006
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

#pragma once

#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"

namespace tse
{

class QualifierChain
{
public:
  virtual ~QualifierChain() = default;

  virtual bool buildChain(FareDisplayTrx& trx, bool useAll) = 0;
  virtual bool demolishChain() = 0;
  virtual int size() const = 0;
  virtual const PaxTypeFare::FareDisplayState
  qualifyPaxTypeFare(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const = 0;

};
} // namespace tse

