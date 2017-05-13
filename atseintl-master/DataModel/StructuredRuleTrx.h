//-------------------------------------------------------------------
//
//  Authors:     Kacper Stapor
//
//  Copyright Sabre 2015
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

#include "DataModel/PricingTrx.h"

#include <memory>

namespace tse
{
class Service;

class StructuredRuleTrx : public PricingTrx
{
public:
  StructuredRuleTrx() = default;
  bool process(Service& srv) override;
  void convert(tse::ErrorResponseException& ere, std::string& response) override;
  bool convert(std::string& response) override;
  void setupFootNotePrevalidation() override final;
  void createMultiPassangerFCMapping() { _passangersFCMapping.reset(new MultiPaxFCMapping); };
  MultiPaxFCMapping* getMultiPassengerFCMapping() const override
  {
    return _passangersFCMapping.get();
  }
  bool isMultiPassengerSFRRequestType() const override;
  void setMultiPassengerSFRRequestType() override { _isMultiPassengerSFRRequest = true; }

private:
  std::unique_ptr<MultiPaxFCMapping> _passangersFCMapping;
  bool _isMultiPassengerSFRRequest = false;
};

} // tse

