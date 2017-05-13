#pragma once

#include "Common/CurrencyConversionCache.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/Money.h"
#include "Common/DateTime.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <map>

namespace tse
{
class Fare;
class FareClassAppInfo;
class FareClassAppSegInfo;
class FareDisplayTrx;
class FareInfo;
class FareTypeMatrix;
class FareMarket;
class Itin;
class Loc;
class PaxTypeFare;
class PricingTrx;

class Record1Resolver
{
public:
  Record1Resolver(PricingTrx& trx, const Itin& itin, FareMarket& fareMarket);
  virtual ~Record1Resolver() = default;

  bool resolveR1s(const Fare& fare, std::vector<PaxTypeFare*>& resultingPTFs);

protected:
  /* a Money subclass that handles calculations needed for Cat19, 25 and 35
    * 'this' is the Money object for the fare in the published currency
    * To minimize impacts on PaxTypeFare, this object is intended as temporary
    * storage area for calulations.  Methods to xfer data to/from PTF are provided
    *
    * TODO remove _nuc???
    */
   class CalcMoney : public Money
   {
   public:
     PricingTrx& _trx;
     Money _itinMoney;
     Money _rexSecondRoeItinMoney;
     CurrencyConversionFacade& _ccf;
     bool _isIntl;
     bool _isRT = false;
     bool _isNucSelectAllowed = true; /* true => fail if no currency match */
     CurrencyConversionCache _cache;
     bool _excCurrOverrideNotNuc;
     bool _applyNonIATARounding = false;

   public:
     CalcMoney(PricingTrx& trx, CurrencyConversionFacade& ccf, const Itin& itin);
     static const MoneyAmount NO_AMT;
     void configForCat25();
     // TODO resolve NUC vs calcCur reqs & naming
     MoneyAmount& nucValue()
     {
       return _itinMoney.value();
     };

     void getFromPTF(PaxTypeFare& paxTypeFare, bool doNotChkNonIATARounding = false);
     void putIntoPTF(PaxTypeFare& paxTypeFare, FareInfo& fi);
     void adjustPTF(PaxTypeFare& paxTypeFare);
     void setCurrency(const CurrencyCode& newCur);
     void setRT(bool newRT);
     MoneyAmount fareAmount();
     void setFareAmount(MoneyAmount fareAmt);
     bool isIntl()
     {
       return _isIntl;
     };

     void doPercent(const MoneyAmount percent);
     /* ret =0 fail, =1 used cur1, =2 used cur2 */
     int doAdd(const MoneyAmount amt1,
               const CurrencyCode cur1,
               const MoneyAmount amt2,
               const CurrencyCode cur2);
     int doMinus(const MoneyAmount amt1,
                 const CurrencyCode cur1,
                 const MoneyAmount amt2,
                 const CurrencyCode cur2);
     int getFromSpec(const MoneyAmount amt1,
                     const CurrencyCode cur1,
                     const MoneyAmount amt2,
                     const CurrencyCode cur2);
     CalcMoney& operator=(const CalcMoney& rhs);

     void setupConverionDateForHistorical();

   private:
     void calcFareAmount();
     void adjust(bool isOutbound);
     int pickAmt(Money& native,
                 Money& nuc,
                 const MoneyAmount amt1,
                 const CurrencyCode cur1,
                 const MoneyAmount amt2,
                 const CurrencyCode cur2);
   };

   static constexpr Indicator UNAVAILABLE = 'X';

  PricingTrx& _trx;
  FareDisplayTrx* _fdTrx = nullptr;
  FareMarket& _fareMarket;
  DateTime _travelDate;

  CurrencyConversionFacade _ccFacade;
  CalcMoney _calcMoney;

  mutable std::map<LocCode, const Loc*> _locCache;
  mutable std::map<FareType, const FareTypeMatrix*> _fareTypeMatrixCache;

  bool resolveFareClassApp(const Fare& fare, std::vector<PaxTypeFare*>& ptFares);
  bool resolveFareClassAppByTravelDT(const Fare& fare, std::vector<PaxTypeFare*>& ptFares);
  bool resolveFareClassAppSeg(const Fare& fare, std::vector<PaxTypeFare*>& ptFares,
                              bool flipGeo, const FareClassAppInfo& fcaInfo);
  bool resolveFareClassAppESV(const Fare& fare, PaxTypeFare*& ptFare);

  PaxTypeFare* createPaxTypeFare(Fare* fare, FareClassAppInfo* fcai, FareClassAppSegInfo* fcasi);
  bool buildFareClassAppInfoBF(const Fare& fare, PaxTypeFare*& ptFare);
  FareClassAppInfo* createFareClassAppInfoBF(const Fare& fare) const;
  FareClassAppSegInfo* createFareClassAppSegInfoBF(const Fare& fare) const;

  bool commandPricingFare(const FareClassCode& fbc) const;
  bool isFdTrx() const { return (_fdTrx != nullptr); }
  bool matchLocation(const Fare& fare, bool& flipGeo, const FareClassAppInfo& fcaInfo) const;
  static bool isForBkgCdsOverFlown(const FareClassAppSegInfo* prevFcas,
                                   const FareClassAppSegInfo* thisFcas);

  const Loc* getLoc(const LocCode& code) const;
  const FareTypeMatrix* getFareTypeMatrix(const FareType& type) const;

};

}
