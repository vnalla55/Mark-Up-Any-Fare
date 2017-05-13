#include "Xform/STLTicketingCxrSchemaNames.h"

namespace tse
{
namespace stlticketingcxr
{
const char* _STLTicketingCxrElementNames[] = {
  "ValidatingCxrCheck",
  "SettlementPlanCheck", "DisplayValidatingCxr",
  "DisplayInterlineAgreement", "POS",
  "Actual",             "Home",
  "Pcc",                "SettlementPlan",
  "ValidatingCxr",      "ParticipatingCxr",
  "TicketType",         "TicketDate",
  "RequestedDiagnostic", "BillingInformation" };

const char* _STLTicketingCxrAttributeNames[] = {
  "company", "duty",
  "lniata", "multiHost", "primeHost",
  "sine", "city",
  "country", "countryCode",
  "settlementPlan", "carrier",
  "number", "province",
  "requestDate", "requestTimeOfDay",
  "actionCode", "businessFunction",
  "parentServiceName", "parentTransactionID",
  "sourceOfRequest", "userBranch",
  "userSetAddress", "userStation"
};
}
}
