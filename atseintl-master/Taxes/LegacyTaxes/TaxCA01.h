//---------------------------------------------------------------------------
//  Copyright Sabre 2008
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
#ifndef TAX_CA_01_H
#define TAX_CA_01_H

#include "Common/TseCodeTypes.h"
#include "DBAccess/Loc.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"

namespace tse
{
class Itin;
class PricingTrx;
class TaxResponse;
class TaxCodeReg;

//---------------------------------------------------------------------------
// Tax special process 25 Canada Air Security Charge - Subject to GST
//---------------------------------------------------------------------------

class TaxCA01 : public Tax
{

public:
  TaxCA01() : _soldInCanada(false), _isItineraryAnalyzed(false) {}
  virtual ~TaxCA01();

  bool _soldInCanada;
  bool _isItineraryAnalyzed;

  enum LocCategory
  {
    CANADA,
    US,
    OTHER
  };

  enum ItinType
  {
    DOMESTIC,
    TRANSBORDER,
    INTERNATIONAL,
    INBOUND_INTERNATIONAL
  };

  struct ItinAnalisis
  {

    ItinAnalisis()
      : journeyOriginIndex(0),
        loopTrip(false),
        stopOver(false),
        chargeCount(0),
        allCanadian(true),
        caStop(true)
    {
    }

    uint16_t journeyOriginIndex;
    bool loopTrip;
    bool stopOver;
    uint16_t chargeCount;
    bool allCanadian;
    bool caStop;
  };

  class ItinAnalyzer
  {
  protected:
    enum ItinType itinType;
    std::set<Loc*> enplanementsSet;
    std::vector<struct ItinAnalisis> itinAnalisisVector;
    TravelSeg* travelSegPrev;
    uint16_t index;
    uint16_t journeyOriginIndex;
    GeoTravelType geoType;
    bool _caStop;
    const Loc* _fromAbroadLoc;
    bool _intrntlCharged;
    bool _isStartEndIntrntl;

    bool isStopOver(TravelSeg* current, TravelSeg* previous);

  public:
    bool correctFlag;

    ItinAnalyzer()
      : itinType(DOMESTIC),
        travelSegPrev(nullptr),
        index(0),
        journeyOriginIndex(0),
        geoType(GeoTravelType::Domestic),
        _caStop(true),
        _fromAbroadLoc(nullptr),
        _intrntlCharged(false),
        _isStartEndIntrntl(false),
        correctFlag(false)
    {
    }

    void typeAnalyze(TravelSeg* travelSeg);
    void journeyAnalyze(PricingTrx& trx, TravelSeg* travelSeg);
    void startEndAnalyze(const TravelSeg& segStart, const TravelSeg& segEnd);
    enum ItinType getItinType() { return itinType; }
    bool charge(uint16_t segIndex, bool bInterntl, const PricingTrx& trx);
    uint16_t getOriginIndex(uint16_t segIndex)const {return itinAnalisisVector[segIndex].journeyOriginIndex; };
    bool getCaStop(uint16_t segIndex)const {return itinAnalisisVector[segIndex].caStop; };
    bool isStartEndIntrntl()const {return _isStartEndIntrntl; };
  } itinAnalyzer;

  static enum LocCategory checkLocCategory(const Loc& loc);
  static bool soldInCanada(PricingTrx& trx);
  bool doesXGApply(const Itin& itin);

  bool validateLocRestrictions(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t& startIndex,
                               uint16_t& endIndex) override;

  bool validateTripTypes(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex) override;

  bool validateCarrierExemption(PricingTrx& trx,
                                TaxResponse& taxResponse,
                                TaxCodeReg& taxCodeReg,
                                uint16_t travelSegIndex) override;

  //---------------------------------------------------------------------------
  // Override of Tax Method to ignore General Processing - adjustment exception
  // on tax-on-tax for CA taxes
  //---------------------------------------------------------------------------

  void adjustTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override;

private:
  enum TaxDiagnostic::FailCodes
  validateSegment(PricingTrx& trx, Itin& itin, TaxCodeReg& taxCodeReg, uint16_t& startIndex, uint16_t& endIndex);

  static const std::string TAX_CODE_CA1;
  static const std::string TAX_CODE_CA2;
  static const std::string TAX_CODE_CA3;
  static const std::string TAXCA1SPR132366;

  static constexpr char TAX_EXCLUDE = 'Y';
  static constexpr char TAX_ORIGIN = 'O';
  static constexpr char TAX_DESTINATION = 'D';
  static constexpr char TAX_ENPLANEMENT = 'E';
  static constexpr char TAX_DEPLANEMENT = 'X';
  static constexpr char TAX_TERMINATION = 'T';
  static constexpr char TAX_FROM_TO = 'F';
  static constexpr char TAX_BETWEEN = 'B';
  static constexpr char TAX_WITHIN_SPEC = 'S';
  static constexpr char TAX_WITHIN_WHOLLY = 'W';

  TaxCA01(const TaxCA01& tax);
  TaxCA01& operator=(const TaxCA01& tax);
};

} /* end tse namespace */

#endif /* TAX_CA_01_H */
