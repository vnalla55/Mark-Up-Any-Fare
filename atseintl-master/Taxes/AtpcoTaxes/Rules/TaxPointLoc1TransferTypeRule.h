// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include "Rules/BusinessRule.h"

namespace tax
{
class Request;
class TaxPointLoc1TransferTypeApplicator;

class TaxPointLoc1TransferTypeRule : public BusinessRule
{
public:
  typedef TaxPointLoc1TransferTypeApplicator ApplicatorType;
  static const type::EquipmentCode ChangeOfGauge;

  TaxPointLoc1TransferTypeRule(const type::TransferTypeTag transferTypeTag);

  virtual ~TaxPointLoc1TransferTypeRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(const type::Index& itinIndex,
                                  const Request& request,
                                  Services& /*services*/,
                                  RawPayments& /*itinPayments*/) const;
  virtual type::TransferTypeTag getTransferTypeTag() const;

private:
  type::TransferTypeTag _transferTypeTag;
};
}

