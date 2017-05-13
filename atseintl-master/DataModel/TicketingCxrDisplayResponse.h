#pragma once


#include "Common/TseCodeTypes.h"
#include "Common/ValidatingCxrConst.h"
#include "DataModel/PricingRequest.h"
#include "DBAccess/AirlineCountrySettlementPlanInfo.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/GenSalesAgentInfo.h"
#include "DBAccess/NeutralValidatingAirlineInfo.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace tse
{
class AirlineInterlineAgreementInfo;
class TicketingCxrValidatingCxrDisplay;

typedef std::map<Alpha3Char, std::set<CarrierCode> > InterlineAgreements;
typedef std::map<Alpha3Char, std::set<CarrierCode> >::const_iterator InterlineAgreementsIter;

typedef std::map<CarrierCode, std::set<CarrierCode> > GeneralSalesAgentMap;
typedef std::map<CarrierCode, std::set<CarrierCode> >::const_iterator GeneralSalesAgentMapIter;

typedef std::map<vcx::TicketType, std::set<CarrierCode> > ValidatingCarrierMap;
typedef std::map<vcx::TicketType, std::set<CarrierCode> >::const_iterator ValidatingCarrierMapIter;

typedef std::map<SettlementPlanType, TicketingCxrValidatingCxrDisplay> ValidatingCxrDisplayMap;
typedef std::map<SettlementPlanType, TicketingCxrValidatingCxrDisplay>::const_iterator
ValidatingCxrDisplayMapIter;

// map to contain settlement plan type and settlement plan name
typedef std::map<SettlementPlanType, std::string> SettlementPlanMap;
typedef std::map<SettlementPlanType, std::string>::const_iterator SettlementPlanMapIter;

class TicketingCxrValidatingCxrDisplay
{
public:
  const ValidatingCarrierMap& validatingCarrierMap() const { return _validatingCarrier; }
  ValidatingCarrierMap& validatingCarrierMap() { return _validatingCarrier; }

  const GeneralSalesAgentMap& generalSalesAgentMap() const { return _generalSalesAgent; }
  GeneralSalesAgentMap& generalSalesAgentMap() { return _generalSalesAgent; }

  const std::set<CarrierCode>& neutralValidatingCxr() const { return _neutralValidatingCxr; }
  std::set<CarrierCode>& neutralValidatingCxr() { return _neutralValidatingCxr; }

  void setNeutralValidatingCxr(const std::vector<NeutralValidatingAirlineInfo*>& neutralCxrList);
  void setGeneralSalesAgentMap(const std::vector<GenSalesAgentInfo*>& gsaList);
  void setValidatingCxrMap(const std::vector<AirlineCountrySettlementPlanInfo*>& acspList,
                           const CountrySettlementPlanInfo* csp);

private:
  ValidatingCarrierMap _validatingCarrier;
  GeneralSalesAgentMap _generalSalesAgent;
  std::set<CarrierCode> _neutralValidatingCxr;
};

class TicketingCxrDisplayResponse
{
public:
  const InterlineAgreements& interlineAgreements() const { return _interlineAgmts; }
  InterlineAgreements& interlineAgreements() { return _interlineAgmts; }

  const CrsCode& primeHost() const { return _primeHost; }
  CrsCode& primeHost() { return _primeHost; }

  const NationCode& country() const { return _country; }
  NationCode& country() { return _country; }

  const CarrierCode& validatingCxr() const { return _valCxr; }
  CarrierCode& validatingCxr() { return _valCxr; }

  void setInterlineAgreements(const std::vector<AirlineInterlineAgreementInfo*>& iaList);

  const ValidatingCxrDisplayMap& validatingCxrDisplayMap() const { return _validatingCxrDisplay; }
  ValidatingCxrDisplayMap& validatingCxrDisplayMap() { return _validatingCxrDisplay; }

private:
  InterlineAgreements _interlineAgmts;
  ValidatingCxrDisplayMap _validatingCxrDisplay;
  CrsCode _primeHost;
  NationCode _country;
  CarrierCode _valCxr;
};
} // End tse
