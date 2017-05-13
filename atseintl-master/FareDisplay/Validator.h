#pragma once

#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"

namespace tse
{

/* Classes that provides the interface to validate a fare based on a criterial.*/
class FareDisplayInclCd;

class Validator
{
public:
  virtual ~Validator() = default;

  virtual bool validate(const PaxTypeFare& fare) = 0;
  virtual bool initialize(const FareDisplayTrx& trx) = 0;

  const static Indicator INCL_AND;
  const static Indicator INCL_OR;
  const static Indicator INCL_YES;
  const static Indicator INCL_NO;
  const static Indicator FARE_NOT_TO_BE_DISPLAYED;
  const static Indicator CHILD_IND;
  const static Indicator INFANT_IND;
  const static Indicator TRUE_IND;

  enum Restriction
  {
    AND = 0,
    OR,
    NONE,
    ALL
  };

  enum ValidatorType
  {
    PAX_TYPE_VALIDATOR = 1,
    RQST_PAX_TYPE_VALIDATOR,
    QUALIFIER_PAX_TYPE_VALIDATOR,
    FARE_TYPE_VALIDATOR,
    RULE_TARIFF_VALIDATOR,
    DISPLAY_TYPE_VALIDATOR,
    WEB_VALIDATOR,
    ALL_PAX_TYPE_VALIDATOR
  };

  virtual Restriction restriction(const PaxTypeFare& fare) const;

  virtual void failValidator(bool& invalidDT, bool& invalidFT, bool& invalidPT) {};

  bool validateMixedRestriction(bool& invalidDT, bool& invalidFT, bool& invalidPT);

protected:
  const FareDisplayInclCd* _fareDisplayInclCd = nullptr;
  const FareDisplayTrx* _trx = nullptr;

  bool hasInclusionCode(const FareDisplayTrx& trx)
  {
    if (trx.fdResponse()->fareDisplayInclCd() == nullptr)
    {
      return false;
    }
    return true;
  }

  void setInclusionCode(const FareDisplayInclCd* _inclCd) { _fareDisplayInclCd = _inclCd; }

  virtual bool isDiscounted(const PaxTypeFare& fare) const { return fare.isDiscounted(); }
  virtual ValidatorType name() const = 0;
};
} // namespace tse
