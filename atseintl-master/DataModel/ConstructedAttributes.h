//-------------------------------------------------------------------
//
//  File:        ConstructedAttributes.h
//  Created:     October 6, 2005
//  Authors:     Doug Batchelor
//
//  Description: Addon items for constructed fares
//
//  Updates:
//          02/15/05 - Doug Batchelor - file created.
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

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/ConstructedFareInfo.h"

namespace tse
{

class ConstructedAttributes : public PricingOptions
{
  ConstructedAttributes(const ConstructedAttributes&) = delete;
  ConstructedAttributes& operator=(const ConstructedAttributes&) = delete;

private:
  // Constructed fare items
  bool _isConstructedFare = false;
  LocCode _gateway1; // AM0 CString5
  LocCode _gateway2; // AN0 CString5
  ConstructedFareInfo::ConstructionType _constructionType =
      ConstructedFareInfo::SINGLE_ORIGIN; // N1J (enum)
  MoneyAmount _specifiedFareAmount = 0; // C66 double
  MoneyAmount _constructedNucAmount = 0; // C6K double

public:
  ConstructedAttributes() = default;

  bool& isConstructedFare() { return _isConstructedFare; }
  const bool& isConstructedFare() const { return _isConstructedFare; }

  LocCode& gateway1() { return _gateway1; }
  const LocCode& gateway1() const { return _gateway1; }

  LocCode& gateway2() { return _gateway2; }
  const LocCode& gateway2() const { return _gateway2; }

  ConstructedFareInfo::ConstructionType& constructionType() { return _constructionType; }
  const ConstructedFareInfo::ConstructionType constructionType() const { return _constructionType; }

  MoneyAmount& specifiedFareAmount() { return _specifiedFareAmount; }
  const MoneyAmount specifiedFareAmount() const { return _specifiedFareAmount; }

  MoneyAmount& constructedNucAmount() { return _constructedNucAmount; }
  const MoneyAmount constructedNucAmount() const { return _constructedNucAmount; }
};

} // tse namespace

