// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#pragma once

#include "DataModel/ItinIndex.h"
#include "DataModel/PaxTypeFare.h"

namespace tse
{
class FareMarket;
class FarePath;
class FareUsage;
class ShoppingTrx;
class SOPUsage;

class FarePathWrapperSource
{
public:
  FarePathWrapperSource(PricingTrx& trx, PaxTypeFare& ptf) : trx(trx), paxTypeFare(ptf) {}
  virtual ~FarePathWrapperSource() {}

  virtual FMDirection origDirection();

  virtual void fillWithItin(FarePath& farePath) = 0;
  virtual std::vector<PaxTypeFare::SegmentStatus> getSegmentStatus();
  virtual PaxType* getPaxType();

  virtual void rtUpdateValidatingCarrier(Itin& itin) = 0;

  bool isRoundTripEnabled() const { return _isRoundTripEnabled; }
  void removeMirrorFareUsage();

  PricingTrx& trx;
  PaxTypeFare& paxTypeFare;

protected:
  bool _isRoundTripEnabled = false;
};

class FarePathWrapper
{
public:
  struct FareMarketBackup
  {
    FareMarketBackup(FareMarket& fm);
    ~FareMarketBackup() { restoreFM(); }

    void setFareMarket(std::vector<TravelSeg*> segments, FMDirection dir);

  private:
    void restoreFM();

  private:
    FareMarket& _fareMarket;
    std::vector<TravelSeg*> _fareMarketTravelSegOrg;
    const FMDirection _fareMarketDirectionOrg;
  };

  FarePathWrapper(FarePathWrapperSource& source)
    : _source(source), _origDirection(_source.origDirection())
  {
  }

  void buildFarePath();
  FarePath* getFarePath() const { return _farePath; }
  void removeMirrorFareUsage() const;
  bool isRoundTripEnabled() const { return _source.isRoundTripEnabled(); }

private:
  void initializeFarePath();
  void populateFarePath();
  void completeRoundTrip() const;

  FareUsage& createMirrorFareUsage(FareUsage& fareUsage) const;

  FarePathWrapperSource& _source;
  const FMDirection _origDirection;
  FarePath* _farePath = nullptr;
};

class AltDatesFarePathWrapperSource : public FarePathWrapperSource
{
public:
  AltDatesFarePathWrapperSource(PricingTrx& trx,
                                PaxTypeFare& ptf,
                                const Itin& itin,
                                PaxType* paxType = nullptr);

  virtual void fillWithItin(FarePath& farePath) override;
  virtual PaxType* getPaxType() override;
  virtual void rtUpdateValidatingCarrier(Itin& itin) override;

  const Itin& itin;

private:
  PaxType* const _paxType;
};

class SoloFarePathWrapperSource : public FarePathWrapperSource
{
public:
  SoloFarePathWrapperSource(ShoppingTrx& trx,
                            PaxTypeFare& ptf,
                            SOPUsage& sopUsage,
                            uint32_t bitIndex);

  virtual void fillWithItin(FarePath& farePath) override;
  virtual std::vector<PaxTypeFare::SegmentStatus> getSegmentStatus() override;
  virtual void rtUpdateValidatingCarrier(Itin& itin) override;

  ShoppingTrx& shoppingTrx();
  SOPUsage& sopUsage;
  const uint32_t bitIndex;

private:
  FarePathWrapper::FareMarketBackup _fareMarketBackup;

  void setFareMarket();
};

class ShoppingFarePathWrapperSource : public FarePathWrapperSource
{
public:
  ShoppingFarePathWrapperSource(ShoppingTrx& trx,
                                PaxTypeFare& ptf,
                                PaxType* paxType,
                                uint32_t bitIndex);

  void initWithCell(const ItinIndex::ItinCell& cell);

  virtual void fillWithItin(FarePath& farePath) override;
  virtual std::vector<PaxTypeFare::SegmentStatus> getSegmentStatus() override;
  virtual PaxType* getPaxType() override;
  virtual void rtUpdateValidatingCarrier(Itin& itin) override;

  ShoppingTrx& shoppingTrx();
  const uint32_t bitIndex;

private:
  FarePathWrapper::FareMarketBackup _fareMarketBackup;

  PaxType* const _paxType;
  Itin* _itin = nullptr;

  void initItin();
};
}
