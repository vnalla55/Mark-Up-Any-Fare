//----------------------------------------------------------------------------
//
//  Copyright Sabre 2003
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "FareDisplay/Validator.h"


namespace tse
{
class FareDisplayInclCd;
class FareDisplayTrx;

class ValidatorBuilder
{
public:
  ValidatorBuilder(FareDisplayTrx& trx, std::vector<Validator*>& validators,
                   std::map<uint8_t, std::vector<Validator*> >& inclValidators)
    : _trx(trx), _validators(validators), _inclusionCodeValidators(inclValidators)
  {
  }
  void build();
  virtual ~ValidatorBuilder();

private:
  FareDisplayTrx& _trx;
  std::vector<Validator*>& _validators;
  std::map<uint8_t, std::vector<Validator*> >& _inclusionCodeValidators;

  Validator* getPaxTypeValidator();
  Validator* getFareTypeValidator();
  Validator* getDisplayTypeValidator();
  Validator* getWebICValidator();
  Validator* getRuleTariffValidator();

  void add(Validator*& validator);
  bool isSpecifiedInclusionCode() const;
  void getAll(const FareDisplayInclCd& inclusionCode);
  void getAllSpecified();
  void getAll();
  std::map<uint16_t, Validator::ValidatorType> _requestedValidators;
  bool buildValidators();
  static constexpr Indicator BLANK_IND = ' ';
};
}

