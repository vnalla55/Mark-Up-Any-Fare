
#pragma once

#include "Common/TseCodeTypes.h"
#include "DataModel/MileageTypeData.h"
#include "FareCalc/FareAmountAdjustment.h"
#include "FareCalc/FcDispItem.h"
#include "FareCalc/FcStream.h"

#include <iosfwd>
#include <set>

namespace tse
{

class FareCalcConfig;
class FareCalcCollector;
class CalcTotals;
class FarePath;
class FareUsage;
class TravelSeg;
class Loc;
class FareBreakPointInfo;
class NetRemitFarePath;

class FcDispFareUsage : public FcDispItem
{
  friend class FcDispFareUsageTest;

public:
  FcDispFareUsage(PricingTrx& trx,
                  const FarePath& fp,
                  const CalcTotals* calcTotals,
                  const FareCalcConfig& fcConfig,
                  FareCalcCollector& fcCollector,
                  FareCalc::FcStream& os,
                  const FareCalc::FareAmountAdjustment& fareAmountAdjustment,
                  bool useNetPubFbc = false);

  std::string toString() const override { return _os.str(); }

  void operator()(const FareUsage* fu);

  void displayTravelCommenceDate();

  void displayFareUsage(const FareUsage* fu);

  bool displayOrigAirport(const TravelSeg* ts,
                          const LocCode& orig,
                          bool firstCity,
                          bool fareBreak,
                          bool cnxWithCityGroup,
                          FareCalc::Group* fcGroup);

  bool displayDestAirport(const FareUsage* fu,
                          const TravelSeg* ts,
                          const LocCode& orig,
                          bool fareBreak,
                          bool lastSeg,
                          const CalcTotals* calcTotals,
                          bool cnxWithCityGroup,
                          FareCalc::Group* fcGroup);

  bool displayCarrier(const FareUsage* fu,
                      const TravelSeg* ts,
                      bool fareBreak,
                      bool cnxWithCityGroup,
                      FareCalc::Group* fcGroup);

  bool displaySideTrip(const FareUsage* fu, const TravelSeg* ts);

  void displaySideTripMarker();

  void displayEndSideTripMarker();

  void displayGlobalDirection(const FareUsage* fu, const TravelSeg* ts);

  bool noDisplayGlobalDirection(const FareUsage* fu,
                                const TravelSeg* ts,
                                const std::string& tempGlobalDir);

  bool displayHipPlusUp(const FareUsage* fu, bool cnxWithCityGroup, FareCalc::Group* fcGroup);

  bool displayCat12Surcharge(const TravelSeg* ts, bool cnxWithCityGroup, FareCalc::Group* fcGroup);
  bool displayCat12Surcharge(const std::vector<TravelSeg*>& ts,
                             bool cnxWithCityGroup,
                             FareCalc::Group* fcGroup);

  bool displayNonBreakOffMileageSurcharge(const FareUsage* fu,
                                          const TravelSeg* ts,
                                          bool cnxWithCityGroup,
                                          FareCalc::Group* fcGroup,
                                          const LocCode& loc);

  bool displayBreakOffMileageSurcharge(const FareUsage* fu,
                                       const TravelSeg* ts,
                                       bool cnxWithCityGroup,
                                       FareCalc::Group* fcGroup);

  bool displaySingleStopOverSurcharge(const TravelSeg* ts,
                                      bool startHIPsGroup,
                                      FareCalc::Group* fcGroup);

  bool displaySingleTransferSurcharge(const TravelSeg* ts,
                                      bool startHIPsGroup,
                                      FareCalc::Group* fcGroup);

  bool displayFareAmount(const FareUsage* fu,
                         bool cnxWithCityGroup,
                         FareCalc::Group* fcGroup,
                         const TravelSeg* tvlSegs);

  static LocCode getDisplayLoc(const FareCalcConfig& fcConfig,
                               const GeoTravelType& geoTravelType,
                               const LocCode& multiCity,
                               const LocCode& airport,
                               const CarrierCode& carrier,
                               const DateTime& date,
                               bool alwaysCheckMultiTable = false);

  static bool
  getFccDisplayLoc(const FareCalcConfig& fcConfig, const LocCode& loc, LocCode& displayLoc);
  bool isMultiGlobalDir(const LocCode& origin, const LocCode& dest, const DateTime& date);

  void processSideTrip(const FareUsage* fu,
                       const TravelSeg* ts,
                       bool& sideTripProcessed,
                       bool& cnxWithCityGroup,
                       FareCalc::Group& fcGroup);

  bool isNotArunkSegBeforeSideTrip(const FareUsage* fu, const TravelSeg* nSeg);

private:
  const FarePath& _fp;
  FareCalc::FcStream& _os;
  const CalcTotals* _calcTotals;
  bool _matchCurrencyCode;
  int _noDec;
  int _surchargeCount;

  const FareUsage* _prevFareUsage;
  LocCode _prevDest;

  bool _inSideTrip;

  const FareCalc::FareAmountAdjustment& _fareAmountAdjustment;

  const CalcTotals* _originalCalcTotals;

  bool _prevDestConnection;
  bool _useNetPubFbc;

  void
  saveMileageTypeData(const TravelSeg* ts, MileageTypeData::MileageType mtype, const LocCode& loc);

  void excMileageEqulalizationDisplay(const TravelSeg* ts, LocCode& destination);

  void displaySurfaceSegment(const LocCode& orig,
                             bool fareBreak,
                             bool cnxWithCityGroup,
                             FareCalc::Group* fcGroup);

  void restartFcGroup(FareCalc::Group* fcGroup);

  const std::vector<TravelSeg*>&
  getTravelSeg(const FareUsage* netOriginalFu, const FareUsage* fu) const;
  void setFbcFromTFDPSC(const std::string& fbc);
  void setFareBasis(const FareUsage* fu,
                    const FareBreakPointInfo& fbpInfo,
                    const TravelSeg* ts,
                    std::string& fareBasis,
                    const NetRemitFarePath* netRemitfp) const;

  class StartSideTrip
  {
  public:
    StartSideTrip(FcDispFareUsage* dispFareUsage) : _dispFpInfo(dispFareUsage)
    {
      _dispFpInfo->_inSideTrip = true;
      _dispFpInfo->displaySideTripMarker();
    }
    ~StartSideTrip()
    {
      _dispFpInfo->displayEndSideTripMarker();
      _dispFpInfo->_inSideTrip = false;
    }

  private:
    FcDispFareUsage* _dispFpInfo;
  };
};

} // namespace tse

