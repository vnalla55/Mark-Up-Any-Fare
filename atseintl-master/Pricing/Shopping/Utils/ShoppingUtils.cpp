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

#include "Pricing/Shopping/Utils/ShoppingUtils.h"

#include "Common/DateTime.h"
#include "DataModel/Billing.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"

#include <iomanip>
#include <sstream>

using namespace std;

namespace tse
{

namespace utils
{

namespace
{
std::string
getLeftTag()
{
  return "{";
}
std::string
getRightTag()
{
  return "}";
}
}

std::ostream& operator<<(std::ostream& out, const SopEntry& e)
{
  out << "LEG " << setw(3) << e.legId;
  out << " SOP " << setw(3) << e.sopId;
  return out;
}

std::ostream& operator<<(std::ostream& out, const SopCombination& v)
{
  out << "[";
  for (unsigned int i = 0; i < v.size(); ++i)
  {
    out << v[i];
    if (i != (v.size() - 1))
    {
      out << ", ";
    }
  }
  out << "]";
  return out;
}

std::ostream& operator<<(std::ostream& out, const SopCombinationList& v)
{
  out << "SopCombinationList:\n";
  for (auto& elem : v)
  {
    out << elem << std::endl;
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const SopCandidate& c)
{
  out << getLeftTag() << "SopCandidate ";
  out << "legId: " << c.legId;
  out << ", sopId: " << c.sopId;
  out << ", carrierCode: " << c.carrierCode;
  out << ", isFlightDirect: " << c.isFlightDirect << getRightTag();
  return out;
}

std::ostream& operator<<(std::ostream& out, const TravelSeg& segment)
{
  using namespace std;
  out << segment.origin()->loc();
  out << " " << segment.departureDT();
  out << " - " << segment.destination()->loc();
  out << " " << segment.arrivalDT();
  return out;
}

std::ostream& operator<<(std::ostream& out, const ::tse::ShoppingTrx::SchedulingOption& sop)
{
  using namespace std;
  out << sop.governingCarrier() << " ";

  const Itin* itin = sop.itin();
  TSE_ASSERT(itin != nullptr);
  for (unsigned int i = 0; i < itin->travelSeg().size(); ++i)
  {
    out << *(itin->travelSeg()[i]);
    if (i != (itin->travelSeg().size() - 1))
    {
      out << " # ";
    }
  }
  return out;
}

const ShoppingTrx::SchedulingOption&
findSopInTrx(unsigned int legId, unsigned int sopId, const ShoppingTrx& trx)
{
  TSE_ASSERT(legId < trx.legs().size());
  const ShoppingTrx::Leg& leg = trx.legs()[legId];

  TSE_ASSERT(sopId < leg.sop().size());
  return leg.sop()[sopId];
}

void
addToFlightMatrix(tse::ShoppingTrx& trx,
                  tse::ItinStatistic& stats,
                  const SopCombinationList& options)
{
  for (SopCombinationList::const_iterator it = options.begin(); it < options.end(); it++)
  {
    const ShoppingTrx::FlightMatrix::value_type item(*it, nullptr);
    trx.flightMatrix().insert(item);
    stats.addFOS(*it);
  }
}

void
addToFlightMatrix(ShoppingTrx::FlightMatrix& matrix, const SopCombinationList& options)
{
  for (auto& option : options)
  {
    // FOS <=> zero GroupFarePath pointer
    matrix.insert(std::make_pair(option, (GroupFarePath*)nullptr));
  }
}

CarrierCode
getRequestingCarrierCodeForTrx(const ShoppingTrx& trx)
{
  return trx.billing()->partitionID();
}

std::size_t
hash_value(const SopEntry& se)
{
  std::size_t seed = 0;
  boost::hash_combine(seed, se.legId);
  boost::hash_combine(seed, se.sopId);
  return seed;
}

} // namespace utils

} // namespace tse
