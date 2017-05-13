// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
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

#include "Common/ErrorResponseException.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <string>

namespace tse
{
class OCFees;

class AncillaryIdentifier
{
public:
  AncillaryIdentifier(std::string ancillaryIdentifier)
  {
    if (!validate(ancillaryIdentifier))
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "Validating attribute ITN/OSC/@AID failed");
    _identifier = ancillaryIdentifier;
  }

  AncillaryIdentifier(const OCFees& ocFees)
  {
    _identifier = generate(ocFees);
    if (!validate(_identifier))
      throw ErrorResponseException(ErrorResponseException::SYSTEM_ERROR, "Generating attribute //OOS/@AID failed");
  }

  const std::string getIdentifier() const { return _identifier; }

  bool operator< (const AncillaryIdentifier& rhs) const
  {
    return getIdentifier() < rhs.getIdentifier();
  }

private:
  bool validate(std::string ancillaryIdentifier);
  std::string generate(const OCFees& ocFees);
  std::string generateCarrierCode(const OCFees& ocFees);
  std::string generateServiceType(const OCFees& ocFees);
  std::string generateSubCode(const OCFees& ocFees);
  std::string generateSegmentList(const OCFees& ocFees);
  std::string generateS7Sequence(const OCFees& ocFees);
  bool hasPadis(const OCFees& ocFees);
  std::string generatePadisSequence(const OCFees& ocFees);

  std::string _identifier;
};

}
