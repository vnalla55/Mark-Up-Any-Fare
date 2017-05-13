#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/ValidatingCxrConst.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/DataHandle.h"
#include "Service/Service.h"

namespace tse
{
class ConfigMan;
class TicketingCxrDisplayTrx;
class TseServer;
class Diag191Collector;

class TicketingCxrDisplay final : public Service
{
  friend class TicketingCxrDisplayTest;

public:
  TicketingCxrDisplay(const std::string& name, TseServer& srv);

  bool initialize(int argc = 0, char* argv[] = nullptr) override;
  virtual bool process(TicketingCxrDisplayTrx& trx) override;

private:
  ConfigMan& _config;
  TicketingCxrDisplay(const TicketingCxrDisplay& rhs);
  TicketingCxrDisplay& operator=(const TicketingCxrDisplay& rhs);

  bool processDisplayInterline(TicketingCxrDisplayTrx& tcsTrx, Diag191Collector* diag191) const;
  bool processDisplayValCxr(TicketingCxrDisplayTrx& tcsTrx, Diag191Collector* diag191) const;
  void processDisplayValCxr(TicketingCxrDisplayTrx& tcdTrx,
      Diag191Collector* diag191,
      const CountrySettlementPlanInfo* csp) const;
  CountrySettlementPlanInfo* getRequestedSettlementPlanInfo(const SettlementPlanType& reqSp,
      const std::vector<CountrySettlementPlanInfo*>& cspList) const;

  bool processDisplayNeutralValCxr( TicketingCxrDisplayTrx& tcdTrx,
                                    const SettlementPlanType& spType,
                                    Diag191Collector* diag191) const;

  bool processDisplayValidatingCxrList( TicketingCxrDisplayTrx& tcdTrx,
                                        const CountrySettlementPlanInfo* csp,
                                        Diag191Collector* diag191) const;

  bool processDisplayGeneralSalesAgent( TicketingCxrDisplayTrx& tcdTrx,
                                        const SettlementPlanType& spType,
                                        Diag191Collector* diag191) const;
};
}
