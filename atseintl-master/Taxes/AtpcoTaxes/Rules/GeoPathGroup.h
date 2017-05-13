
#include "Rules/RulesGroupApplyFunctor.h"

#include "Rules/ContinuousJourneyRule.h"
#include "Rules/JourneyIncludesRule.h"
#include "Rules/JourneyLoc1AsOriginRule.h"
#include "Rules/JourneyLoc1AsOriginExRule.h"
#include "Rules/JourneyLoc2DestinationTurnAroundRule.h"
#include "Rules/ReturnToOriginRule.h"
#include "Rules/TravelWhollyWithinRule.h"

namespace tax
{
struct GeoPathGroup
{
  boost::optional<JourneyLoc1AsOriginRule> _journeyLoc1AsOriginRule;
  boost::optional<JourneyLoc1AsOriginExRule> _journeyLoc1AsOriginExRule;
  boost::optional<JourneyLoc2DestinationTurnAroundRule> _journeyLoc2DestinationTurnAroundRule;
  boost::optional<TravelWhollyWithinRule> _travelWhollyWithinRule;
  boost::optional<JourneyIncludesRule> _journeyIncludesRule;
  boost::optional<ReturnToOriginRule> _returnToOriginRule;
  boost::optional<ContinuousJourneyRule> _continuousJourneyRule;

  template <template <class> class Functor, class ...Args>
  bool foreach(Args&&... args) const
  {
    return apply<JourneyLoc1AsOriginRule, Functor>(
               _journeyLoc1AsOriginRule, std::forward<Args>(args)...) &&
           apply<JourneyLoc1AsOriginExRule, Functor>(
               _journeyLoc1AsOriginExRule, std::forward<Args>(args)...) &&
           apply<JourneyLoc2DestinationTurnAroundRule, Functor>(
               _journeyLoc2DestinationTurnAroundRule, std::forward<Args>(args)...) &&
           apply<JourneyLoc2DestinationTurnAroundRule, Functor>(
               _journeyLoc2DestinationTurnAroundRule, std::forward<Args>(args)...) &&
           apply<TravelWhollyWithinRule, Functor>(
               _travelWhollyWithinRule, std::forward<Args>(args)...) &&
           apply<JourneyIncludesRule, Functor>(
               _journeyIncludesRule, std::forward<Args>(args)...) &&
           apply<ReturnToOriginRule, Functor>(
               _returnToOriginRule, std::forward<Args>(args)...) &&
           apply<ContinuousJourneyRule, Functor>(
               _continuousJourneyRule, std::forward<Args>(args)...);
  }
};
}

