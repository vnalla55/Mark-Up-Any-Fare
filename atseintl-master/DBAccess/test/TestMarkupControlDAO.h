#ifndef TEST_MARKUP_CONTROL_DAO_H
#define TEST_MARKUP_CONTROL_DAO_H

#include "DBAccess/test/TestDataAccessObject.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/MarkupControl.h"
#include "DBAccess/CompressedData.h"
#include "DBAccess/CompressedDataUtils.h"

const int
MAXNUMBERMUCENTRIES(100);

namespace tse
{
typedef int Key;
class MarkupControl;

class TestMarkupControlDAO : public TestDataAccessObject<Key, std::vector<MarkupControl*> >
{
public:
  virtual sfc::CompressedData* compress(const std::vector<MarkupControl*>* vect) const
  {
    return compressVector(vect);
  }

  virtual std::vector<MarkupControl*>* uncompress(const sfc::CompressedData& compressed) const
  {
    return uncompressVectorPtr<MarkupControl>(compressed);
  }

  virtual std::vector<MarkupControl*>* create(const Key& key) override
  {
    size_t numitems(key % MAXNUMBERMUCENTRIES);
    std::vector<MarkupControl*>* ptr(new std::vector<MarkupControl*>(numitems));
    for (size_t i = 0; i < numitems; ++i)
    {
      MarkupControl* obj(new MarkupControl);
      markupControlDummyData(*obj);
      (*ptr)[i] = obj;
    }
    return ptr;
  }

  virtual CreateResult<std::vector<MarkupControl*>> create(const Key& key, bool) override
  {
    tse::CreateResult<std::vector<MarkupControl*>> result;
    result._ptr = create(key);
    return result;
  }

  void destroy(Key key, std::vector<MarkupControl*>* recs)
  {
    std::vector<MarkupControl*>::const_iterator i;
    for (i = recs->begin(); i != recs->end(); i++)
      delete *i;
    delete recs;
  }
  static void markupControlDummyData(MarkupControl& obj)
  {
    static const DateTime dummyDateTime(time(NULL));
    obj.vendor() = "ABCD";
    obj.carrier() = "EFG";
    obj.ruleTariff() = 1;
    obj.rule() = "HIJK";
    obj.seqNo() = 2;
    obj.creatorPseudoCity() = "LMNOP";
    obj.markupType() = 'Q';
    obj.ownerPseudoCityType() = 'R';
    obj.ownerPseudoCity() = "STUVW";
    obj.createDate() = dummyDateTime;
    obj.expireDate() = dummyDateTime;
    obj.requestDate() = dummyDateTime;
    obj.secondarySellerId() = 3;
    obj.accountCode() = "aaaaaaaa";
    obj.ownerId() = "bbbbbbbb";
    obj.redistributeTag() = 'X';
    obj.updateTag() = 'Y';
    obj.sellTag() = 'Z';
    obj.tktTag() = 'a';
    obj.viewNetInd() = 'b';
    obj.status() = 'c';

    MarkupCalculate* mc1 = new MarkupCalculate;
    MarkupCalculate* mc2 = new MarkupCalculate;

    markupCalculateDummyData(*mc1);
    markupCalculateDummyData(*mc2);

    obj.calcs().push_back(mc1);
    obj.calcs().push_back(mc2);
  }
  static void markupCalculateDummyData(MarkupCalculate& obj)
  {
    static const DateTime dummyDateTime(time(NULL));
    obj.orderNo() = 1;
    obj.tvlEffDate() = dummyDateTime;
    obj.tvlDiscDate() = dummyDateTime;
    obj.negFareCalcSeq() = 2;
    obj.directionality() = 'A';

    LocKey::dummyData(obj.loc1());
    LocKey::dummyData(obj.loc2());

    obj.bundledInd() = 'B';
    obj.sellingFareInd() = 'C';
    obj.sellingPercentNoDec() = 3;
    obj.sellingPercent() = 4.444;
    obj.netSellingInd() = 'D';
    obj.sellingNoDec1() = 5;
    obj.sellingFareAmt1() = 6.66;
    obj.sellingCur1() = "EFG";
    obj.sellingNoDec2() = 7;
    obj.sellingFareAmt2() = 8.88;
    obj.sellingCur2() = "HIJ";
    obj.percentMinNoDec() = 9;
    obj.percentMin() = 10.101;
    obj.percentMaxNoDec() = 11;
    obj.percentMax() = 12.121;
    obj.markupFareInd() = 'K';
    obj.markupNoDec1() = 13;
    obj.markupMinAmt1() = 14.14;
    obj.markupMaxAmt1() = 15.15;
    obj.markupCur1() = "LMN";
    obj.markupNoDec2() = 16;
    obj.markupMinAmt2() = 17.17;
    obj.markupMaxAmt2() = 18.18;
    obj.markupCur2() = "OPQ";
    obj.fareClass() = "RSTUVWXY";
    obj.fareType() = "aaa";
    obj.seasonType() = 'Z';
    obj.dowType() = 'a';
    obj.wholesalerNoDec1() = 19;
    obj.wholesalerFareAmt1() = 20.20;
    obj.wholesalerCur1() = "bcd";
    obj.wholesalerNoDec2() = 21;
    obj.wholesalerFareAmt2() = 22.22;
    obj.wholesalerCur2() = "efg";
    obj.wholesalerFareInd() = 'h';
    obj.wholesalerPercentNoDec() = 23;
    obj.wholesalerPercent() = 24.242;
    obj.psgType() = "ijk";
    obj.noSellInd() = 'l';
  }
};
} // namespace tse
#endif // TEST_MARKUP_CONTROL_DAO_H
