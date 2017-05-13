// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Common/TseStringTypes.h"
#include "FreeBagService/CarryOnBaggageTextFormatter.h"

namespace tse
{

class OCFees;

class EmbargoesBaggageTextFormatter : public CarryOnBaggageTextFormatter
{
public:
  friend class EmbargoesBaggageTextFormatterTest;

  static const std::string EMBARGO_0ME_SUBCODE;
  static const int32_t EMBARGO_FIRST_OCCURRENCE_BLANK;

  EmbargoesBaggageTextFormatter(PricingTrx& trx);
  virtual ~EmbargoesBaggageTextFormatter();

  std::string formatEmbargoesText(OCFees* embargo);

private:
  std::string formatEmbargoDescriptionText(const SubCodeInfo* s5, const OptionalServicesInfo* s7);
  std::string
  formatEmbargoDescription0METext(const SubCodeInfo* s5, const OptionalServicesInfo* s7);
  std::string
  formatEmbargoDescriptionGeneralText(const SubCodeInfo* s5, const OptionalServicesInfo* s7);
  std::string formatSubCodeTranslation(const SubCodeInfo* s5);
};
}

