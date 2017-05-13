/*
 * ShoppingTaxUtil.h
 *
 *  Created on: Jul 25, 2013
 *      Author: SG0892420
 */

#pragma once
#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DataHandle.h"

namespace tse
{
class TravelSeg;
class Itin;
class TaxCodeReg;
class PricingTrx;
class TaxNation;
class TaxCodeReg;

class ShoppingTaxUtil
{
public:
  virtual ~ShoppingTaxUtil() {}

  static bool doesAYApply(const TravelSeg* ts, const Itin* itin, PricingTrx& trx);
  class DateSegmentation
  {
  public:
    DateSegmentation(const NationCode& nation, const DateTime& tktDate)
      : _nation(nation), _tktDate(tktDate)
    {
      initDateLimits();
    };
    void buildDateSegmentKey(const Itin& itin, std::string& key) const;

  private:
    void initDateLimits();

    NationCode _nation;
    DateTime _tktDate;
    std::vector<DateTime> _depDateLimits;
    std::vector<DateTime> _journeyCmtDateLimits;
  };
  class FlightRanges
  {
    friend class FlightRangesMock;

  public:
    FlightRanges(const NationCode& nation, const DateTime& tktDate)
      : _nation(nation), _dh(tktDate) {};
    virtual ~FlightRanges() {};
    void initFlightRanges();

    void buildFltRangeKey(const Itin& itin, std::string& key) const;

  private:
    virtual const TaxNation* getTaxNation();
    virtual const std::vector<TaxCodeReg*>& getTaxCode(const TaxCode& tc);

    typedef std::pair<FlightNumber, FlightNumber> FlightRange;
    typedef std::map<CarrierCode, std::vector<FlightRange> > FlightRangesByCarrier;
    FlightRangesByCarrier _flightRanges;
    NationCode _nation;
    DataHandle _dh;
  };

  virtual const TaxNation* getTaxNation(const NationCode nation, DataHandle& dh) const;
  virtual const std::vector<TaxCodeReg*>& getTaxCode(const TaxCode& tc, DataHandle& dh) const;

  bool getRestrWithNextStopover(const NationCode nation,
                                const DateTime& tktDate) const;

  void getNationTransitTimes(std::set<Hours>& transitHours,
                             std::set<Minutes>& transitTotalMinutes,
                             const NationCode nation,
                             const TaxCodesVec& transitMinutesRoundTaxes,
                             const DateTime& tktDate) const;

  void getTaxFirstTravelDates(std::set<boost::gregorian::date>& firstTravelDates,
                              const boost::gregorian::date firstDate,
                              const boost::gregorian::date lastDate,
                              const NationCode& nation,
                              const DateTime& tktDate) const;

private:
  class AYPrevalidator
  {
  public:
    AYPrevalidator(const Itin* itin, const TravelSeg* ts, PricingTrx& trx);
    bool doesAYApply();

  private:
    bool checkEnplanementLoc() const;
    bool checkPOS() const;
    bool checkValCxr() const;
    bool checkEqpmt() const;
    bool checkCxrFlt() const;

    const Itin* _itin;
    const TravelSeg* _ts;
    const TaxCodeReg* _taxRec;
    PricingTrx& _trx;
  };
};
}

