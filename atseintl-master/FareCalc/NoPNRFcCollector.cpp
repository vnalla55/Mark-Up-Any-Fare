#include "FareCalc/NoPNRFcCollector.h"

#include "Common/Logger.h"
#include "DataModel/Fare.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "FareCalc/NoPNRFareCalculation.h"
#include "Rules/RuleConst.h"

#include <iostream>

using namespace tse::FareCalc;
namespace tse
{
namespace
{
Logger
logger("atseintl.FareCalc.NoPNRFcCollector");

struct RuleWarningDescription
{
  RuleWarningDescription(const std::string& text, bool displSegVec, bool displForPU)
    : warningText(text), displaySegmentVector(displSegVec), displaySegmentsForPU(displForPU)
  {
  }

  RuleWarningDescription() = default;

  std::string warningText;
  bool displaySegmentVector = false;
  bool displaySegmentsForPU = false;
};

struct RuleWarningDescriptionsInitializer
{
  std::map<int, RuleWarningDescription> ruleWarningDescriptions;

  RuleWarningDescriptionsInitializer()
  {
    ruleWarningDescriptions.clear();

    ruleWarningDescriptions[WarningMap::cat2_warning_1] =
        RuleWarningDescription("VERIFY DAY OF WEEK RESTRICTIONS", true, false);
    ruleWarningDescriptions[WarningMap::cat2_warning_2] =
        RuleWarningDescription("VERIFY TIME RESTRICTIONS", true, false);
    ruleWarningDescriptions[WarningMap::cat4_warning] =
        RuleWarningDescription("FLIGHT RESTRICTIONS APPLY", true, false);

    ruleWarningDescriptions[WarningMap::cat5_warning_1] =
        RuleWarningDescription("ADV RES/TICKETING REQUIRED", true, true);
    ruleWarningDescriptions[WarningMap::cat5_warning_2] =
        RuleWarningDescription("CONFIRMED RES REQUIRED", true, false);

    ruleWarningDescriptions[WarningMap::cat6_warning] =
        RuleWarningDescription("MIN/MAX STAY REQUIREMENTS APPLY", true, true);
    ruleWarningDescriptions[WarningMap::cat7_warning] =
        RuleWarningDescription("MIN/MAX STAY REQUIREMENTS APPLY", true, true);
    ruleWarningDescriptions[WarningMap::cat11_warning] =
        RuleWarningDescription("VERIFY BLACKOUT DATES", true, false);
    ruleWarningDescriptions[WarningMap::cat12_warning] =
        RuleWarningDescription("SURCHARGES MAY APPLY", true, false);
    ruleWarningDescriptions[WarningMap::cat13_warning] =
        RuleWarningDescription("ACCOMPANYING TRAVEL RESTRICTIONS APPLY", false, false);
    ruleWarningDescriptions[WarningMap::cat14_warning] =
        RuleWarningDescription("TRAVEL RESTRICTIONS MAY APPLY", true, false);
    ruleWarningDescriptions[WarningMap::cat15_warning_1] =
        RuleWarningDescription("VERIFY TKT STOCK RESTRICTIONS", true, false);
    ruleWarningDescriptions[WarningMap::cat15_warning_2] =
        RuleWarningDescription("PAYMENT/TKT RESTRICTIONS APPLY", true, false);
    ruleWarningDescriptions[WarningMap::cat19_22_warning] =
        RuleWarningDescription("ACCOMPANYING TRAVEL RESTRICTIONS APPLY", false, false);
  }
} __ruleWarningDescriptionsInitializer; // automaticly initializes the map in constructor

std::map<int, RuleWarningDescription>& ruleWarningDescriptions =
    __ruleWarningDescriptionsInitializer.ruleWarningDescriptions;
}

void
NoPNRFcCollector::collectMessage()
{
  // call base version
  FcCollector::collectMessage();
  // additionaly collect airport and tax specific messages
  collectAirportSpecificMessages();
  collectTaxWarningMessages();
  collectRuleMessages();
}

void
NoPNRFcCollector::collectRuleMessages()
{
  // first, collect indices for all categories
  std::vector<std::vector<int> > warningIndices;
  warningIndices.resize(WarningMap::map_size);

  for (int warningIdx = 0; warningIdx < WarningMap::map_size; ++warningIdx)
  {
    if (ruleWarningDescriptions.find(warningIdx) == ruleWarningDescriptions.end())
    {
      LOG4CXX_ERROR(logger,
                    "NoPNRFcCollector: no warning defined for warning no "
                        << warningIdx << " , skipping that warning!");
      continue;
    }

    for (const auto pu : _farePath->pricingUnit())
    {
      for (std::vector<FareUsage*>::const_iterator fuIt = pu->fareUsage().begin();
           fuIt != pu->fareUsage().end();
           ++fuIt)
      {
        if (*fuIt == nullptr || (*fuIt)->paxTypeFare() == nullptr)
          continue;

        const PaxTypeFare* ptFare = (*fuIt)->paxTypeFare();

        const WarningMap& warningMap = ptFare->warningMap();
        const WarningMap* discountBaseMap = nullptr;
        if (ptFare->paxTypeFareRuleData(RuleConst::CHILDREN_DISCOUNT_RULE) != nullptr)
          discountBaseMap = &ptFare->paxTypeFareRuleData(RuleConst::CHILDREN_DISCOUNT_RULE)
                                 ->baseFare()
                                 ->warningMap();

        std::vector<int>& indicesForWarning =
            warningIdx == WarningMap::cat7_warning
                ? // for cat 7 - we're showing the same message as for cat 6
                warningIndices.at(WarningMap::cat6_warning)
                : warningIndices.at(warningIdx);

        if (!(warningMap.isSet((WarningMap::WarningID)warningIdx) ||
              (discountBaseMap && discountBaseMap->isSet((WarningMap::WarningID)warningIdx))))
          continue;

        // categories 13/19-22 - skip warning, if restrictions are actually for other pax types
        // and don't apply for this CalcTotals
        if (((warningIdx == WarningMap::cat19_22_warning ||
              warningIdx == WarningMap::cat13_warning) &&
             !_calcTotals->wpaInfo.reqAccTvl))
          continue;

        // if fare isn't discounted - cat 19 warning shouldn't be displayed
        if (warningIdx == WarningMap::cat19_22_warning && !ptFare->isDiscounted())
          continue;

        const std::vector<TravelSeg*>* segmentsVec = nullptr;
        // different logic for cat5, warning 2 - must get
        // proper segment indices from itinerary object
        if (warningIdx == WarningMap::cat5_warning_2)
        {
          for (size_t i = 1; i <= _trx->itin().front()->travelSeg().size(); ++i)
          {
            if (warningMap.isCat5WqWarning(i))
              indicesForWarning.push_back(i);
          }

          continue; // we are done for the whole pricing unit now; do not need to
          // continue with other fare usages for this category
        }
        else
        {

          if (ruleWarningDescriptions[warningIdx].displaySegmentsForPU) // collecting indices of
                                                                        // whole PU
          {
            segmentsVec = &pu->travelSeg();
          }
          else // collecting indices segment indices of only paxTypeFare
          {
            segmentsVec = &(*fuIt)->travelSeg();
          }

          if (segmentsVec == nullptr)
            continue;

          for (const auto ts : *segmentsVec)
          {
            indicesForWarning.push_back(_trx->itin().front()->segmentOrder(ts));
          }
        }
      }
    }
  }

  // now, create the messages
  for (int warningIdx = 0; warningIdx < WarningMap::map_size; ++warningIdx)
  {
    if (ruleWarningDescriptions.find(warningIdx) == ruleWarningDescriptions.end())
    {
      LOG4CXX_ERROR(logger,
                    "NoPNRFcCollector: no warning defined for warning no "
                        << warningIdx << " , skipping that warning!");
      continue;
    }

    std::vector<int>& indicesForWarning = warningIndices.at(warningIdx);

    if (indicesForWarning.size() == 0)
      continue; // no warning for this category

    std::ostringstream ruleWarningStream;

    // append 3-digit ordering prefix (must not be displayed in NoPNRFareCalculation)
    ruleWarningStream << std::setw(3) << std::setfill('0') << warningIdx;
    std::string orderingPrefix = ruleWarningStream.str();

    // get the message
    std::string primaryMessageText = ruleWarningDescriptions[warningIdx].warningText;
    ruleWarningStream << primaryMessageText;

    if (ruleWarningDescriptions[warningIdx].displaySegmentVector) // should indices be appended ?
    {
      // collect the indices
      std::sort(indicesForWarning.begin(), indicesForWarning.end());

      // remove duplicates
      std::vector<int>::iterator newLast =
          std::unique(indicesForWarning.begin(), indicesForWarning.end());
      indicesForWarning.erase(newLast, indicesForWarning.end());
      NoPNRFareCalculation::displayIndicesVector(
          " SEG ", indicesForWarning, "", true, ruleWarningStream, false, false);
    }

    std::string secondaryMessageText = ruleWarningStream.str();

    // add the message with indices for secondary WQ display
    FcMessage secondaryMessage(FcMessage::NOPNR_RULE_WARNING, 0, secondaryMessageText);
    _calcTotals->fcMessage.push_back(secondaryMessage);

    // add the multi-message (without indices) for primary WQ display

    FcMessage primaryMessage(FcMessage::NOPNR_RULE_WARNING, 0, orderingPrefix + primaryMessageText);
    _fcCollector->addMultiMessage(*_trx, _calcTotals->farePath, primaryMessage);
  }
}
}
