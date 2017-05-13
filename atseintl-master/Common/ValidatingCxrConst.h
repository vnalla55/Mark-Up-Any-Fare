#pragma once
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <sstream>
#include <string>

namespace tse
{
namespace vcx
{
constexpr uint16_t MAX_PARTICIPATING_CARRIERS = 48;
constexpr uint16_t MAX_ALTERNATE_CARRIERS = 23;

extern const CarrierCode ALL_COUNTRIES;

constexpr TicketMethod TM_EMPTY = ' ';
constexpr TicketMethod TM_ELECTRONIC = 'E';
constexpr TicketMethod TM_PAPER = 'P';

extern const Alpha3Char AGR_THIRD_PARTY;
extern const Alpha3Char AGR_STANDARD;
extern const Alpha3Char AGR_PAPER;

extern const SettlementPlanType HIERARCHY[];
extern const std::vector<SettlementPlanType> SP_HIERARCHY;

enum TCSRequestType {
  NO_REQ,
  PLAUSIBILITY_CHECK,
  SETTLEMENTPLAN_CHECK,
  DISPLAY_VCXR,
  DISPLAY_INTERLINE_AGMT
};

enum TicketType
{ NO_TKT_TYPE = 0,
  ETKT_PREF,
  ETKT_REQ,
  PAPER_TKT_PREF,
  PAPER_TKT_REQ, // 4
  MAX_TKT_TYPE };

inline const char*
getTicketTypeText(TicketType ind)
{
  static const char* tktTypeText[] = { "NoTktType", "ETKTPREF", "ETKTREQ", "PTKTPREF",
                                       "PTKTREQ" // 4
  };

  if (ind <= 0 || ind >= MAX_TKT_TYPE)
    return "";
  return tktTypeText[ind];
}

inline TicketType
getTicketType(const std::string& str)
{
  if (str == "ETKTPREF")
    return ETKT_PREF;
  else if (str == "ETKTREQ")
    return ETKT_REQ;
  else if (str == "PTKTPREF")
    return PAPER_TKT_PREF;
  else if (str == "PTKTREQ")
    return PAPER_TKT_REQ;
  else
    return NO_TKT_TYPE;
}

enum ValidationStatus {
  NO_MSG = 0,
  VALID_MSG,
  VALID_OVERRIDE,
  VALID_SINGLE_GSA,
  VALID_MULTIPLE_GSA,
  ALTERNATE_CXR,
  OPTIONAL_CXR,
  HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH,
  INVALID_SETTLEMENTPLAN_ERR,
  CXR_DOESNOT_PARTICIPATES_IN_SETTLEMENTPLAN,
  PAPER_TKT_OVERRIDE_ERR,
  NO_VALID_TKT_AGMT_FOUND,
  INVALID_COUNTRYCODE_ERR,
  MAX_VALIDATION_STATUS
};

inline const char*
getValidationCxrMsg(ValidationStatus ind)
{
  static const char* validationCxrMsg[] = {
    "NoMsg", // 0
    "VALIDATING CARRIER - ", //VALID_MSG,
    "VALIDATING CARRIER SPECIFIED - ", //VALID_OVERRIDE,
    "VALIDATING CARRIER - |XX| PER GSA AGREEMENT WITH |ZZ|", // VALID_SINGLE_GSA,
    "VALIDATING CARRIER - ", // VALID_MULTIPLE_GSA,
    "ALTERNATE VALIDATING CARRIER/S - ", // ALTERNATE_CXR
    "OPTIONAL VALIDATING CARRIERS - ", // OPTIONAL_CXR,
    " HAS NO INTERLINE TICKETING AGREEMENT WITH ", //HAS_NO_INTERLINE_TICKETING_AGREEMENT_WITH,
    "INVALID SETTLEMENT METHOD FOR POINT OF SALE",// INVALID_SETTLEMENTPLAN_ERR
     " NOT VALID FOR SETTLEMENT METHOD", // CXR_DOESNOT_PARTICIPATES_IN_SETTLEMENTPLAN
    "PAPER TICKET NOT PERMITTED", //PAPER_TKT_OVERRIDE_ERR
    "NO VALID TICKETING AGREEMENTS FOUND", // NO_VALID_TKT_AGMT_FOUND, //15
    "INVALID COUNTRY CODE" // INVALID_COUNTRYCODE_ERR
  };

  if (ind <= 0 || ind >= MAX_VALIDATION_STATUS)
    return "";
  return validationCxrMsg[ind];
}

enum ValidationResult
{ NO_RESULT = 0,
  VALID,
  VALID_SINGLE_GSA_SWAP,
  VALID_MULTIPLE_GSA_SWAP,
  NOT_VALID,
  ERROR,
  MAX_VALIDATION_RESULT };

inline const char*
getValidationResultText(ValidationResult ind)
{
  static const char* validationResultMsg[] = {
    "NoRes", "Valid", "ValidSingleGSASwap", "ValidMultipleGSASwap", "NotValid",
    "Error" // 4
  };

  if (ind <= 0 && ind >= MAX_VALIDATION_RESULT)
    return "";
  return validationResultMsg[ind];
}

enum ITAgreementType
{ NO_AGMT,
  STANDARD,
  THIRD_PARTY,
  PAPER_ONLY,
  MAX_IT_TYPE };

inline const char* getITAgreementTypeCodeText(ITAgreementType ind)
{
  static const char* iaTypeCodeText[]={
    "",
    "STD",
    "3PT",
    "PPR"
  };

  if (ind < 0 || ind > MAX_IT_TYPE)
    return "";

  return iaTypeCodeText[ind];
}

inline ITAgreementType getITAgreementType(const Alpha3Char& agreementTypeCode)
{
  if ( AGR_STANDARD == agreementTypeCode )
    return STANDARD;
  else if ( AGR_THIRD_PARTY == agreementTypeCode )
    return THIRD_PARTY;
  else if ( AGR_PAPER == agreementTypeCode )
    return PAPER_ONLY;
  else
    return NO_AGMT;
}

struct ParticipatingCxr
{
  ParticipatingCxr(CarrierCode cxr, ITAgreementType agrType) : cxrName(cxr), agmtType(agrType) {}
  ParticipatingCxr(CarrierCode cxr) : cxrName(cxr), agmtType(NO_AGMT) {}
  ParticipatingCxr() : cxrName(""), agmtType(NO_AGMT) {}
  CarrierCode cxrName;
  ITAgreementType agmtType;
};

struct ValidatingCxrData
{
  ValidatingCxrData():
    ticketType(ETKT_PREF),
    interlineFailedCxr(""),
    interlineStatusCode(NO_MSG){}

  TicketType ticketType;
  std::vector<ParticipatingCxr> participatingCxrs;
  CarrierCode interlineFailedCxr;
  ValidationStatus interlineStatusCode;
  std::vector<NationCode> interlineValidCountries;
};

struct Pos
{
  Pos(NationCode countryCode, CrsCode gds) : primeHost(gds), country(countryCode) {}
  Pos() : primeHost(""), country("") {}
  CrsCode primeHost;
  NationCode country;
};

inline void
join(std::string& str, const std::vector<CarrierCode>& v, char d)
{
  if (v.empty())
    return;

  std::vector<CarrierCode>::const_iterator it = v.begin();
  std::stringstream out;
  out << *it++;
  for (; it != v.end(); ++it)
    out << d << *it;
  str += out.str();
}

inline std::string getSettlementPlanName(const SettlementPlanType spType)
{
  static std::map<SettlementPlanType, std::string> settlementPlans = {
    {"ARC", "AIRLINE REPORTING CORPORATION"},
    {"BSP", "BILLING AND SETTLEMENT PLAN"},
    {"SAT", "STUDENT AIRLINE TICKETING AGREEMENT"},
    {"KRY", "STUDENT TICKETING KILROY"},
    {"PRT", "PHILIPPINES TRANSITIONAL AIRLINE TICKET"},
    {"RUT", "RUSSIAN TRANSITIONAL AIRLINE TICKET"},
    {"GEN", "GENERIC TRANSITIONAL AIRLINE TICKET"},
    {"TCH", "TRANSPORT CLEARING HOUSE"},
    {"GTC", "GUARANTEED TICKETING CARRIERS"},
    {"IPC", "INSTANT PURCHASE CARRIERS"},
    {"NSP", "NON SETTLEMENT PLAN CARRIERS"}
  };
  std::map<SettlementPlanType, std::string>::const_iterator it = settlementPlans.find(spType);
  return it != settlementPlans.end() ? it->second : "";
}

} // vcx
} // tse

