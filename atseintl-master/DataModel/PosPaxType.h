//-------------------------------------------------------------------
//
//  File:        PosPaxType.h
//  Created:     October 4, 2004
//  Authors:     Kitima Chunpleng
//
//  Description:
//
//  Updates:
//
//  Copyright Sabre 2004
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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxType.h"

namespace tse
{

class PosPaxType : public PaxType
{
private:
  uint16_t _fgNumber = 1; // fare group number from intellisell
  uint16_t _priority = 0; // fare priority
  bool _positive = true; // true = Positive, false = negative
  PseudoCityCode _pcc; // pcc location
  std::string _corpID; // corporate ID

public:
  // Access
  uint16_t& fgNumber() { return _fgNumber; }
  const uint16_t& fgNumber() const { return _fgNumber; }

  uint16_t& priority() { return _priority; }
  const uint16_t& priority() const { return _priority; }

  bool& positive() { return _positive; }
  const bool& positive() const { return _positive; }

  PseudoCityCode& pcc() { return _pcc; }
  const PseudoCityCode& pcc() const { return _pcc; }

  std::string& corpID() { return _corpID; }
  const std::string& corpID() const { return _corpID; }
};

typedef std::vector<PosPaxType*> PosPaxTypePerPax;

} // tse namespace
