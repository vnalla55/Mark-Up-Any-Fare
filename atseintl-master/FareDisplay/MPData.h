#pragma once

#include "Common/DateTime.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <iosfwd>

namespace tse
{

typedef std::map<GlobalDirection, uint32_t> GlobalMPMMap;
typedef GlobalMPMMap::const_iterator GlobalMPMMapConstIter;
typedef std::vector<GlobalDirection> GlobalVec;
typedef GlobalVec::const_iterator GlobalVecConstIter;

class MPData
{
public:
  void initialize(const LocCode& boardCity,
                  const LocCode& offCity,
                  MoneyAmount,
                  const CurrencyCode&,
                  const DateTime& travelDate,
                  const DateTime& ticketingDate);

  const LocCode& getBoardCity() const { return boardCity_; }

  const LocCode& getOffCity() const { return offCity_; }

  MoneyAmount getAmount() const { return amount_; }

  const CurrencyCode& getCurrency() const { return currency_; }

  const DateTime& getTravelDate() const { return travelDate_; }

  const DateTime& getTicketingDate() const { return ticketingDate_; }

  const GlobalMPMMap& getMPMs() const { return mpms_; }

  GlobalVec getGlobals() const;

  void setMPM(GlobalDirection gd, uint32_t mpm);

private:
  LocCode boardCity_, offCity_;
  MoneyAmount amount_;
  CurrencyCode currency_;
  DateTime travelDate_;
  DateTime ticketingDate_;
  GlobalMPMMap mpms_;
};

std::ostream& operator<<(std::ostream&, const MPData&);

} // namespace tse

