//-------------------------------------------------------------------
//
//  File:        S8BrandedFaresSelector.h
//  Created:     2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
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

#include "Common/TseStringTypes.h"

namespace tse
{

class PaxTypeFare;
class BrandProgram;
class BrandInfo;
class BrandedFareDiagnostics;

class BrandedFaresValidator
{
public:
  virtual PaxTypeFare::BrandStatus validateFare(const PaxTypeFare* paxTypeFare, const BrandProgram* brandPr,
                                                const BrandInfo* brand, bool& needBrandSeparator,
                                                BrandedFareDiagnostics& diagnostics,
                                                bool skipHardPassValidation = false) const = 0;
  virtual ~BrandedFaresValidator() = default;
};
} // tse
