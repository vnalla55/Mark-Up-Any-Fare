//----------------------------------------------------------------------------
//  File:        TaxShoppingResponseFormatter.cpp
//  Created:     2012-05-28
//
//  Description: Shopping formatter for charger tax requests
//
//  Updates:
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

#include "Xform/TaxShoppingResponseFormatter.h"

#include "Common/FallbackUtil.h"
#include "DataModel/Itin.h"
#include "DataModel/TaxResponse.h"
#include "Diagnostic/DiagTools.h"

#include <iomanip>
#include <sstream>
#include <string>

namespace tse
{
FALLBACK_DECL(markupAnyFareOptimization);

TaxShoppingResponseFormatter::TaxShoppingResponseFormatter(TaxTrx* taxTrx) : _taxTrx(taxTrx) {}

TaxShoppingResponseFormatter::~TaxShoppingResponseFormatter() {}

void
TaxShoppingResponseFormatter::formatResponse()
{
  buildTaxMap();

  _taxTrx->response().setf(std::ios::fixed, std::ios::floatfield);
  _taxTrx->response().setf(std::ios::right, std::ios::adjustfield);
  _taxTrx->response().precision(2);

  _taxTrx->response() << "<TRS>";

  generateWAR();
  generateTAX();

  if (_taxTrx->isGroupItins())
  {
    for (size_t i = 0; i < _taxTrx->allItins().size(); ++i)
    {
      const ItinInfo* itinInfo = _itinsVec[_taxTrx->shoppingItinMapping()[i]];
      generateCOM(itinInfo, _taxTrx->allItins()[i]);
    }
  }
  else
  {
    std::vector<ItinInfo*>::const_iterator itinIter = _itinsVec.begin();
    std::vector<ItinInfo*>::const_iterator itinIterE = _itinsVec.end();

    for (; itinIter != itinIterE; ++itinIter)
    {
      const ItinInfo* itinInfo = *itinIter;
      generateCOM(itinInfo, itinInfo->_itin);
    }
  }

  generateDIA();
  _taxTrx->response() << "</TRS>";

  return;
}

void
TaxShoppingResponseFormatter::formatResponse(ErrorResponseException& ere)
{
  std::string resp;
  formatResponse(ere, resp);
  _taxTrx->response() << resp;
}

void
TaxShoppingResponseFormatter::formatResponse(const ErrorResponseException& ere, std::string& response)
{
  response = "<TRS>";

  //format error
  response += "<ERE Q1F=\"";
  response += std::to_string(ere.code());
  response += "\" S01=\"";
  response += ere.message();
  response += "\"/>";

  response += "</TRS>";
}


void
TaxShoppingResponseFormatter::buildTaxMap()
{
  int32_t taxId = 1;
  std::vector<Itin*>::const_iterator itinIter = _taxTrx->itin().begin();
  std::vector<Itin*>::const_iterator itinIterE = _taxTrx->itin().end();

  for (; itinIter != itinIterE; ++itinIter)
  {
    const Itin* itin = *itinIter;
    ItinInfo* itinInfo = nullptr;
    _taxTrx->dataHandle().get(itinInfo);

    itinInfo->_itin = itin;

    std::vector<TaxResponse*>::const_iterator taxResponseIter = itin->getTaxResponses().begin();
    std::vector<TaxResponse*>::const_iterator taxResponseIterE = itin->getTaxResponses().end();

    for (; taxResponseIter != taxResponseIterE; ++taxResponseIter)
    {
      const TaxResponse* taxResponse = *taxResponseIter;

      std::vector<TaxRecord*>::const_iterator taxIter = taxResponse->taxRecordVector().begin();
      std::vector<TaxRecord*>::const_iterator taxIterE = taxResponse->taxRecordVector().end();

      for (; taxIter != taxIterE; ++taxIter)
      {
        TaxRecord* taxRecord = *taxIter;

        std::string taxKey = buildTaxKey(taxRecord);
        std::map<std::string, TaxInfo>::iterator taxMapIter = _taxMap.find(taxKey);

        if (taxMapIter == _taxMap.end())
        {
          TaxInfo taxInfo;
          taxInfo._taxRecord = taxRecord;
          taxInfo._id = taxId;

          _taxMap[taxKey] = taxInfo;
          _taxVec.push_back(taxRecord);
          itinInfo->_taxIdVec.push_back(taxId);

          taxId++;
        }
        else
        {
          itinInfo->_taxIdVec.push_back(taxMapIter->second._id);
        }

        itinInfo->_totalTaxAmount += taxRecord->getTaxAmount();
      }
    }

    _itinsVec.push_back(itinInfo);
  }
}

std::string
TaxShoppingResponseFormatter::buildTaxKey(const TaxRecord* taxRecord)
{
  std::ostringstream stream;
  stream << taxRecord->taxCode() << "|" << taxRecord->getTaxAmount();
  return stream.str();
}

void
TaxShoppingResponseFormatter::generateTAX()
{
  int32_t taxId = 1;
  std::vector<TaxRecord*>::const_iterator taxIter = _taxVec.begin();
  std::vector<TaxRecord*>::const_iterator taxIterE = _taxVec.end();

  for (; taxIter != taxIterE; ++taxIter, ++taxId)
  {
    const TaxRecord* taxRecord = *taxIter;

    _taxTrx->response() << "<TAX";

    _taxTrx->response() << " Q1B=\"" << taxId << "\"";
    _taxTrx->response() << " BC0=\"" << taxRecord->taxCode() << "\"";
    _taxTrx->response() << " C6B=\"" << taxRecord->getTaxAmount() << "\"";

    if (!fallback::markupAnyFareOptimization(_taxTrx))
    {
      _taxTrx->response() << " C6M=\"" << taxRecord->taxAmountAdjusted() << "\"";
    }

    _taxTrx->response() << " S04=\"" << taxRecord->taxDescription() << "\"";

    _taxTrx->response() << "/>";
  }
}

void
TaxShoppingResponseFormatter::generateCOM(const ItinInfo* itinInfo, const Itin* itin)
{
  _taxTrx->response() << "<COM";

  _taxTrx->response() << " Q1D=\"" << itin->sequenceNumber() << "\"";

  if (itin->paxGroup().size() > 1)
  {
    _taxTrx->response() << " C65=\"" << itinInfo->_totalTaxAmount << "\"";
  }

  _taxTrx->response() << ">";

  generatePXI(itinInfo);

  _taxTrx->response() << "</COM>";
}

void
TaxShoppingResponseFormatter::generatePXI(const ItinInfo* itinInfo)
{
  PaxType* paxType = itinInfo->_itin->paxGroup().at(0);

  _taxTrx->response() << "<PXI";

  _taxTrx->response() << " B70=\"" << paxType->paxType() << "\"";
  _taxTrx->response() << " Q0W=\"" << paxType->number() << "\"";
  _taxTrx->response() << " C65=\"" << itinInfo->_totalTaxAmount << "\"";

  if (!itinInfo->_taxIdVec.empty())
  {
    std::string taxIds;
    std::vector<int32_t>::const_iterator taxIter = itinInfo->_taxIdVec.begin();
    std::vector<int32_t>::const_iterator taxIterE = itinInfo->_taxIdVec.end();

    for (; taxIter != taxIterE; ++taxIter)
    {
      taxIds += boost::lexical_cast<std::string>(*taxIter);

      if (taxIter != (taxIterE - 1))
      {
        taxIds += "|";
      }
    }

    _taxTrx->response() << " TID=\"" << taxIds << "\"";
  }

  _taxTrx->response() << "/>";
}

void
TaxShoppingResponseFormatter::generateDIA()
{
  if (_taxTrx->diagnostic().diagnosticType() == DiagnosticNone)
    return;

  _taxTrx->response() << "<DIA";
  _taxTrx->response() << " Q0A=\"" << static_cast<int>(_taxTrx->diagnostic().diagnosticType());
  _taxTrx->response() << "\">";

  _taxTrx->response() << utils::truncateDiagIfNeeded(_taxTrx->diagnostic().toString(), *_taxTrx);

  _taxTrx->response() << "</DIA>";
}

void
TaxShoppingResponseFormatter::generateWAR()
{
  if (_taxTrx->errorMsg().empty())
    return;

  _taxTrx->response() << "<WAR";
  _taxTrx->response() << " MSG=\"WARNING: " << _taxTrx->errorMsg();
  _taxTrx->response() << "\"/>";
}
}
