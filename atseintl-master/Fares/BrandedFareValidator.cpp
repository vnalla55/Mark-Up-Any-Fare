//----------------------------------------------------------------------------
//
//  Copyright Sabre 2012
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

#include "Fares/BrandedFareValidator.h"

#include "Common/FareMarketUtil.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

#include <set>
#include <vector>

namespace tse
{
ConfigurableValue<ConfigSet<std::string>>
BrandedFareValidator::_allowedPartitionsIdsFromConfig(
    "SHOPPING_OPT", "EXTENDED_BOOKING_CODE_VALIDATION_PARTITIONS_IDS");

namespace
{
class DiagFormatter
{
public:
  DiagFormatter(DiagManager& diag, bool isBrandIdPresent, const std::string& trailingMsg = "\n")
    : _diag(diag), _isBrandIdPresent(isBrandIdPresent), _trailingMsg(trailingMsg)
  {
  }

  void printFailBranded(const Code<2>& brandId, const FareClassCode& fareClass) const
  {
    if (LIKELY(!_diag.isActive()))
      return;

    printFailBrandedCommon(brandId);
    _diag << " CHK - FBCODE " << std::setw(8) << std::left << fareClass
          << " NOT IN SPECIFIED BKCODES, FBCODES NOR FFAMILIES" << _trailingMsg;
  }

  void printFailBrandedSelectionForBookingCodes(const Code<2>& brandId,
                                                bool matchedCorpID,
                                                const FareClassCode& fareClass,
                                                const BookingCode& fareBookingCode) const
  {
    if (LIKELY(!_diag.isActive()))
      return;

    printFailBrandedSelectionCommon(brandId, matchedCorpID, fareClass);
    _diag << " BKCODE " << fareBookingCode << _trailingMsg;
  }

  void printFailBrandedSelectionForFareFamily(const Code<2>& brandId,
                                              bool matchedCorpID,
                                              const FareClassCode& fareClass,
                                              const FareClassCode& fareFamily) const
  {
    if (LIKELY(!_diag.isActive()))
      return;

    printFailBrandedSelectionCommon(brandId, matchedCorpID, fareClass);
    _diag << " FFAMILY " << fareFamily << _trailingMsg;
  }

  void printFailBrandedSelectionForFareBasis(const Code<2>& brandId,
                                             bool matchedCorpID,
                                             const FareClassCode& fareClass) const
  {
    if (LIKELY(!_diag.isActive()))
      return;

    printFailBrandedSelectionCommon(brandId, matchedCorpID, fareClass, "FBCODE ");
    _diag << _trailingMsg;
  }

  void
  printFailBrandedExcludesForFareBasis(const Code<2>& brandId, const FareClassCode& fareClass) const
  {
    if (LIKELY(!_diag.isActive()))
      return;

    printFailBrandedCommon(brandId);
    _diag << " CHK - FBCODE " << std::setw(8) << std::left << fareClass << " EXCLUDED.\n";
  }

  void printFailBrandedExcludesForFareFamily(const Code<2>& brandId,
                                             const FareClassCode& fareClass,
                                             const FareClassCode& fareFamily) const
  {
    if (LIKELY(!_diag.isActive()))
      return;

    printFailBrandedCommon(brandId);
    _diag << " CHK FOR FBCODE " << std::setw(8) << std::left << fareClass << " - FFAMILY "
          << fareFamily << " EXCLUDED.\n";
  }

protected:
  std::string getBrandId(const Code<2>& brandId) const
  {
    return _isBrandIdPresent ? " " + brandId : "";
  }

  const char* getFareType(bool matchedCorpID) const { return matchedCorpID ? "PUB" : "CORP ID"; }

  void printFailBrandedCommon(const Code<2>& brandId) const
  {
    _diag << "FAIL BRANDED" << getBrandId(brandId);
  }

  void printFailBrandedSelectionCommon(const Code<2>& brandId,
                                       bool matchedCorpID,
                                       const FareClassCode& fareClass,
                                       const std::string& extra = "") const
  {
    printFailBrandedCommon(brandId);
    _diag << " CHK - REQUIRE " << getFareType(matchedCorpID) << " FARE FOR " << extra
          << std::setw(8) << std::left << fareClass;
  }

  DiagManager& _diag;
  const bool _isBrandIdPresent;
  const std::string _trailingMsg;
};

template <typename T>
bool
isPresent(const T& elem, const std::vector<T>& coll)
{
  return coll.end() != std::find(coll.begin(), coll.end(), elem);
}

template <typename T, typename P>
bool
isPresent(const P& pred, const std::vector<T>& coll)
{
  return coll.end() != std::find_if(coll.begin(), coll.end(), pred);
}

struct IsFareFamily
{
  IsFareFamily(const FareClassCode& fareClass) : _fareClass(fareClass) {}

  bool operator()(const FareClassCode& family) const
  {
    return RuleUtil::matchFareClass(family.c_str(), _fareClass.c_str());
  }

protected:
  const FareClassCode& _fareClass;
};

typedef bool (Fare::*IsState)(const uint16_t) const;
typedef bool (Fare::*SetState)(const uint16_t, bool);

class ValidationEngine
{
public:
  ValidationEngine(const BrandedFaresData& brandedData, DiagFormatter& formatter)
    : _brandedData(brandedData), _formatter(formatter)
  {
  }

  void check(const BookingCode& fareBookingCode, PaxTypeFare& paxTypeFare) const
  {
    for(const BrandedFaresData::value_type& brand : _brandedData)
    {
      if (paxTypeFare.fare()->isInvBrand(brand.first))
        continue;

      bool valid = isValid(brand.second, fareBookingCode, paxTypeFare);
      paxTypeFare.fare()->setInvBrand(brand.first, !valid);
    }
  }

protected:
  bool isValid(const BrandedFareInfo& info,
               const BookingCode& fareBookingCode,
               const PaxTypeFare& paxTypeFare) const
  {
    const bool isBookingCodePresent =
        isPresent(fareBookingCode, info.fareBookingCode) ||
        isPresent(fareBookingCode, info.fareSecondaryBookingCode) ||
        isPresent(paxTypeFare.fareClass(), info.fareBasisCode) ||
        isPresent(IsFareFamily(paxTypeFare.fareClass()), info.fareFamily);

    if (isBookingCodePresent)
      return true;

    _formatter.printFailBranded(info.brandId, paxTypeFare.fareClass());
    return false;
  }

  const BrandedFaresData& _brandedData;
  DiagFormatter& _formatter;
};

class SelectionValidationEngine
{
public:
  SelectionValidationEngine(const BrandedFaresData& data, DiagFormatter& formatter)
    : _brandedData(data), _formatter(formatter), _isOnlyOneBrand(data.getSize() == 1)
  {
  }

  void check(const BookingCode& fareBookingCode, PaxTypeFare& paxTypeFare) const
  {
    for(const BrandedFaresData::value_type& brand : _brandedData)
    {
      if (paxTypeFare.fare()->isInvBrand(brand.first))
        continue;

      bool valid = isValid(brand.second, fareBookingCode, paxTypeFare);
      paxTypeFare.fare()->setInvBrandCorpID(brand.first, !valid);
    }
  }

protected:
  bool checkSelectionIndicator(Indicator indicator, bool matchedCorpID) const
  {
    return (indicator == 'C' && !matchedCorpID) || (indicator == 'P' && matchedCorpID);
  }

  template <typename T>
  bool isRequireSpecialFare(const T& elem, const std::map<T, char>& coll, bool matchedCorpID) const
  {
    typename std::map<T, char>::const_iterator i = coll.find(elem);
    return (i != coll.end()) && checkSelectionIndicator(i->second, matchedCorpID);
  }

  bool isRequireSpecialFare(const FareClassCode& elem,
                            const std::map<FareClassCode, char>& coll,
                            bool matchedCorpID,
                            FareClassCode& family) const
  {
    IsFareFamily checkFareFamily(elem);
    typedef std::map<FareClassCode, char>::value_type Item;
    for(const Item& item : coll)
      if (checkFareFamily(item.first) && checkSelectionIndicator(item.second, matchedCorpID))
      {
        family = item.first;
        return true;
      }

    return false;
  }

  bool isValidForBookingCodes(const BrandedFareInfo& info,
                              const BookingCode& fareBookingCode,
                              const PaxTypeFare& paxTypeFare) const
  {
    if (isRequireSpecialFare(
            fareBookingCode, info.fareBookingCodeData, paxTypeFare.matchedCorpID()) ||
        isRequireSpecialFare(
            fareBookingCode, info.fareSecondaryBookingCodeData, paxTypeFare.matchedCorpID()))
    {
      _formatter.printFailBrandedSelectionForBookingCodes(
          info.brandId, paxTypeFare.matchedCorpID(), paxTypeFare.fareClass(), fareBookingCode);
      return false;
    }

    return true;
  }

  bool isValidForFareFamily(const BrandedFareInfo& info, const PaxTypeFare& paxTypeFare) const
  {
    FareClassCode family;
    if (isRequireSpecialFare(
            paxTypeFare.fareClass(), info.fareFamilyData, paxTypeFare.matchedCorpID(), family))
    {
      _formatter.printFailBrandedSelectionForFareFamily(
          info.brandId, paxTypeFare.matchedCorpID(), paxTypeFare.fareClass(), family);
      return false;
    }

    return true;
  }

  bool isValidForFareBasis(const BrandedFareInfo& info, const PaxTypeFare& paxTypeFare) const
  {
    if (isRequireSpecialFare(
            paxTypeFare.fareClass(), info.fareBasisCodeData, paxTypeFare.matchedCorpID()))
    {
      _formatter.printFailBrandedSelectionForFareBasis(
          info.brandId, paxTypeFare.matchedCorpID(), paxTypeFare.fareClass());
      return false;
    }

    return true;
  }

  bool isValid(const BrandedFareInfo& info, const BookingCode& bc, PaxTypeFare& paxTypeFare) const
  {
    if (!isValidForBookingCodes(info, bc, paxTypeFare))
    {
      if (_isOnlyOneBrand)
        paxTypeFare.bookingCodeStatus().set(PaxTypeFare::BKS_FAIL);
      return false;
    }

    return (isValidForFareFamily(info, paxTypeFare) && isValidForFareBasis(info, paxTypeFare));
  }

  const BrandedFaresData& _brandedData;
  DiagFormatter& _formatter;
  const bool _isOnlyOneBrand;
};

class ExcludeFareValidator
{
public:
  ExcludeFareValidator(const BrandedFaresData& data, DiagFormatter& formatter)
    : _brandedData(data), _formatter(formatter)
  {
  }

  void check(PaxTypeFare& paxTypeFare) const
  {
    for(const BrandedFaresData::value_type& brand : _brandedData)
    {
      paxTypeFare.fare()->setInvBrand(brand.first, !isValid(brand.second, paxTypeFare));
    }
  }

protected:
  bool isValid(const BrandedFareInfo& info, const PaxTypeFare& paxTypeFare) const
  {
    return (isValidForFareFamily(info, paxTypeFare) && isValidForFareBasis(info, paxTypeFare));
  }

  bool isValidForFareFamily(const BrandedFareInfo& info, const PaxTypeFare& paxTypeFare) const
  {
    IsFareFamily isFareFamily(paxTypeFare.fareClass());
    for(const FareClassCode& item : info.fareFamilyExclude)
      if (isFareFamily(item))
      {
        _formatter.printFailBrandedExcludesForFareFamily(
            info.brandId, paxTypeFare.fareClass(), item);
        return false;
      }

    return true;
  }

  bool isValidForFareBasis(const BrandedFareInfo& info, const PaxTypeFare& paxTypeFare) const
  {
    if (isPresent(paxTypeFare.fareClass(), info.fareBasisCodeExclude))
    {
      _formatter.printFailBrandedExcludesForFareBasis(info.brandId, paxTypeFare.fareClass());
      return false;
    }

    return true;
  }

  const BrandedFaresData& _brandedData;
  DiagFormatter& _formatter;
};

bool
isFirstSegmentRebooked(const std::vector<PaxTypeFare::SegmentStatus>& segmentStatus)
{
  return !segmentStatus.empty() &&
         segmentStatus[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED);
}

void
regularValidation(const PricingTrx& trx,
                  const ValidationEngine& validator,
                  const SelectionValidationEngine& selectionValidator,
                  const bool allowedPartitionsId,
                  PaxTypeFare& paxTypeFare)
{
  BookingCode fareBookingCode = isFirstSegmentRebooked(paxTypeFare.segmentStatus())
                                    ? paxTypeFare.segmentStatus()[0]._bkgCodeReBook
                                    : paxTypeFare.bookingCode();

  if (allowedPartitionsId)
  {
    if (fareBookingCode.empty() && isFirstSegmentRebooked(paxTypeFare.segmentStatusRule2()))
      fareBookingCode = paxTypeFare.segmentStatusRule2()[0]._bkgCodeReBook;
  }

  validator.check(fareBookingCode, paxTypeFare);
  selectionValidator.check(fareBookingCode, paxTypeFare);
}

} // namespace

BrandedFareValidator::BrandedFareValidator(const PricingTrx& trx, DiagManager& diag)
  : _trx(trx),
    _req(*trx.getRequest()),
    _diag(diag),
    _allowedPartitionsId(
        trx.billing() ? _allowedPartitionsIdsFromConfig.getValue().has(trx.billing()->partitionID())
                      : false)
{
}

void
BrandedFareValidator::printFareMarket(const FareMarket& fareMarket) const
{
  if (fareMarket.travelSeg().empty() || !_diag.isActive())
    return;

  _diag << " \n" << FareMarketUtil::getBoardMultiCity(fareMarket, *fareMarket.travelSeg().front());

  for(const TravelSeg* segment : fareMarket.travelSeg())
  {
    _diag << "-";
    if (segment->isAir())
      _diag << static_cast<const AirSeg*>(segment)->carrier() << "-";
    _diag << FareMarketUtil::getOffMultiCity(fareMarket, *segment);
  }

  _diag << "    /CXR-" << fareMarket.governingCarrier() << "/";

  std::string globalDir;
  globalDirectionToStr(globalDir, fareMarket.getGlobalDirection());

  _diag << " #GI-" << globalDir << "#  " << fareMarket.getDirectionAsString() << "\n";
}

void
BrandedFareValidator::regularFaresValidation(const std::vector<PaxTypeFare*>& fares) const
{
  DiagFormatter formatter(_diag, _req.getBrandedFaresData().getSize() > 1);

  ValidationEngine val(_req.getBrandedFaresData(), formatter);
  SelectionValidationEngine selVal(_req.getBrandedFaresData(), formatter);

  for(PaxTypeFare* ptf : fares)
  {
    if (!ptf->isValidNoBookingCode())
      continue;
    regularValidation(_trx, val, selVal, _allowedPartitionsId, *ptf);
  }
}

void
BrandedFareValidator::excludeFaresValidation(const std::vector<PaxTypeFare*>& fares) const
{
  DiagFormatter formatter(_diag, _req.getBrandedFaresData().getSize() > 1);

  ExcludeFareValidator val(_req.getBrandedFaresData(), formatter);

  for(PaxTypeFare* ptf : fares)
  {
    val.check(*ptf);
  }
}

void
BrandedFareValidator::brandedFareValidation(const FareMarket& fareMarket) const
{
  printFareMarket(fareMarket);
  regularFaresValidation(fareMarket.allPaxTypeFare());
}

void
BrandedFareValidator::excludeBrandedFareValidation(const FareMarket& fareMarket) const
{
  printFareMarket(fareMarket);

  excludeFaresValidation(fareMarket.allPaxTypeFare());
}

} // tse
