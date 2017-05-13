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

#include "FreeBagService/EmbargoesBaggageTextFormatter.h"

#include "Common/FreeBaggageUtil.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/ServicesDescription.h"
#include "DBAccess/ServicesSubGroup.h"
#include "DBAccess/SubCodeInfo.h"
#include "FreeBagService/BaggageTextFormatter.h"
#include "ServiceFees/OCFees.h"


namespace tse
{

const std::string EmbargoesBaggageTextFormatter::EMBARGO_0ME_SUBCODE = "0ME";
const int32_t EmbargoesBaggageTextFormatter::EMBARGO_FIRST_OCCURRENCE_BLANK = -1;

EmbargoesBaggageTextFormatter::EmbargoesBaggageTextFormatter(PricingTrx& trx)
  : CarryOnBaggageTextFormatter(trx)
{
}

EmbargoesBaggageTextFormatter::~EmbargoesBaggageTextFormatter() {}

std::string
EmbargoesBaggageTextFormatter::formatEmbargoesText(OCFees* embargo)
{
  return formatEmbargoDescriptionText(embargo->subCodeInfo(), embargo->optFee());
}

std::string
EmbargoesBaggageTextFormatter::formatEmbargoDescriptionText(const SubCodeInfo* s5,
                                                            const OptionalServicesInfo* s7)
{
  std::string result;
  if (!s5 || !s7)
    return result;

  return (s5->serviceSubTypeCode() == EMBARGO_0ME_SUBCODE
              ? formatEmbargoDescription0METext(s5, s7)
              : formatEmbargoDescriptionGeneralText(s5, s7));
}

std::string
EmbargoesBaggageTextFormatter::formatEmbargoDescription0METext(const SubCodeInfo* s5,
                                                               const OptionalServicesInfo* s7)
{
  switch (s7->baggageOccurrenceFirstPc())
  {
  case EMBARGO_FIRST_OCCURRENCE_BLANK:
  case 0:
  case 1:
    return "EXCESS OVER ALLOWANCE NOT PERMITTED\n";
  case 2:
    return "ONLY 1 EXCESS ITEM OVER ALLOWANCE PERMITTED\n";
  default:
  {
    std::stringstream ss;
    ss << "ONLY " << (s7->baggageOccurrenceFirstPc() - 1)
       << " EXCESS ITEMS OVER ALLOWANCE PERMITTED\n";
    return ss.str();
  }
  }
}

std::string
EmbargoesBaggageTextFormatter::formatEmbargoDescriptionGeneralText(const SubCodeInfo* s5,
                                                                   const OptionalServicesInfo* s7)
{
  switch (s7->baggageOccurrenceFirstPc())
  {
  case EMBARGO_FIRST_OCCURRENCE_BLANK:
  case 0:
    return formatSubCodeTranslation(s5) + " NOT PERMITTED\n";
  case 1:
    return formatSubCodeTranslation(s5) + " EXCESS OVER ALLOWANCE NOT PERMITTED\n";
  default:
  {
    std::stringstream ss;
    ss << "ONLY " << (s7->baggageOccurrenceFirstPc() - 1) << " EXCESS "
       << formatSubCodeTranslation(s5) << " PERMITTED\n";
    return ss.str();
  }
  }
  return "";
}

std::string
EmbargoesBaggageTextFormatter::formatSubCodeTranslation(const SubCodeInfo* s5)
{
  std::string retText;
  if (!s5->description1().empty())
  {
    const ServicesDescription* svcDesc1 =
        _trx.dataHandle().getServicesDescription(s5->description1());
    if (svcDesc1)
    {
      retText += getServiceSubGroupText(s5) + svcDesc1->description();

      if (!s5->description2().empty())
      {
        const ServicesDescription* svcDesc2 =
            _trx.dataHandle().getServicesDescription(s5->description2());
        if (svcDesc2)
        {
          retText += ((FreeBaggageUtil::isAlpha(svcDesc1->description()) ||
                       FreeBaggageUtil::isAlpha(svcDesc2->description()))
                          ? SPACE
                          : AND);
          retText += svcDesc2->description();
        }
        else
          return s5->commercialName();
      }
      return retText;
    }
  }

  return s5->commercialName();
}
}
