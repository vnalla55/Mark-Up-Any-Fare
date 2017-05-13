#include "Xform/TicketingFeesRequestHandlerWPA.h"

#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxDetail.h"
#include "DataModel/PricingDetailTrx.h"
#include "Xform/CustomXMLParser/IAttributes.h"
#include "Xform/CustomXMLParser/IParser.h"
#include "Xform/CustomXMLParser/IXMLSchema.h"
#include "Xform/CustomXMLParser/IXMLUtils.h"
#include "Xform/TicketingFeesSchemaNamesWPA.h"

#include <boost/assign/std/vector.hpp>
#include <boost/algorithm/string.hpp>

using namespace boost::assign;

namespace tse
{
using namespace ancillary;
using namespace ticketingfees;
using namespace ticketingfeeswpa;

namespace
{
Logger
_logger("atseintl.Xform.TicketingFeesRequestHandlerWPA");

ILookupMap _elemLookupMapTktFeesWpa, _attrLookupMapTktFeesWpa;

bool
init(IXMLUtils::initLookupMaps(_AncillaryElementNames,
                               _NumberOfElementNames_,
                               _TicketingFeesElementNames,
                               _NumberOfElementNamesTktFees_,
                               _elemLookupMapTktFeesWpa,
                               _AncillaryAttributeNames,
                               _NumberOfAttributeNames_,
                               _TicketingFeesAttributeNames,
                               _NumberOfAttributeNamesTktFees_,
                               _attrLookupMapTktFeesWpa));

bool
initTktFeeWpaElem(IXMLUtils::initLookupMaps(_TicketingFeesWpaElementNames,
                                            _NumberOfElementNamesTktFeesWpa_,
                                            _elemLookupMapTktFeesWpa));
bool
initTktFeeWpaAttr(IXMLUtils::initLookupMaps(_TicketingFeesWpaAttributeNames,
                                            _NumberOfAttributeNamesTktFeesWpa_,
                                            _attrLookupMapTktFeesWpa));

} // namespace

bool
TicketingFeesRequestHandlerWPA::startElement(int idx, const IAttributes& attrs)
{
  switch (idx)
  {
  case _REQ:
    onStartREQ(attrs);
    break;
  case _DIG:
    onStartDIG(attrs);
    break;
  case _AGI:
    onStartAGI(attrs);
    break;
  case _BIL:
    //onStartBIL(attrs);
    break;
  case _OBF:
    onStartOBF(attrs);
    break;
  case _CAL:
    onStartCAL(attrs);
    break;
  case _SEG:
    onStartSEG(attrs);
    break;
  case _PXI:
    onStartPXI(attrs);
    break;
  case _DCX:
    onStartDCX(attrs);
    break;
  case _SUM:
    onStartSUM(attrs);
    break;
  }
  return true;
}

bool
TicketingFeesRequestHandlerWPA::endElement(int idx)
{
  switch (idx)
  {
  case _AGI:
    onEndAGI();
    break;
  case _PXI:
    onEndPXI();
    break;
  case _SEG:
    onEndSEG();
    break;
  }

  return true;
}

void
TicketingFeesRequestHandlerWPA::parse(DataHandle& dataHandle, const std::string& content)
{
  createTransaction(dataHandle, content);

  IValueString attrValueArray[_NumberOfAttributeNamesTktFeesWpa_];
  int attrRefArray[_NumberOfAttributeNamesTktFeesWpa_];
  IXMLSchema schema(_elemLookupMapTktFeesWpa,
                    _attrLookupMapTktFeesWpa,
                    _NumberOfAttributeNamesTktFeesWpa_,
                    attrValueArray,
                    attrRefArray,
                    true);

  const char* pChar(content.c_str());
  size_t length(content.length());
  size_t pos(content.find_first_of('<'));
  if (pos != std::string::npos)
  {
    pChar += pos;
    length -= pos;
  }
  IParser parser(pChar, length, *this, schema);
  parser.parse();
}

void
TicketingFeesRequestHandlerWPA::createTransaction(DataHandle& dataHandle,
                                                  const std::string& content)
{
  _trx = _pricingTrx = dataHandle.create<PricingDetailTrx>();
  TSE_ASSERT(_trx);
  setTransactionParameters();
}

void
TicketingFeesRequestHandlerWPA::parseCarriersAndStatus(const IAttributes& attrs, AirSeg& as)
{
  CarrierCode marketingCxr;
  getAttr(attrs, _B00, marketingCxr);
  if (marketingCxr.empty())
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                 "INVALID - MISSING CARRIER");

  CarrierCode operatingCxr = marketingCxr;
  getAttr(attrs, _B01, operatingCxr);

  as.setMarketingCarrierCode(MCPCarrierUtil::swapToActual(_trx, marketingCxr));
  as.setOperatingCarrierCode(MCPCarrierUtil::swapToActual(_trx, operatingCxr));
  as.resStatus() = CONFIRM_RES_STATUS;
}

void
TicketingFeesRequestHandlerWPA::determineStopConx(const IAttributes& seg, TravelSeg& ts)
{
  const bool isStopover = seg.get<bool>(_P2M);
  if (isStopover)
    ts.forcedStopOver() = 'T';
  else
    ts.forcedConx() = 'T';
}

void
TicketingFeesRequestHandlerWPA::updateFareData(const IAttributes& seg, TravelSeg& ts)
{
  TSE_ASSERT(_tktFbi);

  if (_tktFbi->vendorCode().empty())
    getAttr(seg, _S37, _tktFbi->vendorCode());

  // Create fare break association
  const bool isSideTripSegment = seg.get<bool>(_P2N);
  const bool isSideTripStart = isSideTripSegment && !_isInSideTrip;
  const bool isSideTripEnd = isSideTripSegment && seg.get<bool>(_S08);

  if (isSideTripStart)
    ++_sideTripId;
  _isInSideTrip = isSideTripSegment && !isSideTripEnd;

  Fba* fba = _pricingTrx->dataHandle().create<Fba>();
  fba->fareComponentID() = _tktFbi->fareComponentID();
  fba->segmentID() = ts.pnrSegment();
  fba->sideTripStart() = isSideTripStart;
  fba->sideTripEnd() = isSideTripEnd;
  fba->sideTripID() = (isSideTripSegment ? _sideTripId : 0);

  _tktFeesRequest->tktFeesFareBreakAssociationPerItin()[_itin].push_back(fba);
}

void
TicketingFeesRequestHandlerWPA::onStartREQ(const IAttributes& attrs)
{
  std::string diagNumStr;
  getAttr(attrs, _C10, diagNumStr);
  if (diagNumStr == "870")
    _pricingTrx->getRequest()->diagnosticNumber() = Diagnostic870;
}

void
TicketingFeesRequestHandlerWPA::onStartDIG(const IAttributes& attrs)
{
  std::string diagArgData;
  getAttr(attrs, _S01, diagArgData);
  if (!diagArgData.empty())
    _pricingTrx->getRequest()->diagArgData().push_back(diagArgData);
}

void
TicketingFeesRequestHandlerWPA::onStartPXI(const IAttributes& attrs)
{
  if (!_isInSum)
    return;
  PaxTypeCode ptc(ADULT);
  getAttr(attrs, _B71, ptc);
  initPaxType(ptc, 1u);

  PaxDetail* paxDetail = _pricingTrx->dataHandle().create<PaxDetail>();
  paxDetail->paxType() = ptc;
  getAttr(attrs, _C43, paxDetail->constructionCurrencyCode());
  setMonayAmountAttr(attrs, _C5E, paxDetail->constructionTotalAmount());
  getAttr(attrs, _C40, paxDetail->baseCurrencyCode());
  setMonayAmountAttr(attrs, _C5A, paxDetail->baseFareAmount());
  getAttr(attrs, _C45, paxDetail->equivalentCurrencyCode());
  setMonayAmountAttr(attrs, _C5F, paxDetail->equivalentAmount());
  getAttr(attrs, _C44, paxDetail->currencyCodeMinimum());
  setMonayAmountAttr(attrs, _C5C, paxDetail->currencyCodeMinimumAmount());
  setIntAttr(attrs, _Q0X, paxDetail->stopoverCount());
  setMonayAmountAttr(attrs, _C63, paxDetail->stopoverCharges());
  getAttr(attrs, _C46, paxDetail->paxFareCurrencyCode());
  setMonayAmountAttr(attrs, _C66, paxDetail->totalPerPassenger());
  setMonayAmountAttr(attrs, _C65, paxDetail->totalTaxes());
  getAttr(attrs, _S66, paxDetail->fareCalcLine());
  setIntAttr(attrs, _C5D, paxDetail->commissionPercentage());
  setMonayAmountAttr(attrs, _C5B, paxDetail->commissionAmount());
  setMonayAmountAttr(attrs, _N09, paxDetail->commissionCap());
  setMonayAmountAttr(attrs, _C64, paxDetail->travelAgencyTax());
  setCharAttr(attrs, _P27, paxDetail->nonRefundable());
  setCharAttr(attrs, _P2C, paxDetail->negotiatedWithCorp());
  setCharAttr(attrs, _P2B, paxDetail->negotiatedWithoutCorp());
  getAttr(attrs, _P2A, paxDetail->localCurrency());
  setIntAttr(attrs, _Q0W, paxDetail->paxFarePassengerNumber());
  getAttr(attrs, _S02, paxDetail->tourCodeDescription());
  setMonayAmountAttr(attrs, _C62, paxDetail->netFareAmount());
  setCharAttr(attrs, _N0C, paxDetail->tourIndicator());
  getAttr(attrs, _S01, paxDetail->textBox());
  setMonayAmountAttr(attrs, _N0B, paxDetail->netGross());
  setIntAttr(attrs, _C61, paxDetail->cat35CommissionPercent());
  setMonayAmountAttr(attrs, _C60, paxDetail->cat35CommissionAmount());
  getAttr(attrs, _N0A, paxDetail->bspMethodType());
  getAttr(attrs, _QV0, paxDetail->cat35Warning());
  setCharAttr(attrs, _P26, paxDetail->cat35Used());
  setCharAttr(attrs, _P28, paxDetail->overrideCat35());
  setCharAttr(attrs, _P29, paxDetail->ticketRestricted());
  setCharAttr(attrs, _P2E, paxDetail->paperTicketSurchargeMayApply());
  setCharAttr(attrs, _P2D, paxDetail->paperTicketSurchargeIncluded());
  getAttr(attrs, _S84, paxDetail->wpnDetails());
  setIntAttr(attrs, _Q4P, paxDetail->wpnOptionNumber());
  getAttr(attrs, _S85, paxDetail->accTvlData());
  setCharAttr(attrs, _PBS, paxDetail->reqAccTvl());
  setIntAttr(attrs, _C71, paxDetail->transferCount());
  setMonayAmountAttr(attrs, _C70, paxDetail->transferCharges());
  getAttr(attrs, _C72, paxDetail->transferPubCurrency());
  getAttr(attrs, _C73, paxDetail->stopOverPubCurrency());
  setMonayAmountAttr(attrs, _C56, paxDetail->totalPerPaxPlusImposedSrg());
  setMonayAmountAttr(attrs, _C6L, paxDetail->totalTaxesPlusImposedSrg());
  getAttr(attrs, _S86, paxDetail->baggageResponse());
  getAttr(attrs, _VCL, paxDetail->vclInfo());

  dynamic_cast<PricingDetailTrx*>(_pricingTrx)->paxDetails().push_back(paxDetail);

  onStartITN(attrs);
  _itin->validatingCarrier() = _tktFeesRequest->validatingCarrier();
  _tktFeesRequest->validatingCarrier() = EMPTY_STRING();
  _forbidCreditCard = attrs.get(_FCC, false);
}

void
TicketingFeesRequestHandlerWPA::onStartDCX(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter DCX");
  getAttr(attrs, _B00, _tktFeesRequest->validatingCarrier());
}

void
TicketingFeesRequestHandlerWPA::onStartSUM(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter SUM");
  _isInSum = true;

  getAttr(attrs, _B00, _tktFeesRequest->validatingCarrier());

  LocCode& ticketPointOverride = _pricingTrx->getRequest()->ticketPointOverride();
  if (ticketPointOverride.empty())
    getAttr(attrs, _AF0, ticketPointOverride);

  if (!_isTicketingDtInitialized)
    initTicketDate(attrs);
}

void
TicketingFeesRequestHandlerWPA::onStartCAL(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter CAL");

  const bool isFareCompWithSideTrip = attrs.get<bool>(_P2N);

  if (isFareCompWithSideTrip && _fbiWithSideTrip)
  {
    _tktFbi = _fbiWithSideTrip;
    _fbiWithSideTrip = nullptr;
  }
  else
  {
    auto& fbInfos = _tktFeesRequest->tktFeesFareBreakPerItin()[_itin];
    _tktFbi = _pricingTrx->dataHandle().create<Fbi>();
    _tktFbi->fareComponentID() = fbInfos.size() + 1;
    _tktFbi->setForbidCreditCard(_forbidCreditCard);
    fbInfos.push_back(_tktFbi);
  }

  getAttr(attrs, _B02, _tktFbi->governingCarrier());
  getAttr(attrs, _B50, _tktFbi->fareBasis());

  if (isFareCompWithSideTrip && !_fbiWithSideTrip)
    _fbiWithSideTrip = _tktFbi;
}

void
TicketingFeesRequestHandlerWPA::onStartERD(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter ERD");

  // BE0 - Ticket Desginator
  TktDesignator ticketDesignator;
  getAttr(attrs, _BE0, ticketDesignator);
  if (!ticketDesignator.empty())
  {
    _tktFeesRequest->tktDesignatorPerItin()[_itin].insert(
        std::pair<int16_t, TktDesignator>(_currentTvlSeg->segmentOrder(), ticketDesignator));
  }
}

void
TicketingFeesRequestHandlerWPA::onStartITN(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter ITN");
  createITN(attrs);

  addPaxTypePayment();
}



void
TicketingFeesRequestHandlerWPA::onStartSEG(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter SEG");

  if (!attrs.has(_B00))
  {
    parseArunk(attrs);
    return;
  };

  _currentTvlSeg = _pricingTrx->dataHandle().create<AirSeg>();
  _arunk = nullptr;
  _currentTvlSeg->pnrSegment() = _currentTvlSeg->segmentOrder() = _segmentOrder;
  parseSegmentOrgDst(attrs, *_currentTvlSeg);
  parseCarriersAndStatus(attrs, *_currentTvlSeg);
  updateFareData(attrs, *_currentTvlSeg);
  determineStopConx(attrs, *_currentTvlSeg);
  onStartERD(attrs);
}

void
TicketingFeesRequestHandlerWPA::addPaxTypePayment()
{
  TktFeesRequest::PaxTypePayment* ptp;
  _pricingTrx->dataHandle().get(ptp);
  if (ptp == nullptr)
    throw std::runtime_error("Null pointer to PTP data");

  _tktFeesRequest->paxTypePaymentPerItin()[_itin] = ptp;

  TktFeesRequest::PassengerPaymentInfo* ppi(nullptr);
  _pricingTrx->dataHandle().get(ppi);
  ptp->ppiV().push_back(ppi);

  const PaxDetail& paxDetail = *dynamic_cast<PricingDetailTrx*>(_pricingTrx)->paxDetails().back();
  ptp->currency() = paxDetail.equivalentCurrencyCode() != CurrencyCode()
                        ? paxDetail.equivalentCurrencyCode()
                        : paxDetail.baseCurrencyCode();
  ptp->amount() = paxDetail.totalPerPassenger();

  ptp->tktOverridePoint() = _pricingTrx->getRequest()->ticketPointOverride();
}

void
TicketingFeesRequestHandlerWPA::addFopBin(const std::string& bin)
{
  TktFeesRequest::FormOfPayment* fop(nullptr);
  _pricingTrx->dataHandle().get(fop);
  if (fop == nullptr)
    throw std::runtime_error("Null pointer to FOP data");
  fop->fopBinNumber() = bin;

  if (_chargeAmount.empty())
    fop->chargeAmountInRequest() = false;
  else
  {
    fop->chargeAmount() = atof(_chargeAmount.c_str());
    _tktFeesRequest->paymentAmountFop() = atof(_chargeAmount.c_str());
  }

  for (auto itin : _pricingTrx->itin())
  {
    TktFeesRequest::PaxTypePayment* ptp =
        _tktFeesRequest->paxTypePaymentPerItin()[itin];
    if (ptp == nullptr)
      throw std::runtime_error("Null pointer to PTP data");
    std::vector<TktFeesRequest::PassengerPaymentInfo*>& ppiV = ptp->ppiV();
    if (ppiV.empty())
      throw std::runtime_error("Empty PassengerPaymentInfoVector");
    ppiV.back()->fopVector().push_back(fop);
  }
}

void
TicketingFeesRequestHandlerWPA::onStartOBF(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter OBF");

  std::string bin, bin2;
  getAttr(attrs, _FOP, bin);
  getAttr(attrs, _FP2, bin2);
  std::string residualInd;
  getAttr(attrs, _PRS, residualInd);

  _tktFeesRequest->collectOBFee() = 'T';
  std::string obFeesStatus;
  getAttr(attrs, _OFS, obFeesStatus);
  if (!obFeesStatus.empty())
  {
    switch (obFeesStatus.at(0))
    {
      case 'N':
        _tktFeesRequest->collectOBFee() = 'F';
        break;
      case 'O':
        propagateAllOpenSegToAllItins();
        break;
      case 'U':
        propageteUnconfirmedSeg();
        break;
    }
  }

  parseAccCorpId(attrs, _SMX, _tktFeesRequest->accCodeVec());
  parseAccCorpId(attrs, _ACC, _tktFeesRequest->corpIdVec());
  propagateAccCorpIdToAllItins();

  _tktFeesRequest->formOfPayment() = bin;
  _tktFeesRequest->secondFormOfPayment() = bin2;
  _tktFeesRequest->chargeResidualInd() = (residualInd == "T");
  _tktFeesRequest->setCollectRTypeOBFee(false);
  _tktFeesRequest->setCollectTTypeOBFee(false);

  parseFormsOfPayment(attrs);

  getAttr(attrs, _PAT, _chargeAmount);
  if (0.0 == atof(_chargeAmount.c_str()))
    _chargeAmount = EMPTY_STRING();

  addFopBin(bin);
  if (!bin2.empty())
    addFopBin(bin2);
}

void
TicketingFeesRequestHandlerWPA::onEndPXI()
{
  if (!_isInSum)
    return;
  addPaxType();
  doItinPostProcessing();
  onEndITN();
  _itin = nullptr;
  _isInSum = false;
  LOG4CXX_DEBUG(_logger, "Leave PXI");
}

void
TicketingFeesRequestHandlerWPA::onEndITN()
{
  processOnEndITN();
  LOG4CXX_DEBUG(_logger, "Leave ITN");
}

void
TicketingFeesRequestHandlerWPA::onEndSEG()
{
  TravelSeg* ts =
      _currentTvlSeg ? static_cast<TravelSeg*>(_currentTvlSeg) : static_cast<TravelSeg*>(_arunk);
  TSE_ASSERT(ts);

  _itin->travelSeg().push_back(ts);
  _pricingTrx->travelSeg().push_back(ts);
  ++_segmentOrder;

  if (_currentTvlSeg)
    _prevTvlSeg = _currentTvlSeg;

  LOG4CXX_DEBUG(_logger, "Leave SEG");
}

void
TicketingFeesRequestHandlerWPA::checkFBIData(const IAttributes& attrs) const
{
  std::vector<int> requredAttrNames;
  requredAttrNames += _B02, _B50;
  checkData(attrs, requredAttrNames, *_TicketingFeesAttributeNames, "ATTRIBUTE IN FBI SECTION");
}

void
TicketingFeesRequestHandlerWPA::checkFLIData(const IAttributes& attrs) const
{
  std::vector<int> reqFLIAttrs;
  reqFLIAttrs += _A11, _A02, _B00;
  checkData(attrs, reqFLIAttrs, *_TicketingFeesAttributeNames, "ATTRIBUTE IN FLI SECTION");
}

void
TicketingFeesRequestHandlerWPA::setMonayAmountAttr(const IAttributes& attrs,
                                                   const int idx,
                                                   MoneyAmount& monayAmount)
{
  if (attrs.has(idx))
  {
    std::string str;
    getAttr(attrs, idx, str);
    monayAmount = (MoneyAmount)atof(str.c_str());
  }
}

void
TicketingFeesRequestHandlerWPA::setIntAttr(const IAttributes& attrs, const int idx, uint16_t& val)
{
  if (attrs.has(idx))
  {
    std::string str;
    getAttr(attrs, idx, str);
    val = atoi(str.c_str());
  }
}

void
TicketingFeesRequestHandlerWPA::setCharAttr(const IAttributes& attrs, const int idx, char& val)
{
  if (attrs.has(idx))
  {
    std::string str;
    getAttr(attrs, idx, str);

    if (!str.empty())
      val = str.c_str()[0];
  }
}

void
TicketingFeesRequestHandlerWPA::initTicketDate(const IAttributes& sumAttrs)
{
  std::string ticketDtStr;
  getAttr(sumAttrs, _D07, ticketDtStr);
  DateTime ticketDt(DateTime::localTime());

  if (!ticketDtStr.empty())
  {
    ticketDt = convertDate(ticketDtStr);
    ticketDt.addMinutes(sumAttrs.get<int>(_D54));
  }

  setDate(ticketDt);
  _isTicketingDtInitialized = true;
}

void
TicketingFeesRequestHandlerWPA::parseSegmentOrgDst(const IAttributes& attrs, TravelSeg& ts)
{
  getAttr(attrs, _A11, ts.boardMultiCity());
  getAttr(attrs, _C6I, ts.origAirport());
  getAttr(attrs, _A12, ts.offMultiCity());
  getAttr(attrs, _A02, ts.destAirport());

  if (ts.origAirport().empty() || ts.destAirport().empty())
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID - MISSING CITY");

  DataHandle& dh = _pricingTrx->dataHandle();
  ts.origin() = dh.getLoc(ts.origAirport(), _pricingTrx->ticketingDate());
  ts.destination() = dh.getLoc(ts.destAirport(), _pricingTrx->ticketingDate());

  if (!ts.origin() || !ts.destination())
    throw ErrorResponseException(ErrorResponseException::INVALID_CITY_AIRPORT_CODE);

  std::string depDate = attrs.get<std::string>(_D71);
  ts.departureDT() = !depDate.empty() ? convertDate(depDate) : DateTime::emptyDate();
  ts.arrivalDT() = ts.departureDT();
}

void
TicketingFeesRequestHandlerWPA::parseArunk(const IAttributes& attrs)
{
  _currentTvlSeg = nullptr;
  _arunk = _pricingTrx->dataHandle().create<ArunkSeg>();
  _arunk->segmentType() = Arunk;
  _arunk->pnrSegment() = _arunk->segmentOrder() = _segmentOrder;
  parseSegmentOrgDst(attrs, *_arunk);
  updateFareData(attrs, *_arunk);
}

void
TicketingFeesRequestHandlerWPA::doItinPostProcessing()
{
  TSE_ASSERT(_itin);

  for (size_t i = 0; i + 1 < _itin->travelSeg().size(); ++i)
  {
    TravelSeg& ts = *_itin->travelSeg()[i];
    if (ts.segmentType() == Arunk)
      ts.arrivalDT() = _itin->travelSeg()[i + 1]->departureDT();
  }
}

void
TicketingFeesRequestHandlerWPA::parseAccCorpId(const IAttributes& attrs,
                                               const int idx,
                                               std::vector<std::string>& out)
{
  std::string accCorpId;
  getAttr(attrs, idx, accCorpId);
  if (!accCorpId.empty())
    boost::split(out, accCorpId, boost::is_any_of(DELIMITER));
}

void
TicketingFeesRequestHandlerWPA::propagateAccCorpIdToAllItins()
{
  for (Itin* itin : _pricingTrx->itin())
  {
    _tktFeesRequest->accountCodeIdPerItin()[itin] = _tktFeesRequest->accCodeVec();
    _tktFeesRequest->corpIdPerItin()[itin] = _tktFeesRequest->corpIdVec();
  }
}

void
TicketingFeesRequestHandlerWPA::propageteUnconfirmedSeg()
{
  for (Itin* itin : _pricingTrx->itin())
    for (TravelSeg* tvlSeg : itin->travelSeg())
      tvlSeg->resStatus() = EMPTY_STRING();
}

void
TicketingFeesRequestHandlerWPA::propagateAllOpenSegToAllItins()
{
  for (Itin* itin : _pricingTrx->itin())
    for (TravelSeg* travelSeg : itin->travelSeg())
        travelSeg->segmentType() = tse::Open;
}

void
TicketingFeesRequestHandlerWPA::parseFormsOfPayment(const IAttributes& attrs)
{
  std::string fops;
  getAttr(attrs, _P6X, fops);
  if (!fops.empty())
  {
    _tktFeesRequest->formOfPaymentCash() = fops.find(CASH) != std::string::npos ? 'T' : 'F';
    _tktFeesRequest->formOfPaymentCard() = fops.find(CREDIT_CARD) != std::string::npos ? 'T' : 'F';
    _tktFeesRequest->formOfPaymentCheck() = fops.find(CHECK) != std::string::npos ? 'T' : 'F';
    _tktFeesRequest->formOfPaymentGTR() = fops.find(GTR) != std::string::npos ? 'T' : 'F';
  }
}

} // namespace tse
