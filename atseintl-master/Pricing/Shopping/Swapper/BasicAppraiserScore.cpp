
//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"

#include "Pricing/Shopping/Utils/StreamFormat.h"

#include <iomanip>

namespace tse
{

namespace swp
{

std::ostream& operator<<(std::ostream& out, BasicAppraiserScore::CATEGORY c)
{
  switch (c)
  {
  case BasicAppraiserScore::WANT_TO_REMOVE:
    out << "WANT_TO_REMOVE";
    break;
  case BasicAppraiserScore::IGNORE:
    out << "IGNORE";
    break;
  case BasicAppraiserScore::NICE_TO_HAVE:
    out << "NICE_TO_HAVE";
    break;
  case BasicAppraiserScore::MUST_HAVE:
    out << "MUST_HAVE";
    break;
  default:
    out << "#BAD VALUE#";
  }
  return out;
}

void
formatBasicAppraiserScore(std::ostream& out, const BasicAppraiserScore& brs)
{
  const unsigned int MAX_CATEGORY_WIDTH = 14;
  out << "Category " << std::setw(MAX_CATEGORY_WIDTH) << brs.getCategory() << ", rank "
      << brs.getMinorRank();
}

void
formatShort(std::ostream& out, const BasicAppraiserScore& brs)
{
  switch (brs.getCategory())
  {
  case BasicAppraiserScore::WANT_TO_REMOVE:
    out << "R";
    break;
  case BasicAppraiserScore::IGNORE:
    out << ".";
    break;
  case BasicAppraiserScore::NICE_TO_HAVE:
    out << "n";
    break;
  case BasicAppraiserScore::MUST_HAVE:
    out << "M";
    break;
  default:
    out << "?";
  }
  const int minorRank = brs.getMinorRank();
  if (minorRank != BasicAppraiserScore::DEFAULT_MINOR_RANK)
  {
    out << "(" << minorRank << ")";
  }
}

void
printBasShortFormatDescription(std::ostream& out)
{
  out << "score = CATEGORY(rank)" << std::endl;
  out << "with categories:" << std::endl;
  out << "  M : must have" << std::endl;
  out << "  n : nice to have" << std::endl;
  out << "  . : ignore" << std::endl;
  out << "  R : want to remove" << std::endl;
  out << "and ranks as integer numbers." << std::endl;
  out << "If rank is not present, equals zero." << std::endl;
}

std::ostream& operator<<(std::ostream& out, const BasicAppraiserScore& brs)
{
  out << tse::utils::format(brs, formatShort);
  return out;
}

} // namespace swp

} // namespace tse
