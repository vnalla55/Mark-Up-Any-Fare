#include "DataModel/TicketingCxrDisplayResponse.h"

#include "Common/ValidatingCxrConst.h"
#include "Common/ValidatingCxrUtil.h"
#include "DBAccess/AirlineInterlineAgreementInfo.h"

namespace tse
{

void
TicketingCxrDisplayResponse::setInterlineAgreements(
    const std::vector<AirlineInterlineAgreementInfo*>& iaList)
{
  for (AirlineInterlineAgreementInfo* info : iaList)
  {
    _interlineAgmts[info->getAgreementTypeCode()].insert(info->getParticipatingCarrier());
  }
}

void
TicketingCxrValidatingCxrDisplay::setNeutralValidatingCxr(
    const std::vector<NeutralValidatingAirlineInfo*>& nvaList)
{
  for (NeutralValidatingAirlineInfo* nva : nvaList)
  {
    _neutralValidatingCxr.insert(nva->getAirline());
  }
}

void
TicketingCxrValidatingCxrDisplay::setGeneralSalesAgentMap(
    const std::vector<GenSalesAgentInfo*>& gsaList)
{
  for (GenSalesAgentInfo* gsa : gsaList)
  {
    _generalSalesAgent[gsa->getNonParticipatingCxr()].insert(gsa->getCxrCode());
  }
}

void
TicketingCxrValidatingCxrDisplay::setValidatingCxrMap(
    const std::vector<AirlineCountrySettlementPlanInfo*>& acspList,
    const CountrySettlementPlanInfo* csp)
{
  for (AirlineCountrySettlementPlanInfo* acsp : acspList)
  {
    const vcx::TicketType ticketType = ValidatingCxrUtil::getTicketingMethod(*csp, acsp);
    _validatingCarrier[ticketType].insert(acsp->getAirline());
  }
}
} // tse namespace
