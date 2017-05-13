//----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
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

#include "DataModel/TaxInfoResponse.h"

using namespace tse;

TaxInfoResponse::TaxInfoResponse()
{
  std::get<TAX::CODE>(taxAttrNames()) = "BC0"; // Tax Code
  std::get<TAX::ERROR>(taxAttrNames()) = "S18"; // ErrorMsg
  std::get<TAX::TYPE>(taxAttrNames()) = "A06"; // Tax Type
  std::get<TAX::AMOUNT>(taxAttrNames()) = "C6A"; // Amount
  std::get<TAX::CURRENCY>(taxAttrNames()) = "C41"; // Currency Code
  std::get<TAX::NATION>(taxAttrNames()) = "A40"; // Tax Country Code
  std::get<TAX::DESCRIPTION>(taxAttrNames()) = "S04"; // Tax Description
  std::get<TAX::REFUNDABLE>(taxAttrNames()) = "X21"; // Refundable tag
}

TaxInfoResponse::~TaxInfoResponse() {}

void
TaxInfoResponse::initPFC()
{
  std::get<PFC::AIRPORT>(aptAttrNames()) = "A01"; // Airport Code
  std::get<PFC::ERROR>(aptAttrNames()) = "S04"; // Error Msg
  std::get<PFC::AMOUNT>(aptAttrNames()) = "C6A"; // Amount
  std::get<PFC::CURRENCY>(aptAttrNames()) = "C41"; // Currency Code
  std::get<PFC::EFF_DATE>(aptAttrNames()) = "D06"; // Effective Date
  std::get<PFC::DISC_DATE>(aptAttrNames()) = "D05"; // Discontinue Date
}

void
TaxInfoResponse::initZP()
{
  std::get<ZP::AIRPORT>(aptAttrNames()) = "A01"; // Airport Code
  std::get<ZP::ERROR>(aptAttrNames()) = "S04"; // Error Msg
  std::get<ZP::AMOUNT>(aptAttrNames()) = "C6A"; // Amount
  std::get<ZP::CURRENCY>(aptAttrNames()) = "C41"; // Currency Code
  std::get<ZP::IS_DOMESTIC>(aptAttrNames()) = "PYA"; // US Domestic Airport Indicator (T/F)
  std::get<ZP::IS_RURAL>(aptAttrNames()) = "PYB"; // Rural Airport Indicator (T/F)
}
