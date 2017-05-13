//----------------------------------------------------------------------------
//  File:         DiagCollector.h
//  Description:  Diagnostic Collector base class: Defines all the virtual methods
//                derived class may orverride these methods.
//  Authors:      Mohammad Hossan
//  Created:      Dec 2003
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2003
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

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Diagnostic/DiagVisitor.h"
#include "Util/BranchPrediction.h"

#include <ostream>
#include <sstream>
#include <stdarg.h>
#include <string>

namespace tse
{
class Agent;
class Airport;
class AirSeg;
class BaseFareRule;
class BookingCodeExceptionSequence;
class CarrierPreference;
class FPPQItem;
class Fare;
class FareByRuleApp;
class FarePath;
class FareMarket;
class MergedFareMarket;
class FareMarketPath;
class FareMarketPathMatrix;
class PU;
class PUPath;
class PUPathMatrix;
class FareUsage;
class Itin;
class PricingTrx;
class PaxType;
class PaxTypeFare;
class SurfaceSeg;
class TaxOut;
class TaxCodeReg;
class TaxResponse;
class PfcItem;
class TaxRecord;
class TaxItem;
class TravelSeg;
class CarrierMixedClass;
class Money;
class NegFareSecurityInfo;
class NegFareCalcInfo;
class MarkupCalculate;
class CategoryRuleItemInfo;
class CabinType;
class FareTypeDesignator;
class GroupFarePath;
class NegFareRestExtSeq;
class PenaltyInfo;
class VoluntaryChangesInfo;
class VoluntaryRefundsInfo;

class DiagCollector : public std::ostringstream
{
  friend class DiagCollectorTest;

public:
  enum DiagWrapMode
  {
    DiagWrapNone = 0,
    DiagWrapSimple = 1,
    DiagWrapAligned = 2
  };

  class Header {};

  static const uint32_t DEFAULT_LINEWRAP_LENGTH = 63;

  //@TODO will be removed, once the transition is done
  explicit DiagCollector(Diagnostic& root) : _rootDiag(&root)
  {
    if (root.isActive())
    {
      _diagnosticType = root.diagnosticType();
    }
  }

  DiagCollector() = default;
  DiagCollector(const DiagCollector&) = delete;
  DiagCollector& operator=(const DiagCollector&) = delete;

  virtual ~DiagCollector() = default;

  virtual void accept(DiagVisitor& diagVisitor)
  {
    diagVisitor.visit(*this);
  }

  struct ItinPUPath
  {
    ItinPUPath() : itin(nullptr), puPath(nullptr) {}
    const Itin* itin;
    PUPath* puPath;
  };

  virtual DiagCollector& operator<<(DiagCollector& rhs);

  virtual DiagCollector& operator<<(int x);
  virtual DiagCollector& operator<<(long x);
  virtual DiagCollector& operator<<(float x);
  virtual DiagCollector& operator<<(double x);
  virtual DiagCollector& operator<<(uint16_t x);
  virtual DiagCollector& operator<<(uint32_t x);
  virtual DiagCollector& operator<<(uint64_t x);
  virtual DiagCollector& operator<<(char x);
  virtual DiagCollector& operator<<(const char* x);

  template <size_t n, typename T>
  DiagCollector& operator<<(const Code<n, T>& x)
  {
    if (UNLIKELY(_active))
      *this << x.c_str();
    return *this;
  }

  virtual DiagCollector& operator<<(const Header) { return *this; }
  virtual DiagCollector& operator<<(const std::string& x);
  virtual DiagCollector& operator<<(const boost::container::string& x);

  virtual DiagCollector& operator<<(std::ostream& (*pf)(std::ostream&));
  virtual DiagCollector& operator<<(std::ios_base& (*pf)(std::ios_base&));

  virtual DiagCollector& operator<<(const CarrierMixedClass& x) { return *this; }
  virtual DiagCollector& operator<<(const PaxTypeFare::SegmentStatus& x) { return *this; }
  virtual DiagCollector& operator<<(const Agent& x) { return *this; }
  virtual DiagCollector& operator<<(const Airport& x) { return *this; }
  virtual DiagCollector& operator<<(const AirSeg& x) { return *this; }
  virtual DiagCollector& operator<<(const BaseFareRule& x) { return *this; }
  virtual DiagCollector& operator<<(const BookingCodeExceptionSequence& x) { return *this; }
  virtual DiagCollector& operator<<(const CarrierPreference& x) { return *this; }
  virtual DiagCollector& operator<<(const Fare& x);
  virtual DiagCollector& operator<<(const FarePath& x);
  virtual DiagCollector& operator<<(const FareMarket& x);
  virtual DiagCollector& operator<<(const MergedFareMarket& x);
  virtual DiagCollector& operator<<(const ItinPUPath& x);
  virtual DiagCollector& operator<<(const FareUsage& x);
  virtual DiagCollector& operator<<(const Itin& x);
  virtual DiagCollector& operator<<(const PaxType& x) { return *this; }
  virtual DiagCollector& operator<<(const PaxTypeFare& x);
  virtual DiagCollector& operator<<(const PaxTypeFare::BookingCodeStatus& x) { return *this; }
  virtual DiagCollector& operator<<(const PaxTypeFareRuleData& x) { return *this; }
  virtual DiagCollector& operator<<(PricingUnit::Type x);
  virtual DiagCollector& operator<<(const PricingUnit& x);
  virtual DiagCollector& operator<<(const SurfaceSeg& x) { return *this; }
  virtual DiagCollector& operator<<(TaxOut& x) { return *this; }
  virtual DiagCollector& operator<<(const TaxCodeReg& x);
  virtual DiagCollector& operator<<(const TaxResponse& x);
  virtual DiagCollector& operator<<(const PfcItem& x);
  virtual DiagCollector& operator<<(const TaxRecord& x);
  virtual DiagCollector& operator<<(const TaxItem& x);
  virtual DiagCollector& operator<<(const TravelSeg& x) { return *this; }
  virtual DiagCollector& operator<<(const TravelSeg* x) { return *this; }
  virtual DiagCollector& operator<<(const PricingTrx& x) { return *this; }
  virtual DiagCollector& operator<<(const FareByRuleApp& x) { return *this; }
  virtual DiagCollector& operator<<(const Money& x);
  virtual DiagCollector& operator<<(const CategoryRuleInfo&) { return *this; }
  virtual DiagCollector& operator<<(const CategoryRuleItemInfo&) { return *this; }
  virtual DiagCollector& operator<<(const NegFareRest&) { return *this; }
  virtual DiagCollector& operator<<(const NegFareSecurityInfo&) { return *this; }
  virtual DiagCollector& operator<<(const NegFareCalcInfo&) { return *this; }
  virtual DiagCollector& operator<<(const MarkupCalculate&) { return *this; }
  virtual DiagCollector& operator<<(const ShoppingTrx& x) { return *this; }
  virtual DiagCollector& operator<<(const ItinIndex& x) { return *this; }
  virtual DiagCollector& operator<<(const ShoppingTrx::FlightMatrix& x) { return *this; }
  virtual DiagCollector& operator<<(const GroupFarePath& x);
  virtual DiagCollector& operator<<(const Record3ReturnTypes& x);
  virtual DiagCollector& operator<<(const CabinType& x);
  virtual DiagCollector& operator<<(const FareTypeDesignator& x);
  virtual DiagCollector& operator<<(const PenaltyInfo& x);
  virtual DiagCollector& operator<<(const VoluntaryChangesInfoW& x);
  virtual DiagCollector& operator<<(const VoluntaryChangesInfo& x);
  virtual DiagCollector& operator<<(const VoluntaryRefundsInfo& x);

  virtual DiagCollector& operator<<(const std::vector<BookingCode>& x) { return *this; }
  virtual DiagCollector& operator<<(const std::vector<FPPQItem*>& fppqItems);
  virtual DiagCollector& operator<<(const std::vector<FarePath*>& x);
  virtual DiagCollector& operator<<(const std::vector<PricingUnit*>& x);
  virtual DiagCollector& operator<<(const std::vector<TravelSeg*>& x);
  virtual DiagCollector& operator<<(const std::pair<PricingTrx*, PaxTypeFare*>& output) { return *this; }
  virtual DiagCollector& operator<<(const std::pair<PaxType*, PaxTypeFare*>& out) { return *this; }

  static bool parseFareMarket(PricingTrx&, const FareMarket&);
  static std::string printItinMIP(const Itin& x);
  static void displayTFDPSC(DiagCollector& dc,
                            const std::vector<const NegFareRestExtSeq*>& negFareRestExtSeq,
                            bool displayMatchedSeqs);

  void printHeader(std::string diagnosticName, char fillCharacter = '*', const int length = DISPLAY_MAX_SIZE);

  template<typename T>
  void printValue(const std::string& nameOfValue, const T& value)
  {
    ((std::ostringstream&)*this) << nameOfValue << ": " << value << std::endl;
  }

  template<typename T>
  void printValues(const std::string& nameOfValues, const T& container)
  {
    if (container.empty())
      return;

    auto& dc = static_cast<std::ostringstream&>(*this);
    dc << nameOfValues << ": ";

    bool first = true;
    for (const auto& value : container)
    {
      if (!first)
        dc << ", ";
      dc << value;
      first = false;
    }

    dc << std::endl;
  }

  void printValue(const std::string& nameOfValue, const bool value);

  void printLine(const std::string& string) { ((std::ostringstream&)*this) << string << std::endl; }

  virtual void printHeader();
  virtual void printLine();
  virtual void displayInfo() {}
  virtual void printBrand(const BrandCode& brand);
  virtual void printFareUsagesInfo(const PricingUnit& pu);
  virtual void lineSkip(int);
  virtual void printMessages(int, char = 0) {}
  virtual void displayHierarchy(char hierarchy) {}
  virtual bool finalDiag(PricingTrx&, const FareMarket&) { return true; }
  virtual void displayRelation(const CategoryRuleItemInfo* rule, Record3ReturnTypes statusRule);
  virtual void displayFltTktMerchInd(Indicator ind);
  virtual void printStopAtFirstMatchMsg() const;
  virtual bool enableFilter(DiagnosticTypes diagType, int dtCount, size_t currentFarePathNumber);
  virtual void displayRetrievalDate(const PaxTypeFare& x);
  virtual void displayFareDates(const PaxTypeFare& x);
  virtual void printIbfErrorMessage(IbfErrorMessage msg) {}
  virtual void initParam(Diagnostic& root) {}
  virtual void displayPuItins(const PricingUnit& pu) {}
  virtual void displayPtfItins(const PaxTypeFare& ptf) {}

  std::string cnvFlags(const PaxTypeFare& ptf);

  void displayFareMarketItinNums(const std::string& title,
                                 const PricingTrx& trx,
                                 const FareMarket& fareMarket);
  void displayFareMarketItinNumsMultiTkt(const std::string& title,
                                         const PricingTrx& trx,
                                         const FareMarket& fareMarket);

  void printPuType(PricingUnit::Type puType);

  bool isActive() const { return UNLIKELY(_active); }
  void activate() { _active = true; }
  void restoreState(bool priorState) { _active = priorState; }
  void deActivate() { _active = false; }

  void flushMsg();
  void clear();

  DiagnosticTypes& diagnosticType() { return _diagnosticType; }
  const DiagnosticTypes& diagnosticType() const { return _diagnosticType; }

  Diagnostic*& rootDiag() { return _rootDiag; }
  const Diagnostic* rootDiag() const { return _rootDiag; }

  Trx*& trx() { return _trx; }
  const Trx* trx() const { return _trx; }

  bool& useThreadDiag() { return _useThreadDiag; }
  const bool& useThreadDiag() const { return _useThreadDiag; }

  std::string getRelationString(uint32_t relationInd) const;

protected:
  uint32_t getCurrentMessageLength(std::string& outMsg);
  void setDiagWrapMode(DiagCollector::DiagWrapMode wrapMode);
  void setAlignmentMark();
  void setLineWrapAnchor();
  void setLineWrapLength(uint32_t lineWrapLength);
  bool wrapLine();
  virtual uint8_t getFareBasicWide() { return 13; }

  const std::string getStatusString(Record3ReturnTypes ruleStatus) const;
  void splitIntoLines(const std::string& text, std::vector<std::string>& lines,
                      uint32_t lineWidth = DEFAULT_LINEWRAP_LENGTH) const;
  void addMultilineInfo(std::ostream& stream, const std::string& info,
                        uint32_t lineWidth = DEFAULT_LINEWRAP_LENGTH) const;

  bool _active = false;
  boost::mutex _mutex;
  Diagnostic* _rootDiag = nullptr;
  Trx* _trx = nullptr;
  DiagnosticTypes _diagnosticType = DiagnosticTypes::DiagnosticNone;

private:

  template <typename T, typename R>
  using EnableIfNotDiag = std::enable_if<!std::is_convertible<T, DiagnosticTypes>::value, R>;

  void activate(std::initializer_list<DiagnosticTypes> diags, bool status)
  {
    _active = status && std::find(diags.begin(), diags.end(), _diagnosticType) != diags.end();
  }

  DiagWrapMode _wrapMode = DiagCollector::DiagWrapNone;
  uint32_t _alignmentMark = 0;
  uint32_t _lineWrapAnchor = 0;
  uint32_t _lineWrapLength = 0;
  bool _wrappedLine = false;
  bool _useThreadDiag = false;
  void performSimpleLineWrap();
  void performAlignedLineWrap();

public:

  template <typename... DiagType>
  bool enable(DiagType... diagTypes)
  {
    bool previousState = _active;
    if (_rootDiag && _rootDiag->isActive())
      activate({diagTypes...}, true);
    return previousState;
  }

  template <typename T, typename... DiagType>
  typename EnableIfNotDiag<T, bool>::type enable(T obj, DiagType... diagTypes)
  {
    bool previousState = _active;

    if (!_rootDiag || !_rootDiag->isActive())
      return previousState;

    _active = false;
    PricingTrx* trx = dynamic_cast<PricingTrx*>(_trx);

    if (UNLIKELY(trx && trx->getTrxType() == PricingTrx::MIP_TRX && trx->isFlexFare() && obj &&
        !DiagnosticUtil::shouldDisplayFlexFaresGroupInfo(trx->diagnostic(),
                                                         obj->getFlexFaresGroupId())))
    {
      return previousState;
    }
    activate({diagTypes...}, true);
    return previousState;
  }

  template <typename... DiagType>
  void disable(DiagType... diagTypes)
  {
    if (_rootDiag && _rootDiag->isActive())
      activate({diagTypes...}, false);
  }
};

} // namespace tse


