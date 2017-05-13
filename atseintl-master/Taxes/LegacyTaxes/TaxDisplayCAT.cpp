// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/TaxDisplayCAT.h"
#include "Taxes/LegacyTaxes/TaxDisplayList.h"
#include "DataModel/Trx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayCAT.h"
#include "DBAccess/TaxNation.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/Category1.h"
#include "Taxes/LegacyTaxes/Category2.h"
#include "Taxes/LegacyTaxes/Category3.h"
#include "Taxes/LegacyTaxes/Category4.h"
#include "Taxes/LegacyTaxes/Category5.h"
#include "Taxes/LegacyTaxes/Category6.h"
#include "Taxes/LegacyTaxes/Category7.h"
#include "Taxes/LegacyTaxes/Category8.h"
#include "Taxes/LegacyTaxes/Category9.h"
#include "Taxes/LegacyTaxes/Category10.h"
#include "Taxes/LegacyTaxes/Category11.h"
#include "Taxes/LegacyTaxes/Category12.h"
#include "Taxes/LegacyTaxes/Category13.h"
#include "Taxes/LegacyTaxes/Category14.h"
#include "Taxes/LegacyTaxes/Category15.h"
#include "Taxes/LegacyTaxes/Category16.h"
#include "Taxes/LegacyTaxes/Category17.h"
#include "Taxes/LegacyTaxes/Category18.h"
#include "Taxes/LegacyTaxes/Reissue.h"
#include "Taxes/LegacyTaxes/CategorySeqDataVerbiage.h"
#include "DBAccess/DataHandle.h"
#include "DataModel/Itin.h"

using namespace tse;
using namespace std;

log4cxx::LoggerPtr
TaxDisplayCAT::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.TaxDisplayCAT"));

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxDisplayCAT::TaxDisplayCAT() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxDisplayCAT::~TaxDisplayCAT() {}

// ----------------------------------------------------------------------------
// Description:  Tax Display CAT
// ----------------------------------------------------------------------------

bool
TaxDisplayCAT::buildCats(TaxTrx& taxTrx)
{
  uint16_t maxCategories = 18;
  TaxResponse* taxResponse = taxTrx.itin().front()->getTaxResponses().front();
  //--------------------------------------------------------------------------------------------
  // build differential
  if ((taxTrx.getRequest()->sequenceNumber() != TaxRequest::EMPTY_SEQUENCE) &&
      (taxTrx.getRequest()->sequenceNumber2() != TaxRequest::EMPTY_SEQUENCE))
  {
    std::vector<TaxDisplayItem*>::iterator taxDii1 = taxResponse->taxDisplayItemVector().begin();
    std::vector<TaxDisplayItem*>::iterator taxDii2 = taxDii1;

    if (taxResponse->taxDisplayItemVector().front() != taxResponse->taxDisplayItemVector().back())
      taxDii2++;

    for (uint16_t i = 1; i <= maxCategories; i++)
    {
      buildSequenceInfo(taxTrx, i, *taxDii1);
      buildSequenceInfo(taxTrx, i, *taxDii2);
    }

    buildCatDiff(*taxDii1, *taxDii2);
    return true;
  }
  //------------------------------------------------------------------------------------------------

  if (taxTrx.getRequest()->getReissue())
  {
    std::vector<TaxDisplayItem*>::iterator taxDisplayItemIter =
        taxResponse->taxDisplayItemVector().begin();
    std::vector<TaxDisplayItem*>::iterator taxDisplayItemEndIter =
        taxResponse->taxDisplayItemVector().end();

    for (; taxDisplayItemIter != taxDisplayItemEndIter; taxDisplayItemIter++)
    {
      buildReissue(taxTrx, **taxDisplayItemIter);
      buildCat1(taxTrx, **taxDisplayItemIter);
    }
    return true;
  }

  if (taxResponse->taxDisplayItemVector().front() != taxResponse->taxDisplayItemVector().back())
  {
    std::vector<TaxDisplayItem*>::iterator taxDisplayItemIter =
        taxResponse->taxDisplayItemVector().begin();
    std::vector<TaxDisplayItem*>::iterator taxDisplayItemEndIter =
        taxResponse->taxDisplayItemVector().end();

    for (; taxDisplayItemIter != taxDisplayItemEndIter; taxDisplayItemIter++)
    {
      buildCat1(taxTrx, **taxDisplayItemIter, false);
      buildDateVerbiage(taxTrx, **taxDisplayItemIter);

      if (taxTrx.getRequest()->menu())
      {
        if (taxTrx.getRequest()->categoryVec().empty())
        {
          for (uint32_t i = 1; i <= maxCategories; i++)
            taxTrx.getRequest()->categoryVec().push_back(i);
        }

        TaxDisplayItem* taxDisplayItem = *taxDisplayItemIter;
        std::vector<uint32_t>::const_iterator categoryVecIter =
            taxTrx.getRequest()->categoryVec().begin();
        std::vector<uint32_t>::const_iterator categoryVecEndIter =
            taxTrx.getRequest()->categoryVec().end();

        for (; categoryVecIter != categoryVecEndIter; categoryVecIter++)
        {
          buildSequenceInfo(taxTrx, *categoryVecIter, taxDisplayItem);
        }
      }
      else if (!taxTrx.getRequest()->menu() &&
               taxTrx.getRequest()->sequenceNumber() == TaxRequest::EMPTY_SEQUENCE &&
               !taxTrx.getRequest()->categoryVec().empty())
      {

        TaxDisplayItem* taxDisplayItem = *taxDisplayItemIter;
        std::vector<uint32_t>::const_iterator categoryVecIter =
            taxTrx.getRequest()->categoryVec().begin();
        std::vector<uint32_t>::const_iterator categoryVecEndIter =
            taxTrx.getRequest()->categoryVec().end();

        for (; categoryVecIter != categoryVecEndIter; categoryVecIter++)
        {
          buildSequenceInfo(taxTrx, *categoryVecIter, taxDisplayItem);
        }
      }
    }

    return true;
  }

  if (taxTrx.getRequest()->categoryVec().empty())
  {
    for (uint32_t i = 1; i <= maxCategories; i++)
      taxTrx.getRequest()->categoryVec().push_back(i);
  }

  TaxDisplayItem* taxDisplayItem = taxResponse->taxDisplayItemVector().front();

  std::vector<uint32_t>::const_iterator categoryVecIter =
      taxTrx.getRequest()->categoryVec().begin();
  std::vector<uint32_t>::const_iterator categoryVecEndIter =
      taxTrx.getRequest()->categoryVec().end();

  buildDateVerbiage(taxTrx, *taxDisplayItem);

  for (; categoryVecIter != categoryVecEndIter; categoryVecIter++)
  {
    buildSequenceInfo(taxTrx, *categoryVecIter, taxDisplayItem);
  }

  if (taxDisplayItem->category1() == nullptr)
  {
    buildCat1(taxTrx, *taxDisplayItem);
  }

  return true;
}

void
TaxDisplayCAT::buildCatDiff(TaxDisplayItem* taxDisplayItem1, TaxDisplayItem* taxDisplayItem2)
{
  bool changedetected = false;
#define IDENTCLEAN(x, y)                                                                           \
  if (taxDisplayItem1->category##x()->subCat##y() == taxDisplayItem2->category##x()->subCat##y())  \
  {                                                                                                \
    taxDisplayItem1->category##x()->subCat##y().clear();                                           \
    taxDisplayItem2->category##x()->subCat##y().clear();                                           \
  }                                                                                                \
  else                                                                                             \
    changedetected = true;

  // PL 18027
  IDENTCLEAN(1, 1);
  IDENTCLEAN(1, 2);
  IDENTCLEAN(1, 3);
  IDENTCLEAN(1, 4);
  IDENTCLEAN(1, 5);
  IDENTCLEAN(1, 6);
  IDENTCLEAN(1, 7);
  IDENTCLEAN(1, 8);
  IDENTCLEAN(1, 9);
  IDENTCLEAN(1, 10);
  IDENTCLEAN(1, 11);

  IDENTCLEAN(2, 1);
  IDENTCLEAN(2, 2);
  IDENTCLEAN(2, 3);
  IDENTCLEAN(2, 4);
  IDENTCLEAN(2, 5);
  IDENTCLEAN(2, 6);
  IDENTCLEAN(2, 7);
  IDENTCLEAN(2, 8);
  IDENTCLEAN(2, 9);

  IDENTCLEAN(3, 1);
  IDENTCLEAN(3, 2);

  IDENTCLEAN(4, 1);
  IDENTCLEAN(4, 2);
  IDENTCLEAN(4, 3);
  IDENTCLEAN(4, 4);
  IDENTCLEAN(4, 5);

  IDENTCLEAN(5, 1);
  IDENTCLEAN(5, 2);

  IDENTCLEAN(6, 1);
  IDENTCLEAN(6, 2);
  IDENTCLEAN(6, 3);
  IDENTCLEAN(6, 4);
  IDENTCLEAN(6, 5);
  IDENTCLEAN(6, 6);
  IDENTCLEAN(6, 7);

  IDENTCLEAN(7, 1);
  IDENTCLEAN(7, 2);
  IDENTCLEAN(7, 3);

  IDENTCLEAN(8, 1);
  IDENTCLEAN(8, 2);

  IDENTCLEAN(9, 1);
  IDENTCLEAN(9, 2);

  IDENTCLEAN(10, 1);
  IDENTCLEAN(10, 2);

  IDENTCLEAN(11, 1);
  IDENTCLEAN(11, 2);

  IDENTCLEAN(12, 1);
  IDENTCLEAN(11, 2);

  IDENTCLEAN(13, 1);
  IDENTCLEAN(13, 2);
  IDENTCLEAN(13, 3);

  IDENTCLEAN(14, 1);
  IDENTCLEAN(14, 2);

  IDENTCLEAN(15, 1);
  IDENTCLEAN(15, 2);
  IDENTCLEAN(15, 3);
  IDENTCLEAN(15, 4);

  IDENTCLEAN(16, 1);
  IDENTCLEAN(17, 1);

  IDENTCLEAN(18, 1);

  if (!changedetected)
  {
    taxDisplayItem1->category1()->subCat1() = "SEQUENCE COMPARISON DETECTED NO DIFFERENCES";
  }

#undef IDENTCLEAN
}

// ----------------------------------------------------------------------------
// Description:  Tax Display buildSequenceInfo
// ----------------------------------------------------------------------------
void
TaxDisplayCAT::buildSequenceInfo(TaxTrx& taxTrx, uint16_t cat, TaxDisplayItem* taxDisplayItem)
{
  buildDateVerbiage(taxTrx, *taxDisplayItem);

  switch (cat)
  {
  case 1:
    buildCat1(taxTrx, *taxDisplayItem, false);
    break;
  case 2:
    buildCat2(taxTrx, *taxDisplayItem);
    break;
  case 3:
    buildCat3(taxTrx, *taxDisplayItem);
    break;
  case 4:
    buildCat4(taxTrx, *taxDisplayItem);
    break;
  case 5:
    buildCat5(taxTrx, *taxDisplayItem);
    break;
  case 6:
    buildCat6(taxTrx, *taxDisplayItem);
    break;
  case 7:
    buildCat7(taxTrx, *taxDisplayItem);
    ;
    break;
  case 8:
    buildCat8(taxTrx, *taxDisplayItem);
    break;
  case 9:
    buildCat9(taxTrx, *taxDisplayItem);
    break;
  case 10:
    buildCat10(taxTrx, *taxDisplayItem);
    break;
  case 11:
    buildCat11(taxTrx, *taxDisplayItem);
    break;
  case 12:
    buildCat12(taxTrx, *taxDisplayItem);
    break;
  case 13:
    buildCat13(taxTrx, *taxDisplayItem);
    break;
  case 14:
    buildCat14(taxTrx, *taxDisplayItem);
    break;
  case 15:
    buildCat15(taxTrx, *taxDisplayItem);
    break;
  case 16:
    buildCat16(taxTrx, *taxDisplayItem);
    break;
  case 17:
    buildCat17(taxTrx, *taxDisplayItem);
    break;
  case 18:
    buildCat18(taxTrx, *taxDisplayItem);
    break;
  default:
    break;
  }
}

// ----------------------------------------------------------------------------
// Description:  Tax Display Date Verbiage
// ----------------------------------------------------------------------------
void
TaxDisplayCAT::buildDateVerbiage(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  if (taxTrx.getRequest()->effectiveDate() != DateTime::emptyDate())
    return;

  CategorySeqDataVerb* categorySeqDataVerb = nullptr;
  taxTrx.dataHandle().get(categorySeqDataVerb);
  if (categorySeqDataVerb == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }
  categorySeqDataVerb->build(taxTrx, taxDisplayItem);
  taxDisplayItem.categorySeqDataVerb() = categorySeqDataVerb;
}
// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 1
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat1(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem, bool singleSequence)
{
  Category1* category1 = nullptr;
  taxTrx.dataHandle().get(category1);

  if (category1 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  category1->build(taxTrx, taxDisplayItem, singleSequence);
  taxDisplayItem.category1() = category1;
}
// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 2
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat2(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category2* category2 = nullptr;
  taxTrx.dataHandle().get(category2);

  if (category2 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }
  category2->build(taxTrx, taxDisplayItem);
  taxDisplayItem.category2() = category2;
}
// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 3
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat3(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category3* category3 = nullptr;
  taxTrx.dataHandle().get(category3);

  if (category3 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  category3->build(taxTrx, taxDisplayItem);
  taxDisplayItem.category3() = category3;
}
// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 4
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat4(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category4* category4 = nullptr;
  taxTrx.dataHandle().get(category4);

  if (category4 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  category4->build(taxTrx, taxDisplayItem);
  taxDisplayItem.category4() = category4;
}
// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 5
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat5(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category5* category5 = nullptr;
  taxTrx.dataHandle().get(category5);

  if (category5 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  category5->build(taxTrx, taxDisplayItem);
  taxDisplayItem.category5() = category5;
}
// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 6
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat6(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category6* category6 = nullptr;
  taxTrx.dataHandle().get(category6);

  if (category6 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  category6->build(taxTrx, taxDisplayItem);
  taxDisplayItem.category6() = category6;
}
// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 7
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat7(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category7* category7 = nullptr;
  taxTrx.dataHandle().get(category7);

  if (category7 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  category7->build(taxTrx, taxDisplayItem);
  taxDisplayItem.category7() = category7;
}
// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 8
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat8(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category8* category8 = nullptr;
  taxTrx.dataHandle().get(category8);

  if (category8 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  category8->build(taxTrx, taxDisplayItem);
  taxDisplayItem.category8() = category8;
}
// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 9
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat9(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category9* category9 = nullptr;
  taxTrx.dataHandle().get(category9);

  if (category9 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  category9->build(taxTrx, taxDisplayItem);
  taxDisplayItem.category9() = category9;
}
// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 10
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat10(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category10* category10 = nullptr;
  taxTrx.dataHandle().get(category10);

  if (category10 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  category10->build(taxTrx, taxDisplayItem);
  taxDisplayItem.category10() = category10;
}

// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 11
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat11(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category11* category11 = nullptr;
  taxTrx.dataHandle().get(category11);

  if (category11 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  category11->build(taxTrx, taxDisplayItem);
  taxDisplayItem.category11() = category11;
}

// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 12
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat12(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category12* category12 = nullptr;
  taxTrx.dataHandle().get(category12);

  if (category12 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  category12->build(taxTrx, taxDisplayItem);
  taxDisplayItem.category12() = category12;
}

// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 13
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat13(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category13* category13 = nullptr;
  taxTrx.dataHandle().get(category13);

  if (category13 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  category13->build(taxTrx, taxDisplayItem);
  taxDisplayItem.category13() = category13;
}

// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 14
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat14(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category14* category14 = nullptr;
  taxTrx.dataHandle().get(category14);

  if (category14 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  category14->build(taxTrx, taxDisplayItem);
  taxDisplayItem.category14() = category14;
}

// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 15
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat15(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category15* category15 = nullptr;
  taxTrx.dataHandle().get(category15);

  if (category15 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  category15->build(taxTrx, taxDisplayItem);
  taxDisplayItem.category15() = category15;
}
// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 16
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat16(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category16* category16 = nullptr;
  taxTrx.dataHandle().get(category16);

  if (category16 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  category16->build(taxTrx, taxDisplayItem);
  taxDisplayItem.category16() = category16;
}
// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 17
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat17(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category17* category17 = nullptr;
  taxTrx.dataHandle().get(category17);

  if (category17 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  category17->build(taxTrx, taxDisplayItem);
  taxDisplayItem.category17() = category17;
}
// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 18
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildCat18(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Category18* category18 = nullptr;
  taxTrx.dataHandle().get(category18);

  if (category18 == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  category18->build(taxTrx, taxDisplayItem);
  taxDisplayItem.category18() = category18;
}
// ----------------------------------------------------------------------------
// Description:  Tax Display CAT 18
// ----------------------------------------------------------------------------

void
TaxDisplayCAT::buildReissue(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  Reissue* reissue = nullptr;
  taxTrx.dataHandle().get(reissue);

  if (reissue == nullptr)
  {
    LOG4CXX_WARN(_logger, "**** No MEMORY Available ****");
    return;
  }

  reissue->build(taxTrx, taxDisplayItem);
  taxDisplayItem.reissue() = reissue;
}
