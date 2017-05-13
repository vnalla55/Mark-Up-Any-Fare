//----------------------------------------------------------------------------
//   File : DisplayTypeValidator.cpp
// Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseConsts.h"
#include "FareDisplay/Validator.h"


#include <map>

namespace tse
{
/**
 *@class DisplayTypeValidator.
 *
 */

class FareDisplayTrx;
class PaxTypeFare;

class DisplayTypeValidator : public Validator
{
public:
  bool validate(const PaxTypeFare& paxType) override;
  bool initialize(const FareDisplayTrx& trx) override;
  void failValidator(bool& invalidDT, bool& invalidFT, bool& invalidPT) override;

private:

  /**
  * Read-only set for looking up supported dspl cat types
  **/
  std::set<Indicator> _displayTypes;
  virtual bool getData(const FareDisplayTrx& trx);

  virtual bool validateDisplayType(const Indicator& displayType) const;
  virtual Validator::ValidatorType name() const override { return Validator::DISPLAY_TYPE_VALIDATOR; }
};

} // namespace tse

