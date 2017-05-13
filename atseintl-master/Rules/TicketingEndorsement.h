#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/PaxTypeFare.h"

#include <set>
#include <string>

namespace tse
{

class TicketEndorsementsInfo;
class PricingTrx;
class FareUsage;
class FarePath;
class TravelSeg;
class Diag858Collector;
class DCFactory;

class TicketEndorseItem
{
public:
  TicketEndorseItem() = default;
  TicketEndorseItem(const TicketEndorsementsInfo& tei, const PaxTypeFare& fare);
  bool operator==(const TicketEndorseItem& teItem)
  {
    return endorsementTxt == teItem.endorsementTxt;
  }

  uint32_t priorityCode = 950; // Priority code for sorting
  CarrierCode carrier; // Carrier from the fare that has endorsement
  std::string endorsementTxt; // Endorsement text string
  Indicator tktLocInd = ' '; // Ticket Location Indicator

  const PaxTypeFare* paxTypeFare = nullptr;
  uint32_t itemNo = 0; // Item Number
};

class TicketEndorseLine
{
public:
  TicketEndorseLine() = default;
  explicit TicketEndorseLine(const TicketEndorseItem& item)
    : endorseMessage(item.endorsementTxt), priorityCode(item.priorityCode), carrier(item.carrier)
  {
// ### remove together with fallbackEndorsementsRefactoring ########################################
    if (item.paxTypeFare)
      addSegmentOrder(item.paxTypeFare->fareMarket()->travelSeg());
//##################################################################################################
  }
  bool operator==(const std::string& msg)
  {
    return endorseMessage == msg;
  }

// ### remove together with fallbackEndorsementsRefactoring ########################################
  void addSegmentOrder(const std::vector<TravelSeg*>& travelSegs);
//##################################################################################################

  std::string endorseMessage;
  std::vector<std::string> endorseFormatedMessage;
  std::set<int16_t> segmentOrders;
  // these two members needs for the WPResponseFormatter to send back to PSS
  uint32_t priorityCode = 0; // Priority code
  CarrierCode carrier; // Carrier from the fare that has endorsement
};

class EndorseCutter
{
public:
  void operator()(const std::vector<TicketEndorseItem>& items, TicketEndorseLine* target) const;

// ### remove together with fallbackEndorsementsRefactoring #######################################
  using EndorseItemItr = std::vector<TicketEndorseItem>::iterator;
  using EndorseItemConstItr = std::vector<TicketEndorseItem>::const_iterator;

  virtual void operator()(EndorseItemConstItr begin,
                          EndorseItemConstItr end,
                          TicketEndorseLine* target) const /*= 0*/;

  virtual ~EndorseCutter() = default;
//##################################################################################################
};

// ### remove together with fallbackEndorsementsRefactoring #######################################
class EndorseCutterLimited : public EndorseCutter
{
public:
  EndorseCutterLimited(int maxLen) : _maxLen(maxLen) {}

  void operator()(EndorseItemConstItr begin,
                  EndorseItemConstItr end,
                  TicketEndorseLine* target) const override;

protected:
  size_t _maxLen;
};

class EndorseCutterUnlimited : public EndorseCutter
{
public:
  void operator()(EndorseItemConstItr begin,
                  EndorseItemConstItr end,
                  TicketEndorseLine* target) const override;
};
//##################################################################################################

class TicketingEndorsement // Cat18
{
  friend class TicketingEndorsementTest;

public:
  using TicketEndoLines = std::vector<TicketEndorseLine*>;

// ### remove together with fallbackEndorsementsRefactoring #######################################
  static const uint16_t MAX_TKT_ENDORSEMENT_LINE_SIZE = 60;
  static const uint16_t MAX_TKT_ENDORSEMENT_LINE_SIZE_FOR_ABACUS = 147;

  static const uint32_t SABRE_VIEW_MSG_SIZE = 63;
  static const uint32_t SABRE_VIEW_MSG_WITH_ENDOS_SIZE = 57;
//##################################################################################################

  static constexpr Indicator TKTLOCIND_FOP_1 = '1';
  static constexpr Indicator TKTLOCIND_2 = '2';
  static constexpr Indicator TKTLOCIND_FOP_3 = '3';
  static constexpr Indicator TKTLOCIND_4 = '4';
  static constexpr Indicator TKTLOCIND_FOP_5 = '5';
  static constexpr Indicator TKTLOCIND_6 = '6';

  TicketingEndorsement() = default;

  void initialize(const TicketEndorsementsInfo* teInfo) { _teInfo = teInfo; }

  Record3ReturnTypes process(PricingTrx& trx, FareUsage& fareUsage, const PaxTypeFare& fare);

// ### remove together with fallbackEndorsementsRefactoring #######################################
  static std::string trimEndorsementMsg(const PricingTrx& trx, const std::string& endorsementMsg);

  static uint16_t maxEndorsementMsgLen(const PricingTrx& trx);
//##################################################################################################

  TicketEndorseLine* sortAndGlue(const PricingTrx& trx,
                                 const Itin& itin,
                                 FareUsage& fareUsage,
                                 const EndorseCutter& endoCat,
                                 Diag858Collector* dc = nullptr);

  void collectEndorsements(const PricingTrx& trx,
                           const FarePath& farePath,
                           std::vector<TicketEndorseLine*>& message,
                           const EndorseCutter& endoCat);

  void sortLinesByPnr(const PricingTrx& trx, std::vector<TicketEndorseLine*>& messages);

  void sortLinesByPrio(const PricingTrx& trx,
                       const FarePath& farePath,
                       std::vector<TicketEndorseLine*>& messages);

  void removeTheSameTexts(std::vector<TicketEndorseLine*>& messages) const;

  // ### remove together with fallbackEndorsementsRefactoring #######################################
  void removeTheSameTextsOld(std::vector<TicketEndorseLine*>& messages) const;

  static void glueEndorsementMessageTktEnd(std::string& target, const std::string& source);
//##################################################################################################

protected:
  bool checkTicketEndoLines(const std::vector<TicketEndorseLine*>& messages);

  void
  insertIfOrReplaceIf(std::vector<TicketEndorseItem>& endorsements, const PaxTypeFare& fare) const;

private:
// ### remove together with fallbackEndorsementsRefactoring #######################################
  class LineComparator
  {
  public:
    bool operator()(TicketEndorseLine* item)
    {
      TSE_ASSERT(item && _toFind); // Empty ptrs
      return item->endorseMessage == _toFind->endorseMessage;
    }

    LineComparator(TicketEndorseLine* toFind) : _toFind(toFind) {}

  private:
    TicketEndorseLine* _toFind = nullptr;
  };
//##################################################################################################

  bool shouldBeRemoved(TicketEndorseItem& item)
  {
    return item.tktLocInd == TicketingEndorsement::TKTLOCIND_FOP_1 ||
           item.tktLocInd == TicketingEndorsement::TKTLOCIND_FOP_3 ||
           item.tktLocInd == TicketingEndorsement::TKTLOCIND_FOP_5;
  }

  static bool
  lowerPriorityNumTktEnd(const TicketEndorseItem& item1, const TicketEndorseItem& item2);

  static std::vector<TicketEndorseItem>::const_iterator
  eraseNotWantedMsgsTktEnd(std::vector<TicketEndorseItem>& endorsements, bool isDFF);

  void eraseNotWantedMsgsTktEnd(std::vector<TicketEndorseItem>& endorsements,
                                std::vector<TicketEndorseItem>& endoOut,
                                bool isDFF);

  static bool isEndorseUser(const PricingTrx& trx); // need to add condition for hosted carrier
  void addSegmentOrders(TicketEndorseLine* tel, const TicketEndorseItem& tei, const Itin& itin);

  const TicketEndorsementsInfo* _teInfo = nullptr;
  DCFactory* _factory = nullptr;
  Diag858Collector* _diagPtr = nullptr;
};
}
