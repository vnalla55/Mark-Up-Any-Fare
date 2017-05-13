#ifndef TAX_LOC_ITERATOR_MOCK_H
#define TAX_LOC_ITERATOR_MOCK_H

#include "Taxes/LegacyTaxes/TaxLocIterator.h"

namespace tse
{

class TaxLocItMockItem
{
public:
  uint16_t nextSegNo;
  uint16_t nextSubSegNo;
  bool stop;
  bool fareBreak;
  bool turnAround;
  bool isLoc1;
  bool isLoc2;
  bool airSeg;
  Loc* loc;
  bool isMirror;
  TravelSeg* travelSeg;

  TaxLocItMockItem()
    : nextSegNo(0),
      nextSubSegNo(0),
      stop(false),
      fareBreak(false),
      turnAround(false),
      isLoc1(false),
      isLoc2(false),
      airSeg(true),
      loc(NULL),
      isMirror(false),
      travelSeg(0)
  {
  }
};

class TaxLocIteratorMock : public TaxLocIterator
{
public:
  std::vector<TaxLocItMockItem> _items;
  uint16_t _itemNo;

  void addLoc(TaxLocItMockItem item) { _items.push_back(item); }

  void printLocs()
  {
    std::vector<TaxLocItMockItem>::iterator tli;
    for (tli = _items.begin(); tli != _items.end(); ++tli)
    {
      std::cout << std::endl << " seg:" << tli->nextSegNo << " subseg:" << tli->nextSubSegNo
                << " stop:" << tli->stop << " fb:" << tli->fareBreak << " turn:" << tli->turnAround
                << " isLoc1:" << tli->isLoc1 << " isLoc2:" << tli->isLoc2 << " :";
    }
  }

  virtual void initialize(FarePath&) {}

  virtual bool hasNext() const { return (_itemNo + 1u < _items.size()); }

  virtual bool hasPrevious() const { return (_itemNo > 0); }

  virtual bool isEnd() const { return !(_itemNo < _items.size()); }

  virtual void next() { ++_itemNo; }

  virtual void previous() { --_itemNo; }

  virtual void toBack() { _itemNo = _items.size() - 1; }

  virtual void toFront() { _itemNo = 0; }

  virtual void toSegmentNo(uint16_t s)
  {
    std::vector<TaxLocItMockItem>::iterator tli;
    _itemNo = 0;
    for (tli = _items.begin(); tli != _items.end() && _itemNo != s; ++tli, ++_itemNo)
      ;
  }

  virtual uint16_t prevSegNo() const { return _items[_itemNo - 1].nextSegNo; }

  virtual uint16_t nextSegNo() const { return _items[_itemNo].nextSegNo; }

  virtual TravelSeg* prevSeg() const { return _items[_itemNo - 1].travelSeg; }

  virtual TravelSeg* nextSeg() const { return _items[_itemNo].travelSeg; }

  virtual uint16_t subSegNo() const { return _items[_itemNo].nextSubSegNo; }

  virtual bool isStop() { return _items[_itemNo].stop; }

  virtual bool isFareBreak() { return _items[_itemNo].fareBreak; }

  virtual bool isTurnAround() { return _items[_itemNo].turnAround; }

  virtual bool isNextSegAirSeg() const { return _items[_itemNo].airSeg; }

  virtual bool isPrevSegAirSeg() const { return _items[_itemNo - 1].airSeg; }

  virtual const Loc* loc() const { return _items[_itemNo].loc; }

  virtual bool isInLoc1(TaxCodeReg& tcr, PricingTrx& trx) const { return _items[_itemNo].isLoc1; }

  virtual bool isInLoc2(TaxCodeReg& tcr, PricingTrx& trx) const { return _items[_itemNo].isLoc2; }

  virtual bool isHidden() { return _items[_itemNo].nextSubSegNo; }

  virtual bool isMirror() { return _items[_itemNo].isMirror; }
};
}

#endif
