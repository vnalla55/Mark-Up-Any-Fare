//-------------------------------------------------------------------
//
//  File:        PaxTypeCodeValidator.h
//  Created:     June 22, 2009
//
//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc. All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/TseCodeTypes.h"

namespace tse
{

class DiagCollector;
class PaxType;

class PaxTypeCodeValidator
{
public:
  PaxTypeCodeValidator(DiagCollector* dc, const std::string& label) : _dc(dc), _label(label) {}

  bool validate(uint32_t itemNoR3,
                const PaxType& paxType,
                const PaxTypeCode& psgType);

protected:
  DiagCollector* _dc;
  const std::string& _label;

private:
  PaxTypeCodeValidator(const PaxTypeCodeValidator&);
  PaxTypeCodeValidator& operator=(const PaxTypeCodeValidator&);
  friend class PaxTypeCodeValidatorTest;
};
}
