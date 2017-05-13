#include "Common/ValidatingCxrConst.h"

namespace tse
{
namespace vcx
{
const CarrierCode ALL_COUNTRIES = "ZZ";

const Alpha3Char AGR_THIRD_PARTY = "3PT";
const Alpha3Char AGR_STANDARD = "STD";
const Alpha3Char AGR_PAPER = "PPR";

const SettlementPlanType HIERARCHY[] = {
    "BSP", "ARC", "TCH", "GEN", "RUT", "PRT", "SAT", "KRY", "GTC", "IPC", "NSP"};

const std::vector<SettlementPlanType>
SP_HIERARCHY(HIERARCHY, HIERARCHY + sizeof(HIERARCHY) / sizeof(HIERARCHY[0]));
} // vcx
} // tse
