#include "Xform/XMLShoppingSoloCarnivalResponse.h"

#include "Common/Assert.h"
#include "Common/FareCalcUtil.h"

namespace tse
{

XMLShoppingSoloCarnivalResponse::XMLShoppingSoloCarnivalResponse(PricingTrx& trx)
  : XMLShoppingResponse(trx)
{
}

void
XMLShoppingSoloCarnivalResponse::generateItineraries()
{
  for (const PricingTrx::SOLItinGroupsMap::value_type& i : _trx.solItinGroupsMap())
  {
    TSE_ASSERT(i.first);
    TSE_ASSERT(i.second);
    Itin* itin = i.first;

    boost::optional<SOLItinGroups::GroupType> cheapestGroup = i.second->getCheapestItinGroup();

    if (!cheapestGroup)
    {
      // it is possible that no itinerary within a group is priced and so it is
      // impossible to determine the cheapest group
      continue;
    }

    if (*cheapestGroup == SOLItinGroups::ORIGINAL)
    {
      ItineraryTotals t;
      generateITNbody(
          itin, "ITN", boost::bind(&XMLShoppingSoloCarnivalResponse::generateSID, this, _1), t);
    }
    else
    {
      const SOLItinGroups::ItinGroup& group = i.second->getItinGroup(*cheapestGroup);
      generateSumOfLocalITNbody(itin, group);
    }
  }
}

void
XMLShoppingSoloCarnivalResponse::generateSumOfLocalITNbody(const Itin* itin,
                                                           const SOLItinGroups::ItinGroup& group)
{
  Node nodeITN(_writer, "ITN");
  generateITNIdAttribute(nodeITN, itin);

  generateSID(itin);

  ItineraryTotals totalsForAllSubItins;

  for (Itin* i : group)
  {
    if(_trx.isFlexFare())
    {
      generateFlexFareGRI(group);
      // we need to run the loop only once as generateFlexFareGRI() will take care of all the itins in the group
      break;
    }
    else
    {
      ItineraryTotals totals;
      generateITNbody(i,
                      "ITT",
                      boost::bind(&XMLShoppingSoloCarnivalResponse::generateSubItinFlights, this, _1),
                      totals);
      totalsForAllSubItins.add(totals, *this);
    }
  }

  bool FlexFareCheckNotRequired = (!_trx.isFlexFare());
  if (FlexFareCheckNotRequired)
  {
    //TOT tag is removed under ITN for flex fare xml responses
    //TOT will still exist for GRI
    generateTOTbody(totalsForAllSubItins);
  }
}

void
XMLShoppingSoloCarnivalResponse::generateFlexFareGRI(const SOLItinGroups::ItinGroup& group)
{
  flexFares::GroupsData::const_iterator iterFlexFareGroup =
      _trx.getRequest()->getFlexFaresGroupsData().begin();
  flexFares::GroupsData::const_iterator iteratorEnd =
      _trx.getRequest()->getFlexFaresGroupsData().end();

  for (; iterFlexFareGroup != iteratorEnd; iterFlexFareGroup++)
  {
    Node nodeGRI(_writer, "GRI");
    nodeGRI.convertAttr("Q17", iterFlexFareGroup->first);

    ItineraryTotals totalsForAllSubItins;
    bool isFFOffered = true;
    for (Itin* i : group)
    {
      if (!isFlexFareGroupValidForItin(i, iterFlexFareGroup->first))
      {
        isFFOffered = false;
        continue;
      }

      isFFOffered = true;
      ItineraryTotals totals;
      generateITNbody(i,
                        "ITT",
                        boost::bind(&XMLShoppingSoloCarnivalResponse::generateSubItinFlights, this, _1),
                        totals,
                        iterFlexFareGroup->first,
                        true);
      totalsForAllSubItins.add(totals, *this);
    }

    if (isFFOffered)
    {
      generateTOTbody(totalsForAllSubItins);
    }
    else
    {
      nodeGRI.convertAttr("SGL", FLEX_FARE_GROUP_NOT_OFFERED);
    }
  }
}

void
XMLShoppingSoloCarnivalResponse::generateSubItinFlights(const Itin* itin)
{
  if (!itin)
  {
    return;
  }

  Node nodeIFL(_writer, "IFL");
  for (const tse::TravelSeg* segment : itin->travelSeg())
  {
    if (segment->isAir())
    {
      Node nodeFID(_writer, "FID");
      nodeFID.convertAttr("Q1K", segment->originalId() - 1);
    }
  }
}
}
