//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------

#pragma once

#include "DataModel/FrequentFlyerTrx.h"
#include "Xform/ResponseFormatter.h"

#include <string>

namespace tse
{
class FrequentFlyerResponseFormatter : public ResponseFormatter
{
public:
  FrequentFlyerResponseFormatter(const FrequentFlyerTrx& trx) : ResponseFormatter(), _trx(trx) {}
  void formatResponse(const ErrorResponseException& ere, std::string& response);
  std::string formatResponse();
  void prepareMessage(XMLConstruct& construct,
                      const char msgType,
                      const uint16_t lineNum,
                      const std::string& msgText);

private:
  const FrequentFlyerTrx& _trx;

  using CarrierData = FrequentFlyerTrx::CarriersData::value_type;
  void formatFFData(XMLConstruct&, const CarrierData&);
  void formatTierData(XMLConstruct&, const FreqFlyerStatusData&);
};

} // namespace tse
