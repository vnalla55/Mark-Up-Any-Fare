//----------------------------------------------------------------------------
//  File:           TaxItem.h
//  Description:    TaxItem header file for ATSE International Project
//  Created:        2/11/2004
//  Authors:        Dean Van Decker
//
//  Description: This Object will be used for all Tax Build functionality.
//          Tax Package will build TaxItemOut Objects for each Passenger Type
//          /Fare Calculation. The Fare Calculation Configuration and
//          Tax Diagnostics will utilize these objects in a vector in TaxOut.
//
//
//  Updates:
//          2/11/04 - DVD - updated for model changes.
//
//  Copyright Sabre 2004
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/TaxReissue.h"

#include <vector>

namespace tse
{
class TaxTrx;
class TaxNation;
class TaxCodeReg;
class TaxReissue;
class Category1;
class Category2;
class Category3;
class Category4;
class Category5;
class Category6;
class Category7;
class Category8;
class Category9;
class Category10;
class Category11;
class Category12;
class Category13;
class Category14;
class Category15;
class Category16;
class Category17;
class Category18;
class CategorySeqDataVerb;
class Reissue;

class TaxDisplayItem final
{

public:
  using Category1Vector = std::vector<Category1*>;
  using ReissueVector = std::vector<Reissue*>;

  TaxDisplayItem() = default;

  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------
  TaxDisplayItem(const TaxDisplayItem&) = delete;
  TaxDisplayItem& operator=(const TaxDisplayItem&) = delete;
  //-----------------------------------------------------------------------------
  // buildTaxItem will move items into this Object from TaxCodeData Object
  //-----------------------------------------------------------------------------
  void buildTaxDisplayItem(TaxTrx& taxTrx, TaxCodeReg& taxCodeReg, const TaxNation& taxNation);

  TaxCodeReg*& taxCodeReg() { return _taxCodeReg; }
  const TaxCodeReg* taxCodeReg() const { return _taxCodeReg; }

  CategorySeqDataVerb*& categorySeqDataVerb() { return _categorySeqDataVerb; }
  const CategorySeqDataVerb* categorySeqDataVerb() const { return _categorySeqDataVerb; }

  Category1*& category1() { return _category1; }
  const Category1* category1() const { return _category1; }

  Category2*& category2() { return _category2; }
  const Category2* category2() const { return _category2; }

  Category3*& category3() { return _category3; }
  const Category3* category3() const { return _category3; }

  Category4*& category4() { return _category4; }
  const Category4* category4() const { return _category4; }

  Category5*& category5() { return _category5; }
  const Category5* category5() const { return _category5; }

  Category6*& category6() { return _category6; }
  const Category6* category6() const { return _category6; }

  Category7*& category7() { return _category7; }
  const Category7* category7() const { return _category7; }

  Category8*& category8() { return _category8; }
  const Category8* category8() const { return _category8; }

  Category9*& category9() { return _category9; }
  const Category9* category9() const { return _category9; }

  Category10*& category10() { return _category10; }
  const Category10* category10() const { return _category10; }

  Category11*& category11() { return _category11; }
  const Category11* category11() const { return _category11; }

  Category12*& category12() { return _category12; }
  const Category12* category12() const { return _category12; }

  Category13*& category13() { return _category13; }
  const Category13* category13() const { return _category13; }

  Category14*& category14() { return _category14; }
  const Category14* category14() const { return _category14; }

  Category15*& category15() { return _category15; }
  const Category15* category15() const { return _category15; }

  Category16*& category16() { return _category16; }
  const Category16* category16() const { return _category16; }

  Category17*& category17() { return _category17; }
  const Category17* category17() const { return _category17; }

  Category18*& category18() { return _category18; }
  const Category18* category18() const { return _category18; }

  Reissue*& reissue() { return _reissue; }
  const Reissue* reissue() const { return _reissue; }

  TaxReissue*& taxReissue() { return _taxReissue; }
  const TaxReissue* taxReissue() const { return _taxReissue; }

  const TaxCode& taxCode() const { return _taxCode; }

  TaxDescription& taxDescription() { return _taxDescription; }
  const TaxDescription& taxDescription() const { return _taxDescription; }

  TaxDescription& taxNation() { return _taxNation; }
  const TaxDescription& taxNation() const { return _taxNation; }

  TaxDescription& message() { return _message; }
  const TaxDescription& message() const { return _message; }

  const RoundingFactor& roundingUnit() const { return _roundingUnit; }
  const CurrencyNoDec& roundingNoDec() const { return _roundingNoDec; }
  RoundingRule& roundingRule() { return _roundingRule; }

  Description& errorMsg() { return _errorMsg; }
  const Description& errorMsg() const { return _errorMsg; }

private:
  TaxCodeReg* _taxCodeReg = nullptr;
  Category1* _category1 = nullptr;
  Category2* _category2 = nullptr;
  Category3* _category3 = nullptr;
  Category4* _category4 = nullptr;
  Category5* _category5 = nullptr;
  Category6* _category6 = nullptr;
  Category7* _category7 = nullptr;
  Category8* _category8 = nullptr;
  Category9* _category9 = nullptr;
  Category10* _category10 = nullptr;
  Category11* _category11 = nullptr;
  Category12* _category12 = nullptr;
  Category13* _category13 = nullptr;
  Category14* _category14 = nullptr;
  Category15* _category15 = nullptr;
  Category16* _category16 = nullptr;
  Category17* _category17 = nullptr;
  Category18* _category18 = nullptr;

  CategorySeqDataVerb* _categorySeqDataVerb = nullptr;
  Reissue* _reissue = nullptr;
  TaxReissue* _taxReissue = nullptr;

  TaxCode _taxCode;
  TaxDescription _taxDescription;
  TaxDescription _taxNation;
  TaxDescription _message;

  RoundingFactor _roundingUnit = 0;
  CurrencyNoDec _roundingNoDec = 0;
  RoundingRule _roundingRule = RoundingRule::EMPTY;

  Description _errorMsg;
};
} // namespace tse
