//////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//
//  File:           Qualifier.h
//
//  Description:    Base class for Fare Qualifier classes.
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

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class FareDisplayTrx;
class PaxTypeFare;

class Qualifier
{
public:
  virtual ~Qualifier() = default;

  virtual const PaxTypeFare::FareDisplayState
  qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const = 0;
  virtual bool setup(FareDisplayTrx& trx) { return true; }

  Qualifier*& successor() { return _successor; }
  const Qualifier* successor() const { return _successor; }

  const PaxTypeFare::FareDisplayState retProc(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
  {
    return _successor ? _successor->qualify(trx, ptFare) : PaxTypeFare::FD_Valid;
  }

protected:
  Qualifier* _successor = nullptr;

  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

