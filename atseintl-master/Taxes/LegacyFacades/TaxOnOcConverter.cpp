
// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Taxes/LegacyFacades/TaxOnOcConverter.h"

namespace tax
{
TaxOnOcConverter::TaxOnOcConverter(const V2TrxMappingDetails::OptionalServicesRefs& optServicesMap,
                                   std::shared_ptr<OutputTaxDetails>& outputTaxDetails,
                                   tse::FarePath* farePath)
  : _optServicesMap(optServicesMap),
    _outputTaxDetails(outputTaxDetails),
    _farePath(farePath),
    _ocFeesIterator(_optServicesMap.end())
{
  if (const boost::optional<tax::type::Index>& id = _outputTaxDetails->optionalServiceId())
  {
    _ocFeesIterator = _optServicesMap.find(*id);
  }
}

void
TaxOnOcConverter::convert(tse::OCFees::TaxItem* taxItem) const
{
  if (_ocFeesIterator == _optServicesMap.end())
    return;

  tse::OCFees* ocFees = getOCFees();
  tse::OCFees::Memento memento = ocFees->saveToMemento();
  ocFees->setSeg(getSegmentIndex());
  ocFees->addTax(*taxItem);
  ocFees->restoreFromMemento(memento);
}

int
TaxOnOcConverter::getSegmentIndex() const
{
  return _ocFeesIterator->second.second;
}
tse::OCFees*
TaxOnOcConverter::getOCFees() const
{
  return _ocFeesIterator->second.first;
}

} // end of tax namespace
