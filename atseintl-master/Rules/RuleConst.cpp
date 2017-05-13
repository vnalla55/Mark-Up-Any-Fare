#include "Rules/RuleConst.h"

namespace tse
{
namespace RuleConst
{
const RuleNumber NULL_GENERAL_RULE = "0000";
const Zone NOT_APPLICABLE_ZONE = "0000000";

const std::set<uint16_t> CATEGORY_NOT_FOOTNOTE = {ELIGIBILITY_RULE,
                                                  ADVANCE_RESERVATION_RULE,
                                                  MINIMUM_STAY_RULE,
                                                  ACCOMPANIED_PSG_RULE,
                                                  MAXIMUM_STAY_RULE,
                                                  TRANSFER_RULE,
                                                  VOLUNTARY_EXCHANGE_RULE};

const std::set<uint16_t> CATEGORY_GRULE_PROCESS = {ADVANCE_RESERVATION_RULE,
                                                   STOPOVER_RULE,
                                                   BLACKOUTS_RULE,
                                                   SURCHARGE_RULE,
                                                   SALE_RESTRICTIONS_RULE,
                                                   PENALTIES_RULE};

// Web Travel Agencies
const std::set<LocCode> WEB_AGENCIES = {LocCode("BO44")};
const std::set<LocCode> NON_TVL_WEB_AGENCIES = {LocCode("F6IC")};

inline static const std::set<LocCode>&
getWebAgencies(RuleConst::WebAgentLocation agentLoc)
{
  return (agentLoc == TRAVELOCITY) ? WEB_AGENCIES : NON_TVL_WEB_AGENCIES;
}

bool
isWebAgencyPresent(const LocCode& agency, WebAgentLocation agentLoc)
{
  const std::set<LocCode>& webAgencySet = getWebAgencies(agentLoc);
  return webAgencySet.find(agency) != webAgencySet.end();
}

// Defined Constants for the SalesRestriction Cat15:
const LocCode DFW_CITY = "DFW";
const LocCode HDQ_CITY = "HDQ";

const CarrierCode SABRE1B = "1B";
const CarrierCode SABRE1J = "1J";
const CarrierCode SABRE1W = "1W";
const CarrierCode SABRE1F = "1F";
const CarrierCode SABRE1S = "1S";
const CarrierCode JOINT_CARRIER = "*J";
const PseudoCityCode WEB_FARE_PCC = "BO44";
const PaxTypeCode MATCH_NEG_PAX_TYPE = "NEG";

const CarrierCode AA_CARRIER = "AA";
const EquipmentType M80_EQUIPMENT_TYPE = "M80"; // M80
const EquipmentType S80_EQUIPMENT_TYPE = "S80"; // S80

const std::string MATCH_STOP_OVER_DESC = "STOPOVER";
const std::string MATCH_INBOUND_DESC = "INBOUND ";
const std::string MATCH_OUTBOUND_DESC = "OUTBOUND";
const std::string MATCH_FURTHEST_DESC = "FURTHEST";
const std::string MATCH_DOMESTIC_DESC = "DOMESTIC";
const std::string MATCH_ONE_COUNTRY_DESC = "ONECNTRY";
const std::string MATCH_INTERNATIONAL_DESC = "INTL    ";
const std::string MATCH_GATEWAY_DESC = "GATEWAY ";
const std::string MATCH_ORIG_GATEWAY_DESC = "O-GATEWY";
const std::string MATCH_DEST_GATEWAY_DESC = "D-GATEWY";
const std::string MATCH_TRANS_ATLANTIC_DESC = "TRANSATL";
const std::string MATCH_TRANS_PACIFIC_DESC = "TRANSPAC";
const std::string MATCH_TRANS_OCEANIC_DESC = "TRANSOCN";
const std::string MATCH_INTERCONTINENTAL_DESC = "INTERCON";
const std::string MATCH_OVER_WATER_DESC = "OVERWATR";
const std::string MATCH_INTL_DOM_TRANSFER_DESC = "INDOMTFR";

const std::string NO_MATCH_TURNAROUND_POINT = "TURN-PNT";
const std::string NO_MATCH_LAST_SEGMENT = "LAST-SEG";

const std::string DIAGNOSTIC_INCLUDE_GEO = "GEO";
} // ns RuleConst
} // tse
