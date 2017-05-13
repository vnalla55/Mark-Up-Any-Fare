// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "DataModel/AncillaryOptions/AncillaryIdentifier.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "ServiceFees/OCFees.h"

#include <regex>

namespace tse
{

bool
AncillaryIdentifier::validate(std::string ancillaryIdentifier)
{
  // example aid             1S|C|0DF|1.2|1000
  // example aid with padis  1S|C|0DF|1.2|1000|12555 - same as above + padis at the end
  static const std::string aidRegex(R"(^[[:alnum:]]{2}\|[A-Z]\|[[:alnum:]]{3}\|[0-9.]+\|\d+(\|\d+)?$)");
  return std::regex_match(ancillaryIdentifier, std::regex(aidRegex));
}

std::string
AncillaryIdentifier::generate(const OCFees& ocFees)
{
  static const std::string DATA_SEPARATOR = "|";
  std::string aid;
  if (ocFees.subCodeInfo() && ocFees.travelStart() && ocFees.travelEnd() && ocFees.optFee())
  {
    aid = generateCarrierCode(ocFees) + DATA_SEPARATOR
        + generateServiceType(ocFees) + DATA_SEPARATOR
        + generateSubCode(ocFees) + DATA_SEPARATOR
        + generateSegmentList(ocFees) + DATA_SEPARATOR
        + generateS7Sequence(ocFees)
        + (hasPadis(ocFees) ? DATA_SEPARATOR + generatePadisSequence(ocFees) : "");
  }

  return aid;
}

std::string
AncillaryIdentifier::generateCarrierCode(const OCFees& ocFees)
{
  return ocFees.carrierCode();
}

std::string
AncillaryIdentifier::generateServiceType(const OCFees& ocFees)
{
  return std::string(1, ocFees.subCodeInfo()->fltTktMerchInd());
}

std::string
AncillaryIdentifier::generateSubCode(const OCFees& ocFees)
{
  return ocFees.subCodeInfo()->serviceSubTypeCode();
}

std::string
AncillaryIdentifier::generateSegmentList(const OCFees& ocFees)
{
  std::string segList;
  int16_t startSeg = ocFees.travelStart()->pnrSegment();
  int16_t endSeg = ocFees.travelEnd()->pnrSegment();
  for (int16_t i = startSeg; i <= endSeg; ++i)
  {
    segList += std::to_string(i);
    if (i != endSeg)
      segList += ".";
  }
  return segList;
}

std::string
AncillaryIdentifier::generateS7Sequence(const OCFees& ocFees)
{
  return std::to_string(ocFees.optFee()->seqNo());
}

bool
AncillaryIdentifier::hasPadis(const OCFees& ocFees)
{
  return !ocFees.padisData().empty() && ocFees.optFee()->upgrdServiceFeesResBkgDesigTblItemNo();
}

std::string
AncillaryIdentifier::generatePadisSequence(const OCFees& ocFees)
{
  return std::to_string(ocFees.optFee()->upgrdServiceFeesResBkgDesigTblItemNo());
}

}
