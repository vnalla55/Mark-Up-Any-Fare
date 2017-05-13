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

#include "Pricing/Shopping/Diversity/DmcRequirement.h"

namespace tse
{

std::ostream& operator<<(std::ostream& dc, const DmcRequirement::Printer& manip)
{
  for (size_t i = DmcRequirement::REQUIREMENTS_COUNT; i > 0; --i)
  {
    switch (manip._rbits & (1 << (i - 1)))
    {
    case DmcRequirement::NEED_GOLD:
      dc << 'G';
      break;
    case DmcRequirement::NEED_UGLY:
      dc << 'U';
      break;
    case DmcRequirement::NEED_LUXURY:
      dc << 'L';
      break;
    case DmcRequirement::NEED_JUNK:
      dc << 'J';
      break;
    case DmcRequirement::NEED_CARRIERS:
      dc << 'C';
      break;
    case DmcRequirement::NEED_NONSTOPS:
      dc << 'N';
      break;
    case DmcRequirement::NEED_ADDITIONAL_NONSTOPS:
      dc << 'A';
      break;
    case DmcRequirement::NEED_NONSTOPS_CARRIERS:
      dc << 'S';
      break;
    case DmcRequirement::NEED_CUSTOM:
      dc << 'R';
      break;
    case DmcRequirement::NEED_OUTBOUNDS:
      dc << 'O';
      break;
    case DmcRequirement::NEED_INBOUNDS:
      dc << 'I';
      break;
    case DmcRequirement::NEED_RC_ONLINES:
      dc << 'Q';
      break;
    case DmcRequirement::NEED_IBF_DIRECTS:
      dc << 'D';
      break;
    default:
      dc << '-';
      break;
    }
  }
  return dc;
}

DmcRequirement::SopInfosStatistics::SopInfosStatistics(unsigned legSize,
                                                       MoneyAmount pScore,
                                                       Value pStatus)
  : minimumFlightTimeMinutes(legSize, 0), score(pScore), status(pStatus)
{
}

} // ns tse
