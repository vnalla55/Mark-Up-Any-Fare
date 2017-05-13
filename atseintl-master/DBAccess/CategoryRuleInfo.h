//-------------------------------------------------------------------
//
//  File:	CategoryRuleInfo.h
//  Authors:	Devapriya SenGupta
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
//-------------------------------------------------------------------

#pragma once


#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/Utils/DBStash.h"
#include "Common/Utils/ShadowPtr.h"
#include "Common/Utils/ShadowVector.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"
#include "Util/BranchPrediction.h"

#include <sstream>

namespace tse
{

FIXEDFALLBACK_DECL(fallback_record2_sharing_part2);

inline Logger&
getCategoryRuleInfoLogger()
{
  static Logger logger("atseintl.DBAccess.CategoryRuleInfo");
  return logger;
}

template <class T>
class CategoryRuleInfoT
{
public:
  typedef T item_info_type;

  CategoryRuleInfoT()
  {
    if (!fallback::fixed::fallback_record2_sharing_part2())
    {
      _categoryRuleItemInfoSet.reset(new shadow_vector_type);
    }
  }

  CategoryRuleInfoT(const CategoryRuleInfoT&) = delete;
  CategoryRuleInfoT& operator=(const CategoryRuleInfoT&) = delete;

  virtual ~CategoryRuleInfoT() = default;

  void init()
  {
    _hasCatStopovers = init(8);
    _hasCatTransfers = init(9);
  }

  uint16_t& categoryNumber() { return _categoryNumber; }
  const uint16_t& categoryNumber() const { return _categoryNumber; }

  SequenceNumberLong& sequenceNumber() { return _sequenceNumber; }
  SequenceNumberLong sequenceNumber() const { return _sequenceNumber; }

  VendorCode& vendorCode() { return _vendorCode; }
  const VendorCode& vendorCode() const { return _vendorCode; }

  TariffNumber& tariffNumber() { return _tariffNumber; }
  const TariffNumber& tariffNumber() const { return _tariffNumber; }

  CarrierCode& carrierCode() { return _carrierCode; }
  const CarrierCode& carrierCode() const { return _carrierCode; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  LocKey& loc1() { return _location1; }
  const LocKey& loc1() const { return _location1; }

  LocKey& loc2() { return _location2; }
  const LocKey& loc2() const { return _location2; }

  RuleNumber& ruleNumber() { return _ruleNumber; }
  const RuleNumber& ruleNumber() const { return _ruleNumber; }

  const std::vector<CategoryRuleItemInfoSetT<T>*>& categoryRuleItemInfoSet() const
  {
    return get_shadow_vector().storage();
  }

  void addItemInfoSetNosync(CategoryRuleItemInfoSetT<T>* s)
  {
    TSE_ASSERT(s != nullptr);
    get_mutable_shadow_vector().mutableStorage().push_back(s);
  }


  void shrinkSetsToFit()
  {
    for (auto* s: get_mutable_shadow_vector().mutableStorage())
    {
      s->shrink_to_fit();
    }
  }

  void sync_with_cache()
  {
    shrinkSetsToFit();
    get_mutable_shadow_vector().sync_with_cache();
    if (!fallback::fixed::fallback_record2_sharing_part2())
    {
      _categoryRuleItemInfoSet.sync_with_cache();
    }
    if (UNLIKELY(getCategoryRuleInfoLogger()->isDebugEnabled()))
      tryPrintStashSummary();
  }

  Indicator& applInd() { return _applInd; }
  const Indicator& applInd() const { return _applInd; }

  virtual bool operator==(const CategoryRuleInfoT& rhs) const
  {
    bool eq =
        ((_createDate == rhs._createDate) && (_vendorCode == rhs._vendorCode) &&
         (_tariffNumber == rhs._tariffNumber) && (_carrierCode == rhs._carrierCode) &&
         (_ruleNumber == rhs._ruleNumber) && (_categoryNumber == rhs._categoryNumber) &&
         (_sequenceNumber == rhs._sequenceNumber) && (_location1 == rhs._location1) &&
         (_location2 == rhs._location2) && (_applInd == rhs._applInd) &&
         (_hasCatStopovers == rhs._hasCatStopovers) && (_hasCatTransfers == rhs._hasCatTransfers) &&
         (categoryRuleItemInfoSet().size() == rhs.categoryRuleItemInfoSet().size()));

    for (size_t i = 0; (eq && (i < categoryRuleItemInfoSet().size())); ++i)
    {
      eq = (*(categoryRuleItemInfoSet()[i]) == *(rhs.categoryRuleItemInfoSet()[i]));
    }

    return eq;
  }

  bool hasCat(unsigned catNumber) const
  {
    bool result(true);
    switch (catNumber)
    {
    case 8:
      result = _hasCatStopovers;
      break;
    case 9:
      result = _hasCatTransfers;
      break;
    default:
      break;
    }
    return result;
  }

  bool hasCatStopovers() const { return _hasCatStopovers; }

  bool hasCatTransfers() const { return _hasCatTransfers; }

  static void dummyData(CategoryRuleInfoT& obj)
  {
    CategoryRuleItemInfoSetT<T>* criis1 = new CategoryRuleItemInfoSetT<T>();
    CategoryRuleItemInfoSetT<T>* criis2 = new CategoryRuleItemInfoSetT<T>();

    T crii1;
    T::dummyData(crii1);

    criis1->push_back(crii1);
    criis1->push_back(crii1);
    criis2->push_back(crii1);
    criis2->push_back(crii1);

    obj.addItemInfoSetNosync(criis1);
    obj.addItemInfoSetNosync(criis2);

    obj.createDate() = DateTime(2010, 04, 01, 13, 45, 30, 10);
    obj.vendorCode() = "ABCD";
    obj.tariffNumber() = 1;
    obj.carrierCode() = "EFG";
    obj.ruleNumber() = "HIJK";
    obj.categoryNumber() = 2;
    obj.sequenceNumber() = 3;
    obj.loc1().loc() = "LMNOPQRS";
    obj.loc1().locType() = 'T';
    obj.loc2().loc() = "UVWXYZab";
    obj.loc2().locType() = 'c';
    obj.applInd() = 'd';
    obj._hasCatStopovers = false;
    obj._hasCatTransfers = true;
  }

  virtual void flattenize(Flattenizable::Archive& archive)
  {
    if (fallback::fixed::fallback_record2_sharing_part2())
    {
      FLATTENIZE(archive, _categoryRuleItemInfoSet__OLD__);
    }
    else
    {
      FLATTENIZE(archive, _categoryRuleItemInfoSet);
    }
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _vendorCode);
    FLATTENIZE(archive, _tariffNumber);
    FLATTENIZE(archive, _carrierCode);
    FLATTENIZE(archive, _ruleNumber);
    FLATTENIZE(archive, _categoryNumber);
    FLATTENIZE(archive, _sequenceNumber);
    FLATTENIZE(archive, _location1);
    FLATTENIZE(archive, _location2);
    FLATTENIZE(archive, _applInd);
    FLATTENIZE(archive, _hasCatStopovers);
    FLATTENIZE(archive, _hasCatTransfers);
  }

protected:
  using info_set_type = CategoryRuleItemInfoSetT<T>;
  using shadow_vector_type = ShadowVector<info_set_type>;

  ShadowPtr<shadow_vector_type> _categoryRuleItemInfoSet;

  // TODO(sg218694): remove this with FALLBACK_RECORD2_SHARING_PART2
  shadow_vector_type _categoryRuleItemInfoSet__OLD__;

  DateTime _createDate;
  VendorCode _vendorCode;
  TariffNumber _tariffNumber = 0;
  CarrierCode _carrierCode;
  RuleNumber _ruleNumber;
  SequenceNumberLong _sequenceNumber = 0;
  LocKey _location1;
  LocKey _location2;
  uint16_t _categoryNumber = 0;
  Indicator _applInd = ' ';

public:
  template <typename B, typename U>
  static B& convert(B& buffer, U ptr)
  {
    if (fallback::fixed::fallback_record2_sharing_part2())
    {
      return buffer & ptr->_categoryRuleItemInfoSet__OLD__ & ptr->_createDate & ptr->_vendorCode &
             ptr->_tariffNumber & ptr->_carrierCode & ptr->_ruleNumber & ptr->_categoryNumber &
             ptr->_sequenceNumber & ptr->_location1 & ptr->_location2 & ptr->_applInd &
             ptr->_hasCatStopovers & ptr->_hasCatTransfers;
    }
    return buffer & ptr->_categoryRuleItemInfoSet & ptr->_createDate & ptr->_vendorCode &
           ptr->_tariffNumber & ptr->_carrierCode & ptr->_ruleNumber & ptr->_categoryNumber &
           ptr->_sequenceNumber & ptr->_location1 & ptr->_location2 & ptr->_applInd &
           ptr->_hasCatStopovers & ptr->_hasCatTransfers;
  }

private:
  shadow_vector_type& get_mutable_shadow_vector()
  {
    if (fallback::fixed::fallback_record2_sharing_part2())
    {
      return _categoryRuleItemInfoSet__OLD__;
    }
    else
    {
      TSE_ASSERT(_categoryRuleItemInfoSet.get() != nullptr);
      return *_categoryRuleItemInfoSet;
    }
  }

  const shadow_vector_type& get_shadow_vector() const
  {
    if (fallback::fixed::fallback_record2_sharing_part2())
    {
      return _categoryRuleItemInfoSet__OLD__;
    }
    else
    {
      TSE_ASSERT(_categoryRuleItemInfoSet.get() != nullptr);
      return *_categoryRuleItemInfoSet;
    }
  }

  bool init(unsigned int category)
  {
    for (const auto& iset: categoryRuleItemInfoSet())
    {
      for (const auto& info: *iset)
      {
        if (category == info.itemcat())
        {
          return true;
        }
      }
    }
    return false;
  }

  void tryPrintStashSummary()
  {
    static size_t summary_ctr = 0;

    ++summary_ctr;
    if (summary_ctr >= 5000)
    {
      std::ostringstream out;
      tools::GetStash<CategoryRuleItemInfoSetT<T>>()().print_summary(out);
      LOG4CXX_DEBUG(getCategoryRuleInfoLogger(), out.str());
      summary_ctr = 0;
    }
  }

  bool _hasCatStopovers = true;
  bool _hasCatTransfers = true;
};

using CategoryRuleInfo = CategoryRuleInfoT<CategoryRuleItemInfo>;

inline void pprint_impl(std::ostream& out, const CategoryRuleInfo& c)
{
  out << "<CategoryRuleInfo: ";
  tools::pprint(out, c.categoryRuleItemInfoSet());
  out << ">";
}
} // tse namespace
