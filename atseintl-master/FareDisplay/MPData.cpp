#include "FareDisplay/MPData.h"

#include <iostream>
#include <iterator>
#include <string>

using std::ostream;
using std::string;
using std::ostream_iterator;

namespace
{

template <typename MapType>
struct FirstGetter
{
  typename MapType::key_type operator()(const typename MapType::value_type& pair) const
  {
    return pair.first;
  }
};

/*FirstType getFirst(const ValueType pair)
{
   return pair.first;
}*/
}

namespace tse
{

void
MPData::initialize(const LocCode& boardCity,
                   const LocCode& offCity,
                   MoneyAmount amount,
                   const CurrencyCode& currency,
                   const DateTime& travelDate,
                   const DateTime& ticketingDate)
{
  boardCity_ = boardCity;
  offCity_ = offCity;
  amount_ = amount;
  currency_ = currency;
  travelDate_ = travelDate;
  ticketingDate_ = ticketingDate;
}

GlobalVec
MPData::getGlobals() const
{
  GlobalVec globals;
  transform(mpms_.begin(), mpms_.end(), back_inserter(globals), FirstGetter<GlobalMPMMap>());
  return globals;
}

void
MPData::setMPM(GlobalDirection gd, uint32_t mpm)
{
  mpms_[gd] = mpm;
}

ostream& operator<<(ostream& os, const GlobalMPMMap::value_type& pair)
{
  string gd;
  globalDirectionToStr(gd, pair.first);
  os << gd << "-" << pair.second;
  return os;
}

ostream& operator<<(ostream& os, const MPData& mpData)
{
  os << "market " << mpData.getBoardCity() << "-" << mpData.getOffCity() << "\n"
     << "fare " << mpData.getAmount() << " " << mpData.getCurrency() << "\n"
     << "MPM for globals: ";
  copy(mpData.getMPMs().begin(),
       mpData.getMPMs().end(),
       ostream_iterator<GlobalMPMMap::value_type>(os, " "));
  return os;
}

} // namespace tse
