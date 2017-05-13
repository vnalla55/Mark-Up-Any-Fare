//-------------------------------------------------------------------
//
//  File:        TicketingCxrService.h
//  Created:     November 2013
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

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/ValidatingCxrConst.h"
#include "Service/Service.h"

namespace tse
{
class ConfigMan;
class TicketingCxrTrx;
class Diag191Collector;
class CountrySettlementPlanInfo;
class TseServer;

class TicketingCxrService final : public Service
{
  friend class TicketingCxrServiceTest;

public:
  TicketingCxrService(const std::string& name, TseServer& srv);

  TicketingCxrService(const TicketingCxrService&) = delete;
  TicketingCxrService& operator=(const TicketingCxrService&) = delete;

  bool initialize(int argc = 0, char* argv[] = nullptr) override;
  virtual bool process(TicketingCxrTrx& trx) override;

private:
  ConfigMan& _config;
  bool processCxrSettlementPlan(TicketingCxrTrx& tcsTrx, Diag191Collector* diag191) const;
  bool processPlausibilityCheck(TicketingCxrTrx& tcsTrx, Diag191Collector* diag191) const;

  CountrySettlementPlanInfo* checkCountrySettlementPlan(TicketingCxrTrx& tcsTrx,
                                                        Diag191Collector* diag191,
                                                        SettlementPlanType& sp) const;

  vcx::ValidationStatus checkForValCxr(TicketingCxrTrx& tcsTrx,
                      Diag191Collector* diag,
                      const vcx::Pos& pos,
                      const CarrierCode& cxr) const;

  vcx::ValidationStatus checkForValCxrAmongGSA(TicketingCxrTrx& tcsTrx,
                              Diag191Collector* diag,
                              const vcx::Pos& pos) const;

  bool isCxrInItin(const CarrierCode& cxr,
                      const std::vector<vcx::ParticipatingCxr>& cxrs) const;

  vcx::ValidationStatus determineValidationStatus(const TicketingCxrTrx& tcsTrx,
    const vcx::ValidationStatus& vstatus,
    const std::vector<vcx::ValidationStatus>& statusList) const;
};
}
