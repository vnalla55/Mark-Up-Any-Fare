//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#pragma once

#include "Rules/RuleValidationMonitor.h"
#include "Rules/RuleValidationObserver.h"

namespace tse
{

class CategoryValidationObserver : public RuleValidationObserver
{
public:
  CategoryValidationObserver(Monitor& monitor);

  void update(const RuleValidationContext& rvc,
              flexFares::ValidationStatusPtr validationStatus,
              const uint16_t& catNumber,
              const Record3ReturnTypes& ret) override;

  void update(const RuleValidationContext& rvc,
              const std::string& acctCodeOrCorpIdString,
              flexFares::ValidationStatusPtr validationStatus,
              const bool isAccountCode) override;

  virtual ~CategoryValidationObserver();
}; // CategoryValidationObserver

} // tse

