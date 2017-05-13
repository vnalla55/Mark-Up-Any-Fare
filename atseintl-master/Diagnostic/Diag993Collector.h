//----------------------------------------------------------------------------
//  File:        Diag993Collector.h
//
//
//  Copyright Sabre 2010
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

#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FlexFares/Types.h"
#include "DataModel/TNBrandsTypes.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class Itin;
class ShoppingTrx;

class Diag993Collector : public DiagCollector
{
public:
  using DiagCollector::operator<<;

  explicit Diag993Collector(Diagnostic& root)
    : DiagCollector(root), _application(false), _globalDirection(GlobalDirection::ZZ), _groupId(0)
  {
  }

  Diag993Collector() : _application(false), _globalDirection(GlobalDirection::ZZ), _groupId(0) {}

  void printNewItin(const Itin& itin);

  void writeBrandInfo(uint16_t itinIndex,
                      uint16_t brandIndex);
  void writeBrandCombinationInfo(const std::string& brands);

  void printMergerHeader();
  void printMergerFooter();
  void displayInputFareMarkets(const std::vector<FareMarket*>& allGovCxrFM);
  void displaySoldoutStatus(IbfErrorMessage status);
  void displayCbsX();
  void displayValidFareFound(const FareClassCode& fareClass, const BrandCode& brandCode);
  void displayNoValidFaresForBrand(const BrandCode& brandCode);
  void displayCbsNotAllowedForLeg();

  void writeUseAnyBrand(const MergedFareMarket& mfm, const BrandCode& origBrand);
  Diag993Collector& operator<<(const PaxTypeFare& paxFare) override;
  Diag993Collector& operator<<(const FareMarket& fareMarket) override;

  Diag993Collector& operator<<(const MergedFareMarket& mfm) override;
  void printMFM(const MergedFareMarket& mergedFareMarket);
  void setFlexFaresGroupId(flexFares::GroupId groupId) { _groupId = groupId; }
  void displayFailedFlexFaresHeader(const FareMarket& fm, flexFares::GroupId groupId);
  void displayFailedFlexFaresPTF(const PaxTypeFare* ptf);
  void displayPassedFlexFareCount(const FareMarket& fm);

  void printAllFareMarkets(ShoppingTrx& trx);

  void printFareMarket(const FareMarket& mergedFareMarket,
                       flexFares::GroupId groupId,
                       const skipper::BrandingOptionSpace& brandingOptionSpace,
                       const skipper::ItinBranding& itinBranding);

  void printMergedFareMarket(const MergedFareMarket& mergedFareMarket,
                             flexFares::GroupId groupId,
                             const skipper::BrandingOptionSpace& brandingOptionSpace,
                             const skipper::ItinBranding& itinBranding);

  void printMergedFareMarketVect(const std::vector<MergedFareMarket*>& mergedFareMarketVect,
                                 flexFares::GroupId groupId,
                                 const skipper::BrandingOptionSpace& brandingOptionSpace,
                                 const skipper::ItinBranding& itinBranding);

private:
  enum SeparatorType
  {
    FARE_HEADER = 1,
    DIVIDER,
    SURCHARGES_HEADER
  };

  bool _application;
  GlobalDirection _globalDirection;
  flexFares::GroupId _groupId;

  void writeSeparator(SeparatorType);
  void writePtfPrefix(const PaxTypeFare& paxFare);
  void writePtfAllTaxes(const FareMarket& fareMarket,
                        const PaxTypeFare& paxFare,
                        const PrecalculatedTaxes& taxes);

  void printPaxTypeBucket(const FareMarket& fareMarket, const PaxTypeBucket& cortege);
  void printPrecalculatedTaxes(const FareMarket& fareMarket,
                               const CxrPrecalculatedTaxes& cxrTaxes);
  bool canPrintPrecalculatedTaxes() const;
  bool canPrintFares() const;

  PricingTrx& pricingTrx();
  const PricingTrx& pricingTrx() const;
};

} // namespace tse

