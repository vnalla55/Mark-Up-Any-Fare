//----------------------------------------------------------------------------
//
//      File: PricingDetailModelMap.h
//      Description: Create and interpret Data Model mappings for Pricing Detail
//                   request.
//      Created: May 17, 2005
//      Authors: Andrea Yang
//
//  Copyright Sabre 2005
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

#include "Common/Config/ConfigMan.h"
#include "DataModel/Trx.h"
#include "DBAccess/DataHandle.h"
#include "Xform/DataModelMap.h"

#include <xercesc/sax2/Attributes.hpp>

#include <map>
#include <stack>
#include <string>

namespace tse
{

class PricingDetailTrx;
class SegmentDetail;
class SurchargeDetail;
class TaxDetail;
class TaxBreakdown;
class FareCalcDetail;
class PaxDetail;
class CurrencyDetail;
class PlusUpDetail;
class DifferentialDetail;
class SellingFareData;

class PricingDetailModelMap : public DataModelMap
{
public:
  enum WpdfType
  {
    WPDF_NONE = 0,
    WPDF_AFTER_WP,
    WPDF_AFTER_WPX // wpdf after wpa or wqdf
  };

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::PricingDetailModelMap
  //
  // Description: constructor
  //
  // @param config - reference to a valid ConfigMan
  // @param dataHandle - reference to a valid dataHandle
  // @param trx - reference to a Trx
  // @param isWpdfTrx - true if this is a wpdf trx
  //--------------------------------------------------------------------------
  PricingDetailModelMap(ConfigMan& config, DataHandle& dataHandle, Trx*& trx, WpdfType wpdfType)
    : DataModelMap(config, dataHandle, trx), _wpdfType(wpdfType)
  {
  }
  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::~PricingDetailModelMap
  //
  // Description: destructor
  //--------------------------------------------------------------------------
  virtual ~PricingDetailModelMap();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::createMap
  //
  // Description: Work with a currently initialized configuration file to
  //              create a working interpretation of a Data Model.
  //
  // @param none
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  bool createMap() override;

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::classMapEntry
  //
  // Description: Multi purpose method.
  //     1.  tagName can be a class tag name
  //     2.  tagName can be a member tag name
  //     3.  tagName can be a data value
  //
  // @param tagName - value to be evaluated
  // @param atts - vector of attributes
  // @return bool - success or failure
  //--------------------------------------------------------------------------
  bool classMapEntry(std::string& tagName, const xercesc::Attributes& atts) override;

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::saveMapEntry
  //
  // Description: Using the current mapping close out any processing and
  //              save the object to the Trx object
  //
  // @param tagName - value to be evaluated
  //--------------------------------------------------------------------------
  void saveMapEntry(std::string& tagName) override;

private:
  struct Mapping
  {
    void (PricingDetailModelMap::*func)(const xercesc::Attributes&); // store function
    void (PricingDetailModelMap::*trxFunc)(); // Trx interaction func
    MemberMap members; // associative members
  };

  PricingDetailTrx* _pricingDetailTrx = nullptr;
  PaxDetail* _paxDetail = nullptr;
  TaxDetail* _taxDetail = nullptr;
  TaxBreakdown* _taxBreakdown = nullptr;
  CurrencyDetail* _currencyDetail = nullptr;
  PlusUpDetail* _plusUpDetail = nullptr;
  FareCalcDetail* _fareCalcDetail = nullptr;
  SegmentDetail* _segmentDetail = nullptr;
  SurchargeDetail* _surchargeDetail = nullptr;
  DifferentialDetail* _differentialDetail = nullptr;
  SellingFareData* _sellingFareData = nullptr;

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storePricingDetailInformation
  //
  // Description: This is the equivalent of a document start for a Pricing
  //              Detail xml document.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storePricingDetailInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::savePricingDetailInformation
  //
  // Description: This is the equivalent of a document end for a Pricing
  //              detail xml document.
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void savePricingDetailInformation();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::nullStoreFunction
  //
  // Description: Used when no parsing action is needed for an element
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void nullStoreFunction(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::nullSaveFunction
  //
  // Description: Used when no parsing action is needed for an element
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void nullSaveFunction();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storeBillingInformation
  //
  // Description: Convenience method for mapping BIL sent XML summary
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeBillingInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::saveBillingInformation
  //
  // Description: Convenience method for closing out a populated
  //              Billing object
  //
  // @param none
  // @return void
  //--------------------------------------------------------------------------
  void saveBillingInformation();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storeSummaryInformation
  //
  // Description: Convenience method for mapping SUM sent XML summary
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeSummaryInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::saveSummaryInformation
  //
  // Description: Convenience method for closing out a populated
  //              SummaryDetail object
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveSummaryInformation();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storePassengerInformation
  //
  // Description: Convenience method for mapping PXI sent XML PaxDetail
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storePassengerInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::savePassengerInformation
  //
  // Description: Convenience method for closing out a populated
  //              PassengerDetail object
  //
  // @return void
  //--------------------------------------------------------------------------
  void savePassengerInformation();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storeTaxInformation
  //
  // Description: Convenience method for mapping TAX sent XML tax
  //              detail into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeTaxInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::saveTaxInformation
  //
  // Description: Convenience method for saving a populated travel
  //              tax detail object to the trx object.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveTaxInformation();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storeTaxBreakdown
  //
  // Description: Convenience method for mapping TAX sent XML tax
  //              detail into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeTaxBreakdown(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::saveTaxBreakdown
  //
  // Description: Convenience method for saving a populated travel
  //              tax detail object to the trx object.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveTaxBreakdown();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storeCurrencyConversion
  //
  // Description: Convenience method for mapping currency conversion
  //              detail into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeCurrencyConversion(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::saveCurrencyConversion
  //
  // Description: Convenience method for saving a populated travel
  //              tax detail object to the trx object.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveCurrencyConversion();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storeFareIATARate
  //
  // Description: Convenience method for mapping currency conversion
  //              detail into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeFareIATARate(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::saveFareIATARate
  //
  // Description: Convenience method for saving a populated currency
  //              exchange detail object to the trx object.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveFareIATARate();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storeFareBankerSellRate
  //
  // Description: Convenience method for mapping currency conversion
  //              detail into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeFareBankerSellRate(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::saveFareBankerSellRate
  //
  // Description: Convenience method for saving a populated currency
  //              exchange detail object to the trx object.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveFareBankerSellRate();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storeTaxBankerSellRate
  //
  // Description: Convenience method for mapping currency conversion
  //              detail into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeTaxBankerSellRate(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::saveTaxBankerSellRate
  //
  // Description: Convenience method for saving a populated currency
  //              exchange detail object to the trx object.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveTaxBankerSellRate();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storePlusUp
  //
  // Description: Convenience method for mapping plus up information
  //              into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storePlusUp(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::savePlusUp
  //
  // Description: Convenience method for saving a populated plus up
  //              detail object to the trx object.
  //
  // @return void
  //--------------------------------------------------------------------------
  void savePlusUp();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storeFareCalcInformation
  //
  // Description: Convenience method for mapping CAL sent fare calc
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeFareCalcInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::saveFareCalcInformation
  //
  // Description: Convenience method for saving a populated FareCalcDetail
  //              object to the trx object.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveFareCalcInformation();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storeSurchargeInformation
  //
  // Description: Convenience method for mapping PCC sent XML options
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeSurchargeInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::saveSurchargeInformation
  //
  // Description: Convenience method for saving populated surcharge detail
  //              information to the request object
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveSurchargeInformation();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storeSegmentInformation
  //
  // Description: Convenience method for mapping PCC sent XML segment
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeSegmentInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::saveSegmentInformation
  //
  // Description: Convenience method for saving a populated segment detail
  //              object to the trx vector.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveSegmentInformation();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storeHIPInformation
  //
  // Description: Convenience method for mapping PCC sent XML segment
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeHIPInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::saveHIPInformation
  //
  // Description: Convenience method for saving a populated HIP detail
  //              object to the trx vector.
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveHIPInformation();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storeAgentInformation
  //
  // Description: Convenience method for mapping AGI sent XML agent
  //              information into the response stream.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeAgentInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::saveAgentInformation
  //
  // Description: Convenience method for saving a populated AGI information
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveAgentInformation();

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storeMessageInformation
  //
  // Description: Convenience method for mapping MSG sent XML segment
  //              information into the response stream.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeMessageInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::saveMessageInformation
  //
  // Description: Convenience method for saving a populated MSG information
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveMessageInformation();

  void storeDynamicConfigOverride(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::storeSellingFareInformation
  //
  // Description: Convenience method for mapping SFD sent XML SellingFareData
  //              information into the Data Model.  Must have a valid
  //              mapping scheme.
  //
  // @param attrs - attribute list
  // @return void
  //--------------------------------------------------------------------------
  void storeSellingFareInformation(const xercesc::Attributes& attrs);

  //--------------------------------------------------------------------------
  // @function PricingDetailModelMap::saveSellingFareInformation
  //
  // Description: Convenience method for closing out a populated
  //              SellingFareData object
  //
  // @return void
  //--------------------------------------------------------------------------
  void saveSellingFareInformation();

private:
  bool _ignoreNetRemit = true;
  bool _isInNetRemit = false;
  bool _isInAsl = false;
  WpdfType _wpdfType;

  void copyTaxOverrideTbd2Tax(PaxDetail* paxDetail);
};
} // End namespace tse
