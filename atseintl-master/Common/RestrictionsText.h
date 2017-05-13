//----------------------------------------------------------------------------
//  Copyright Sabre 2009
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "DataModel/FareDisplayTrx.h"
#include "Routing/RoutingInfo.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class Routing;
class RoutingRestriction;

typedef std::vector<RoutingRestriction*> RtgRests;
typedef RtgRests::const_iterator RtgRestCI;

class RestrictionsText final
{
  friend class RestrictionsTextTest;

public:
  RestrictionsText(std::ostringstream& oss, bool lineWrap = true) : _oss(oss), _lineWrap(lineWrap)
  {
  }

  RestrictionsText(const RestrictionsText& restText) = delete;
  RestrictionsText& operator=(const RestrictionsText& collector) = delete;

  void restriction1(const RoutingRestriction& restriction, const bool& indent);
  void restriction1(RtgRestCI, RtgRestCI, const bool& indent);
  void restriction2(const RoutingRestriction& restriction, const bool& indent);
  void restriction2(RtgRestCI, RtgRestCI, const bool& indent);
  void restriction3(const RoutingRestriction& restriction, const bool& indent);
  void restriction4(const RoutingRestriction& restriction, const bool& indent);
  void restriction5(const RoutingRestriction& restriction, const bool& indent);
  void restriction5(RtgRestCI, RtgRestCI, const bool& indent);
  void restriction6(const RoutingRestriction& restriction, const bool& indent);
  void restriction7(const RoutingRestriction& restriction, const bool& indent);
  void restriction7(RtgRestCI, RtgRestCI, const bool& indent);
  void restriction8FMR(const FareDisplayTrx& trx,
                       const RoutingRestriction& restriction,
                       const bool& indent);
  void restriction8(const RoutingRestriction& restriction, const bool& indent);
  void restriction9FMR(const FareDisplayTrx& trx, const bool& indent);
  void restriction10(const RoutingRestriction& restriction, const bool& indent);
  void restriction10(RtgRestCI, RtgRestCI, const bool& indent);
  void restriction11(const RoutingRestriction& restriction, const bool& indent);
  void restriction12(const RoutingRestriction& restriction, const bool& indent);
  void restriction16(const RoutingRestriction& restriction, const bool& indent);
  void restriction17(const RoutingRestriction& restriction, const bool& indent);
  void restriction17(RtgRestCI, RtgRestCI, const bool& indent);
  void restriction21(const RoutingRestriction& restriction, const bool& indent);
  void restriction21(RtgRestCI, RtgRestCI, const bool& indent);

  static bool isRtwOrCt(const FareDisplayTrx& trx);
private:
  std::ostringstream& oss() { return _oss; }
  const std::ostringstream& oss() const { return _oss; }

  void checkIndent(bool indent);
  void finalizeRestrictionFunction(std::ostringstream& ostr);

  std::string displayCities(const RtgRestCI& it, const RtgRestCI& ie, const std::string& conj);
  std::string displayCarriers(const RtgRestCI& it, const RtgRestCI& ie, const std::string& conj);
  std::string permReqText(Indicator negViaAppl);
  std::string mustBeText(Indicator negViaAppl);
  std::string nonStopText(Indicator nonStopDirectInd);

  std::ostringstream& _oss;
  bool _lineWrap;
};
} // namespace tse
