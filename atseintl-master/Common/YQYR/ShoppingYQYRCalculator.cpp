#include "Common/YQYR/ShoppingYQYRCalculator.h"

#include "Common/YQYR/YQYRUtils.h"
#include "DataModel/FareUsage.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/YQYRFees.h"
#include "Util/BranchPrediction.h"

namespace tse
{
namespace YQYR
{
ShoppingYQYRCalculator::ShoppingYQYRCalculator(ShoppingTrx& trx) : _trx(trx)
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic818 &&
      _trx.diagnostic().diagParamIsSet("DD", "SHOPPING"))
    _dc.reset(new DiagCollectorShopping(_trx));
}

bool
ShoppingYQYRCalculator::isFilteredOutByCarrierDiag(const CarrierCode validatingCarrier) const
{
  if (LIKELY(!_trx.isValidatingCxrGsaApplicable() ||
             _trx.diagnostic().diagnosticType() != Diagnostic827))
    return false;

  const std::string& diagCarrier = _trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CARRIER);
  if (!diagCarrier.empty() && diagCarrier != validatingCarrier)
    return true;

  std::ostringstream stream;
  stream << "\n**************************************************************\n"
         << "VALIDATING CARRIER: " << validatingCarrier << "\n"
         << "\n**************************************************************\n";
  _trx.diagnostic().insertDiagMsg(stream.str());

  return false;
}

void
ShoppingYQYRCalculator::appendFilter(YQYRFilter* filter)
{
  if (_filters.append(filter))
    updateValidations(filter->replacesValidation());
}

void
ShoppingYQYRCalculator::prependFilter(YQYRFilter* filter)
{
  if (_filters.prepend(filter))
    updateValidations(filter->replacesValidation());
}

void
ShoppingYQYRCalculator::setClassifier(YQYRClassifier* classifier)
{
  _classifier = classifier;
  updateValidations(classifier->replacesValidation());
}

void
ShoppingYQYRCalculator::initDefaultFilters()
{
  appendFilter(&_trx.dataHandle().safe_create<YQYRFilterPointOfSale>(_trx));
  appendFilter(&_trx.dataHandle().safe_create<YQYRFilterPointOfTicketing>(_trx));
  appendFilter(&_trx.dataHandle().safe_create<YQYRFilterAgencyPCC>(_trx));
  appendFilter(&_trx.dataHandle().safe_create<YQYRFilterTicketingDate>(_trx));
}

const CarrierStorage*
ShoppingYQYRCalculator::getCarrierStorage(const CarrierCode carrier)
{
  FeesPerCarrierMap::const_iterator feesForCarrier(_feesPerCarrier.find(carrier));
  if (feesForCarrier == _feesPerCarrier.end())
  {
    if (UNLIKELY(_dc.get()))
      _dc->diagStream() << " - NO FEES FOR CARRIER " << carrier << " GOING TO FETCH DATA FROM DB"
                        << std::endl;

    readRecords(carrier);
  }

  feesForCarrier = _feesPerCarrier.find(carrier);
  if (feesForCarrier == _feesPerCarrier.end() || feesForCarrier->second._feesPerCode.empty())
  {
    if (UNLIKELY(_dc.get()))
      _dc->diagStream() << " - COULD NOT FIND ANY FEES FOR THE " << carrier << " CARRIER"
                        << std::endl;
    return nullptr;
  }

  return &feesForCarrier->second;
}

const CarrierStorage&
ShoppingYQYRCalculator::getCarrierStorage(FareMarket* fareMarket,
                                          PaxTypeBucket& cortege,
                                          const CarrierCode carrier)
{
  static const CarrierStorage EMPTY_STORAGE;

  CarrierStorage& cortegeStorage(cortege.getYqyrCarrierStorage(carrier));
  if (cortegeStorage.isInitialized())
  {
    if (UNLIKELY(_dc.get()))
      _dc->diagStream() << "- USING FEES STORAGE FROM PREFILTERED FARE MARKET "
                        << fareMarket->toString() << std::endl;

    return cortegeStorage;
  }

  if (const CarrierStorage* storage = getCarrierStorage(carrier))
    return *storage;

  return EMPTY_STORAGE;
}

bool
ShoppingYQYRCalculator::calculateYQYRsOnBucket(
    const uint32_t bucketIndex,
    const CarrierCode validatingCarrier,
    const StdVectorFlatSet<CarrierCode>& carriersToProcess,
    FareMarket* fareMarket,
    PaxTypeBucket& cortege,
    FarePath* farePath,
    const YQYRCalculator::FareGeography& geography,
    YQYRCalculator::YQYRFeesApplicationVec& results)
{
  if (UNLIKELY(_dc.get()))
  {
    DiagStream stream(_dc->diagStream());
    stream << " - CARRIERS TO PROCESS: [";
    for (const CarrierCode carrier : carriersToProcess)
      stream << carrier.c_str() << ", ";
    stream << "]" << std::endl;
  }

  OldCalcAdapter feeApplier(
      _trx, farePath, geography, _validations, results, carriersToProcess.container(), _dc.get());

  for (const CarrierCode carrier : carriersToProcess)
  {
    const CarrierStorage& carrierStorage(getCarrierStorage(fareMarket, cortege, carrier));
    if (!carrierStorage.isInitialized())
      continue;

    for (const FeeStorage& storage : carrierStorage._feesPerCode)
      storage.processBucket(bucketIndex, carrier, feeApplier);
  }

  feeApplier.applyFees(validatingCarrier);

  return true;
}

void
ShoppingYQYRCalculator::updateValidations(const Validations::Validation validation)
{
  if (validation != Validations::NONE)
    _validations.skipValidation(validation);
}

void
ShoppingYQYRCalculator::readRecords(const CarrierCode carrier)
{
  if (UNLIKELY(_dc.get()))
    _dc->diagStream() << " - READING RECORDS FOR CARRIER " << carrier << std::endl;
  const std::vector<YQYRFees*>& yqyrs = _trx.dataHandle().getYQYRFees(carrier);

  CarrierStorage& carrierStorage = _feesPerCarrier[carrier];

  if (yqyrs.empty())
  {
    if (UNLIKELY(_dc.get()))
      _dc->diagStream() << " - NO APPLICABLE RECORDS FOUND IN DB" << std::endl;
    return;
  }

  std::vector<FeeStorage>& feesPerCode = carrierStorage._feesPerCode;
  feesPerCode.reserve(4);
  feesPerCode.push_back(FeeStorage(_originalBuckets));
  if (UNLIKELY(_dc.get()))
    _dc->diagStream() << " - CREATED NEW FEE BUCKET" << std::endl;

  const YQYRFees* previousFee = nullptr;
  for (const YQYRFees* fee : yqyrs)
  {
    if (_filters.isFilteredOut(fee, _dc.get()))
      continue;

    feesPerCode.back().addFee(fee, *_classifier, _dc.get());

    if (previousFee &&
        (previousFee->taxCode() != fee->taxCode() || previousFee->subCode() != fee->subCode()))
    {
      if (UNLIKELY(_dc.get()))
        _dc->diagStream() << " - CREATED NEW BUCKET" << std::endl;
      feesPerCode.push_back(FeeStorage(_originalBuckets));
    }

    previousFee = fee;
  }
}

ShoppingYQYRCalculator::ConcurContext
ShoppingYQYRCalculator::getConcur(const CarrierCode carrier)
{
  const auto it = _concurringCarriers.find(carrier);
  if (it != _concurringCarriers.end())
    return it->second;

  ConcurContext concurr;

  const std::vector<YQYRFeesNonConcur*>& concurringCarriersVec =
      _trx.dataHandle().getYQYRFeesNonConcur(carrier, _trx.getRequest()->ticketingDT());
  if (!concurringCarriersVec.empty())
  {
    concurr.concurRecord = concurringCarriersVec.front();

    if (concurr.concurRecord->taxCarrierApplTblItemNo())
      concurr.taxCarrierAppl = _trx.dataHandle().getTaxCarrierAppl(
          concurr.concurRecord->vendor(), concurr.concurRecord->taxCarrierApplTblItemNo());
  }

  _concurringCarriers.insert(std::make_pair(carrier, concurr));
  return concurr;
}

StdVectorFlatSet<CarrierCode>
ShoppingYQYRCalculator::determineCarriersToProcess(const Itin* itin,
                                                   const CarrierCode validatingCarrier)
{
  StdVectorFlatSet<CarrierCode> carriersInItin, carriersToProcess;

  for (const TravelSeg* seg : itin->travelSeg())
  {
    const AirSeg* airSeg(seg->toAirSeg());
    if (!airSeg)
      continue;

    carriersInItin.insert(airSeg->marketingCarrierCode());
  }

  const ConcurContext concurr = getConcur(validatingCarrier);

  if (!concurr.concurRecord)
  {
    if (carriersInItin.find(validatingCarrier) != carriersInItin.end())
      carriersToProcess.insert(validatingCarrier);
    return carriersToProcess;
  }

  if (concurr.concurRecord->selfAppl() != 'X' &&
      carriersInItin.find(validatingCarrier) != carriersInItin.end())
    carriersToProcess.insert(validatingCarrier);

  for (const CarrierCode carrier : carriersInItin)
  {
    if (!YQYRCalculator::validateT190(concurr.taxCarrierAppl, carrier))
      continue;

    carriersToProcess.insert(carrier);
  }

  return carriersToProcess;
}

OldCalcAdapter::OldCalcAdapter(ShoppingTrx& trx,
                               FarePath* farePath,
                               const YQYRCalculator::FareGeography& geography,
                               const Validations validations,
                               YQYRCalculator::YQYRFeesApplicationVec& results,
                               const std::vector<CarrierCode>& concurringCarriers,
                               DiagCollectorShopping* dc)
  : _trx(trx),
    _fare(farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare()),
    _farePath(farePath),
    _results(results),
    _dc(dc),
    _yqyrCalc(trx,
              _farePath,
              geography,
              validations,
              _trx.journeyItin()->calculationCurrency(),
              concurringCarriers,
              YQYRUtils::printDiag(_trx))
{
}

void
OldCalcAdapter::matchFees(const CarrierCode carrier, const std::vector<const YQYRFees*>& fees)
{
  YQYRCalculator::FeeListT feesToProcess(_yqyrCalc.allocateFeeList());
  std::copy(fees.begin(), fees.end(), std::back_inserter(feesToProcess));

  try
  {
    _yqyrCalc.processCarrier(carrier, feesToProcess);
  }
  catch (const YQYRCalculator::TPMemoryOverused&)
  {
    // does nothing
  }
}
}
}
