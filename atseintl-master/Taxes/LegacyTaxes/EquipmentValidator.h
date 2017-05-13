// ---------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------

#ifndef EQUIPMENT_VALIDATOR_H
#define EQUIPMENT_VALIDATOR_H

#include "Common/TseCodeTypes.h"

namespace tse

{
class PricingTrx;
class TaxResponse;
class TaxCodeReg;
class TravelSeg;

class EquipmentValidator
{

public:
  EquipmentValidator();
  virtual ~EquipmentValidator();

  bool validateEquipment(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegIndex);

  bool validateEquipment(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         TravelSeg* travelSeg);

private:
  static constexpr char TAX_EXCLUDE = 'Y';
  static constexpr char TAX_NOT_EXCLUDE = 'N';

  EquipmentValidator(const EquipmentValidator& validation);
  EquipmentValidator& operator=(const EquipmentValidator& validation);
};
}

#endif // EQUIPMENT_VALIDATOR_H
