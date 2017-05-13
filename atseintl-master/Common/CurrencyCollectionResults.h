//----------------------------------------------------------------------------
//
//  File: CurrencyCollectionResults.h
//
//  Created: June 2004
//
//  Copyright Sabre 2004
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

namespace tse
{
/**
*   @class CurrencyCollectionResults
*
*   Description:
*   Base class for Currency Collection results
*/
class CurrencyCollectionResults
{
public:
  virtual ~CurrencyCollectionResults() = default;
  /**
  *   @method collect
  *
  *   Description: Get/Set methods for determining whether or not to collect
  *                conversion information.
  *
  *   @return bool - reference to collect attribute.
  *
  */
  virtual bool& collect() { return _collect; }
  virtual const bool& collect() const { return _collect; }

private:
  bool _collect = false;
};
}

