//-------------------------------------------------------------------
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

#include "DataModel/ItinIndex.h"

#include "Common/Assert.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"

namespace tse
{

static Logger
ItinIndexlogger("atseintl.DataModel.ItinIndex");

static Logger
logger("atseintl.DataModel.ItinIndex.ItinIndexIterator");

//------------------------------------------------------------------------------
// This constructor used for the end of the iterator path across
// the itin index container
//-------------------------------------------------------------------
ItinIndex::ItinIndexIterator::ItinIndexIterator(IteratorMode mode) : _mode(mode)
{
  invalidate();
}

ItinIndex::ItinIndexIterator::ItinIndexIterator(const ItinIndex::Key& rowKey,
                                                ItinIndex::ItinRowCellVector* cells)
  : _rowKey(rowKey), _cellVector(cells)
{
  initialize();
}

ItinIndex::ItinIndexIterator::ItinIndexIterator(const ItinIndex::Key& rowKey,
                                                ItinIndex::ItinRowCellVector* cells,
                                                const uint32_t& atIndex)
  : _rowKey(rowKey), _cellVector(cells)
{
  initialize(atIndex);
}

ItinIndex::ItinIndexIterator::ItinIndexIterator(const ItinIndex::Key& rowKey,
                                                const uint32_t& acrossLegIndex,
                                                ShoppingTrx* shoppingTrx)
  : _mode(IteratorModeAcrossLeg),
    _acrossLegIndex(acrossLegIndex),
    _rowKey(rowKey),
    _shoppingTrx(shoppingTrx)
{
  initializeAcrossLeg();
}

ItinIndex::ItinIndexIterator::ItinIndexIterator(const ItinIndex::Key& rowKey,
                                                const uint32_t& acrossLegIndex,
                                                ShoppingTrx* shoppingTrx,
                                                const uint32_t& atIndex)
  : _mode(IteratorModeAcrossLeg),
    _acrossLegIndex(acrossLegIndex),
    _rowKey(rowKey),
    _shoppingTrx(shoppingTrx)
{
  initializeAcrossLeg(atIndex);
}

void
ItinIndex::ItinIndexIterator::invalidate()
{
  _valid = false;
  _bitIndex = ItinIndexIterator::INVALID_BITINDEX;

  if (_mode == ItinIndex::ItinIndexIterator::IteratorModeAcrossLeg)
  {
    if (!_tSegHolder.empty() && (_cell != nullptr))
    {
      // Restore the across stop over leg original travel seg poiners
      Itin*& itin = _cell->second;
      FareMarket*& fm = itin->fareMarket().front();
      std::vector<TravelSeg*>& iTSegs = itin->travelSeg();
      std::vector<TravelSeg*>& fTSegs = fm->travelSeg();

      itin->travelSeg().clear();
      itin->travelSeg().reserve(_tSegHolder.size());
      fm->travelSeg().clear();
      fm->travelSeg().reserve(_tSegHolder.size());

      iTSegs.insert(iTSegs.end(), _tSegHolder.begin(), _tSegHolder.end());
      fTSegs.insert(fTSegs.end(), _tSegHolder.begin(), _tSegHolder.end());

      // Set the cell to null and empty the tSegHolder
      _cell = nullptr;
      _tSegHolder.clear();
    }
  }
}

void
ItinIndex::ItinIndexIterator::prepareCell(uint32_t atIndex)
{
  if (UNLIKELY(!_valid))
  {
    LOG4CXX_ERROR(logger, "Iterator is not valid!");
    return;
  }

  if (UNLIKELY((atIndex > 0) && (atIndex < _numberBits)))
  {
    _bitIndex = atIndex;
  }

  _cell = &(_cellVector->operator[](_bitIndex));
  _totalTravelSegSize = static_cast<uint32_t>(_cell->second->travelSeg().size());
}

void
ItinIndex::ItinIndexIterator::generateItinSet()
{
  // Get combination table data
  IndexVectors& comboTable = _acrossStopOverCombinations->operator[](_rowKey);
  IndexVectorsIterator iVectIter = comboTable.begin();
  IndexVectorsIterator iVectEIter = comboTable.end();

  // Reset total travel seg size
  _totalTravelSegSize = 0;

  // Generate itin set
  for (uint32_t n = 0; iVectIter != iVectEIter; ++iVectIter, ++n)
  {
    uint32_t& jumpedLegIdx = _jumpedLegs->operator[](n);
    if (jumpedLegIdx == ASOLEG_SURFACE_SECTOR_ID)
    {
      IndexPair& iPair = _currentItinSet[n];
      iPair.first = _rowKey;
      iPair.second = ASOLEG_SURFACE_SECTOR_ID;
      _currentSopSet[n] = ASOLEG_SURFACE_SECTOR_ID;
      _totalTravelSegSize += 1;
      continue;
    }

    IndexVector& curIdxVector = *iVectIter;

    if (UNLIKELY(curIdxVector.empty() || curIdxVector.size() == 0))
    {
      LOG4CXX_ERROR(logger, "cur index vector is empty");
      invalidate();
      return;
    }
    uint32_t& curIdxVal = curIdxVector[_bitIndex];

    ShoppingTrx::Leg& curJumpedLeg = _shoppingTrx->legs()[jumpedLegIdx];

    ItinIndex::ItinRowCellMap& curRowCellMap = curJumpedLeg.carrierIndex().rowCellMap();

    // We can only go through the row key specified carrier SOPs
    if (jumpedLegIdx == _adoptedLegIndex)
    {
      ItinIndex::ItinRowCellVector& curCellVector = curRowCellMap[_rowKey];
      IndexPair& iPair = _currentItinSet[n];
      iPair.first = _rowKey;
      iPair.second = curIdxVal;
      ItinIndex::ItinCell& curCell = curCellVector[curIdxVal];
      _currentSopSet[n] = curCell.first.sopIndex();
      _totalTravelSegSize += static_cast<uint32_t>(curCell.second->travelSeg().size());
    }
    else
    {
      // this is a non-adopted leg. We simply look up the sop by its index
      TSE_ASSERT(curIdxVal < _shoppingTrx->legs()[jumpedLegIdx].sop().size());
      const ShoppingTrx::SchedulingOption& sop =
          _shoppingTrx->legs()[jumpedLegIdx].sop()[curIdxVal];

      // now we must find where the sop is in the itin index, because
      // we need to record its location.
      ItinIndex::Key cxr;
      ShoppingUtil::createKey(sop.governingCarrier(), cxr);

      ItinIndex::ItinRowCellMapIterator itor = curRowCellMap.find(cxr);
      TSE_ASSERT(itor != curRowCellMap.end());

      // now we do a linear search for the sop with the sop index
      // that we're looking for.
      for (ItinRowCellVector::const_iterator i = itor->second.begin(); i != itor->second.end(); ++i)
      {
        if (i->first.sopIndex() == curIdxVal)
        {
          IndexPair& iPair = _currentItinSet[n];
          iPair.first = cxr;
          iPair.second = static_cast<uint32_t>(i - itor->second.begin());
          _currentSopSet[n] = curIdxVal;
          _totalTravelSegSize += static_cast<uint32_t>(i->second->travelSeg().size());
          break;
        }
      }
    }
  }
}

void
ItinIndex::ItinIndexIterator::prepareAcrossLegCell(uint32_t atIndex)
{
  if (UNLIKELY(!_valid))
  {
    return;
  }

  if (UNLIKELY((atIndex != 0) && (atIndex < _numberBits)))
  {
    _bitIndex = atIndex;
  }

  // Clear travel seg pointers from the Itin object
  // that represents the across stop over leg
  _cell->second->travelSeg().clear();
  _cell->second->fareMarket().front()->travelSeg().clear();

  // Generate bit index configured set of itineraries
  // that compose this variation of across stop over leg
  generateItinSet();

  // Distribute the itins gathered by pushing their travel segs
  // onto the travel seg vector of the across stop over leg cell
  Itin*& itin = _cell->second;
  FareMarket*& fm = itin->fareMarket().front();
  std::vector<TravelSeg*>& tSegRef = itin->travelSeg();
  std::vector<TravelSeg*>& fSegRef = fm->travelSeg();

  tSegRef.reserve(_totalTravelSegSize);
  fSegRef.reserve(_totalTravelSegSize);
  IndexPairVectorIterator iCIter = _currentItinSet.begin();
  IndexPairVectorIterator iCEIter = _currentItinSet.end();

  // Insert all of the travel segs that compose the across stop
  // over leg itin in its current bit specified combination
  for (uint32_t n = 0; iCIter != iCEIter; ++iCIter, ++n)
  {
    uint32_t& jumpedLegIdx = _jumpedLegs->operator[](n);
    if (jumpedLegIdx == ASOLEG_SURFACE_SECTOR_ID)
    {
      tSegRef.push_back(_tSegHolder[n]);
      continue;
    }

    ShoppingTrx::Leg& curJumpedLeg = _shoppingTrx->legs()[jumpedLegIdx];
    ItinIndex::ItinRowCellMap& curRowCellMap = curJumpedLeg.carrierIndex().rowCellMap();
    IndexPair& curPair = *iCIter;
    ItinIndex::ItinRowCellVector& curCellVector = curRowCellMap[curPair.first];
    ItinIndex::ItinCell& curPairCell = curCellVector[curPair.second];
    std::vector<TravelSeg*>& curITSeg = curPairCell.second->travelSeg();

    if (UNLIKELY(IS_DEBUG_ENABLED(logger)))
    {
      LOG4CXX_DEBUG(logger, "Inserting the following travel segs into the main cell:");
      std::vector<TravelSeg*>::iterator curITIter = curITSeg.begin();
      std::vector<TravelSeg*>::iterator curITEIter = curITSeg.end();
      for (; curITIter != curITEIter; ++curITIter)
      {
        AirSeg* curASeg = dynamic_cast<AirSeg*>(*curITIter);
        SUPPRESS_UNUSED_WARNING(curASeg);
        LOG4CXX_DEBUG(logger,
                      "(" << curASeg->origin()->loc() << "-" << curASeg->destination()->loc()
                          << ")[" << curASeg->carrier() << "/" << curASeg->flightNumber() << "]");
      }
    }

    tSegRef.insert(tSegRef.end(), curITSeg.begin(), curITSeg.end());
  }

  // Assign these pointers to the fare market travel seg vector as well
  fSegRef.insert(fSegRef.end(), tSegRef.begin(), tSegRef.end());
}

void
ItinIndex::ItinIndexIterator::initialize(uint32_t atIndex)
{
  if (UNLIKELY(_cellVector == nullptr))
  {
    invalidate();
    LOG4CXX_ERROR(logger, "_cellVector is NULL");
    return;
  }

  _valid = true;
  _bitIndex = 0;
  _numberBits = static_cast<uint32_t>(_cellVector->size());

  prepareCell(atIndex);
}

void
ItinIndex::ItinIndexIterator::initializeAcrossLeg(uint32_t atIndex)
{
  if (_shoppingTrx == nullptr)
  {
    LOG4CXX_ERROR(logger, "Shopping transaction object pointer is null!");
    invalidate();
    return;
  }
  TSELatencyData metrics(*_shoppingTrx, "ITINIDXITER INITASO");

  _valid = true;
  std::vector<ShoppingTrx::Leg>& legs = _shoppingTrx->legs();

  // Check if the across leg index is valid
  if (_acrossLegIndex >= legs.size())
  {
    LOG4CXX_ERROR(logger, "Invalid across stop over leg index!");
    invalidate();
    return;
  }

  // Perform several operations before checking
  // if the iterator is valid
  ShoppingTrx::Leg& aCOLeg = legs[_acrossLegIndex];

  if (!aCOLeg.stopOverLegFlag())
  {
    LOG4CXX_ERROR(logger, "Initializing across leg iterator with normal leg data!");
    invalidate();
    return;
  }

  // Initialize main iterator elements
  _jumpedLegs = &(aCOLeg.jumpedLegIndices());
  _acrossStopOverCombinations = &(aCOLeg.acrossStopOverCombinations());
  _adoptedLegIndex = aCOLeg.adoptedCrossedLegRefIndex();
  _numberBits = aCOLeg.getFlightBitmapSize(*_shoppingTrx, _rowKey);
  _bitIndex = 0;
  _cell = aCOLeg.carrierIndex().retrieveTopItinCell(_rowKey, ItinIndex::CHECK_NOTHING);

  // Reserve proper space in current itin set and current sop set
  _currentItinSet.resize(_jumpedLegs->size());
  _currentSopSet.resize(_jumpedLegs->size());

  LOG4CXX_DEBUG(logger, "_adoptedLegIndex = " << _adoptedLegIndex);
  LOG4CXX_DEBUG(logger, "_numberBits      = " << _numberBits);

  // Check main factors for initialization failure
  if (_jumpedLegs == nullptr || _jumpedLegs->empty() || _jumpedLegs->size() == 0 || _cell == nullptr ||
      _numberBits < 1)
  {
    LOG4CXX_ERROR(logger, "Failed to initialize iterator!");
    invalidate();
    return;
  }

  // Preserve across leg travel segs in tSeg Holder
  _tSegHolder = _cell->second->travelSeg();

  // Make the preparation call
  prepareAcrossLegCell(atIndex);
}

void
ItinIndex::ItinIndexIterator::advance()
{
  if (UNLIKELY(!_valid))
  {
    return;
  }

  ++_bitIndex;

  if (_bitIndex == _numberBits)
  {
    invalidate();
    return;
  }

  if (_mode == IteratorModeNormalLeg)
  {
    prepareCell();
  }
  else
  {
    prepareAcrossLegCell();
  }
  logData("advance()");
}

ItinIndex::ItinIndexIterator&
ItinIndex::ItinIndexIterator::
operator++()
{
  advance();
  return (*this);
}

bool
ItinIndex::ItinIndexIterator::
operator==(const ItinIndex::ItinIndexIterator& rhs)
{
  if (_valid != rhs._valid)
  {
    return (false);
  }

  if (_bitIndex != rhs._bitIndex)
  {
    return (false);
  }

  return (true);
}

bool
ItinIndex::ItinIndexIterator::
operator!=(const ItinIndex::ItinIndexIterator& rhs)
{
  if (_valid == rhs._valid)
  {
    return (false);
  }

  if (UNLIKELY(_bitIndex == rhs._bitIndex))
  {
    return (false);
  }

  return (true);
}

void
ItinIndex::ItinIndexIterator::internalLog(const std::string& desc)
{
  LOG4CXX_DEBUG(logger, "********( BEG " << desc << ")********");
  LOG4CXX_DEBUG(logger, "- IsValid    = " << ((_valid) ? "true" : "false"));
  if (_valid)
  {
    LOG4CXX_DEBUG(logger, "- BitIndex   = " << _bitIndex);
    LOG4CXX_DEBUG(logger, "- NumberBits = " << _numberBits);
    LOG4CXX_DEBUG(logger, "- RowKey     = " << _rowKey);
    LOG4CXX_DEBUG(
        logger,
        "- Mode       = " << ((_mode == ItinIndex::ItinIndexIterator::IteratorModeNormalLeg)
                                  ? "Normal"
                                  : "Across"));
    LOG4CXX_DEBUG(logger, "CellInfo: ");
    Itin* itin = _cell->second;
    LOG4CXX_DEBUG(logger, "GovCxr  : " << itin->fareMarket().front()->governingCarrier());
    LOG4CXX_DEBUG(logger, "- Travel segs:");
    std::vector<TravelSeg*>& tSegs = itin->travelSeg();
    LOG4CXX_DEBUG(logger, "- Size = " << tSegs.size());
    std::vector<TravelSeg*>::iterator iTIter = tSegs.begin();
    std::vector<TravelSeg*>::iterator iTEIter = tSegs.end();

    std::ostringstream oStr;
    for (uint32_t n = 0; iTIter != iTEIter; ++iTIter, n++)
    {
      AirSeg* aSeg = dynamic_cast<AirSeg*>(*iTIter);
      if (aSeg->segmentType() == Arunk)
      {
        oStr << "[" << n << "] "
             << "ARUNK(" << aSeg->origin()->loc() << "-" << aSeg->destination()->loc() << ")";
      }
      else
      {
        oStr << "[" << n << "] " << aSeg->flightNumber() << "-" << aSeg->carrier() << "("
             << aSeg->origin()->loc() << "-" << aSeg->destination()->loc() << ")";
      }
    }
    LOG4CXX_DEBUG(logger, oStr.str());
  }
  LOG4CXX_DEBUG(logger, "********( END " << desc << " )********");
}

void
ItinIndex::ItinIndexIterator::logData(const char* desc)
{
  if (UNLIKELY(IS_DEBUG_ENABLED(logger)))
  {
    internalLog(std::string(((desc == nullptr) ? " " : desc)));
  }
}

void
ItinIndex::ItinIndexIterator::logData(const std::string& desc)
{
  if (IS_DEBUG_ENABLED(logger))
  {
    internalLog(desc);
  }
}

//-------------------------------------------------------------------
// Adds an itin row to the index
//-------------------------------------------------------------------
ItinIndex::ItinRow&
ItinIndex::addItinRow(const ItinIndex::Key& itinRowKey)
{
  return (_root[itinRowKey]); // lint !e1413
}

//-------------------------------------------------------------------
// Adds an itin column to the specified row
//-------------------------------------------------------------------
ItinIndex::ItinColumn&
ItinIndex::addItinColumn(ItinIndex::ItinRow& itinRow, const ItinIndex::Key& itinColumnKey)
{
  return (itinRow[itinColumnKey]); // lint !e1413
}

//-------------------------------------------------------------------
// Create a group key from a string
//-------------------------------------------------------------------
void
ItinIndex::createKey(const std::string& value, ItinIndex::Key& key)
{
  ItinIndex::KeyGenerator keyGenerator(tse::Global::hasherMethod());
  keyGenerator << value;
  key = keyGenerator.hash();
}

//-------------------------------------------------------------------
// Create a group key from a numeric value
//-------------------------------------------------------------------
void
ItinIndex::createKey(const uint32_t& value, ItinIndex::Key& key)
{
  key = static_cast<ItinIndex::Key>(value);
}

//-------------------------------------------------------------------
// Adds the specified itin
//-------------------------------------------------------------------
void
ItinIndex::addItinCell(Itin*& itin,
                       ItinCellInfo& itinCellInfo,
                       const ItinIndex::Key& itinRowKey,
                       const ItinIndex::Key& itinColumnKey)
{
  // Retrieve the itinRow object, the add operation
  // will add one if necessary, or retrieve the one specified
  ItinIndex::ItinRow& itinRow = addItinRow(itinRowKey);

  // Retrieve the itinColumn object, the add operation
  // will add one if necessary, or retrieve the one specified
  ItinIndex::ItinColumn& itinColumn = addItinColumn(itinRow, itinColumnKey);

  // Add the itinCell to the itinColumn object
  itinCellInfo.itinColumnIndex() = static_cast<uint32_t>(itinColumn.size());
  itinColumn.push_back(ItinIndex::ItinCell(itinCellInfo, itin));
  LOG4CXX_DEBUG(ItinIndexlogger, "- Pushed itin cell, itin ptr value = " << itin);

  // Add to row cell bases if the itinerary is not a "dummy" itin
  if (!(itinCellInfo.flags() & ItinIndex::ITININDEXCELLINFO_FAKEDIRECTFLIGHT))
  {
    // Construct the row cell base map as itins are added to the map
    ItinIndex::ItinRowCellVector& rowCellV = _rowCellMap[itinRowKey];
    rowCellV.push_back(ItinIndex::ItinCell(itinCellInfo, itin));
  }
}

ItinIndex::ItinCell*
ItinIndex::findCellLoop(ItinIndex::ItinRowIterator& iRowIter,
                        const ItinIndex::ItinRowIterator& iRowEIter,
                        const ItinIndex::Key& targetKey)
{
  for (; iRowIter != iRowEIter; ++iRowIter) // lint !e1561
  {
    ItinIndex::ItinColumn& curItinColumn = iRowIter->second;
    if (UNLIKELY((curItinColumn.empty()) || (curItinColumn.size() == 0)))
    {
      continue;
    }

    return (&(curItinColumn[targetKey]));
  }

  return (nullptr);
}

const ItinIndex::ItinCell*
ItinIndex::findCellLoop(ItinIndex::ItinRowConstIterator& iRowIter,
                        const ItinIndex::ItinRowConstIterator& iRowEIter,
                        const ItinIndex::Key& targetKey) const
{
  for (; iRowIter != iRowEIter; ++iRowIter)
  {
    const ItinIndex::ItinColumn& curItinColumn = iRowIter->second;
    if (UNLIKELY((curItinColumn.empty()) || (curItinColumn.size() == 0)))
    {
      continue;
    }

    return (&(curItinColumn[targetKey]));
  }

  return (nullptr);
}

ItinIndex::ItinCell*
ItinIndex::findCellInRow(ItinIndex::ItinRow& itinRow, const ItinIndex::ItinCheckType& checkType)
{
  ItinIndex::ItinRowIterator itinRowIter = itinRow.begin();
  ItinIndex::ItinRowIterator itinRowEndIter = itinRow.end();
  ItinIndex::Key targetKey;
  ItinIndex::ItinCell* rt = nullptr;
  createKey(ITININDEX_DEFAULT, targetKey);

  // Find the first valid "itinColumn" object
  rt = findCellLoop(itinRowIter, itinRowEndIter, targetKey);

  if ((checkType == CHECK_FAKEDIRECTFLIGHT) && (rt != nullptr) &&
      (rt->first.flags() & ItinIndex::ITININDEXCELLINFO_FAKEDIRECTFLIGHT))
  {
    // Continue searching where the last loop ended. By
    // using the same logic we will get the next itinerary
    ++itinRowIter;
    rt = findCellLoop(itinRowIter, itinRowEndIter, targetKey);
  }

  return (rt);
}

const ItinIndex::ItinCell*
ItinIndex::findCellInRow(const ItinIndex::ItinRow& itinRow,
                         const ItinIndex::ItinCheckType& checkType) const
{
  ItinIndex::ItinRowConstIterator itinRowIter = itinRow.begin();
  ItinIndex::ItinRowConstIterator itinRowEndIter = itinRow.end();
  ItinIndex::Key targetKey;
  const ItinIndex::ItinCell* rt = nullptr;
  targetKey = static_cast<ItinIndex::Key>(ITININDEX_DEFAULT);

  // Find the first valid "itinColumn" object
  rt = findCellLoop(itinRowIter, itinRowEndIter, targetKey);

  if (UNLIKELY((checkType == CHECK_FAKEDIRECTFLIGHT) && (rt != nullptr) &&
      (rt->first.flags() & ItinIndex::ITININDEXCELLINFO_FAKEDIRECTFLIGHT)))
  {
    // Continue searching where the last loop ended. By
    // using the same logic we will get the next itinerary
    ++itinRowIter;
    rt = findCellLoop(itinRowIter, itinRowEndIter, targetKey);
  }

  return (rt);
}

//-------------------------------------------------------------------
// Finds the direct itin for the carrier specified
// or the most direct itin available
//-------------------------------------------------------------------
ItinIndex::ItinCell*
ItinIndex::retrieveTopItinCell(const ItinIndex::Key& itinRowKey,
                               const ItinIndex::ItinCheckType& checkType)
{
  // Find the itinRow from the root with the specified key
  ItinIndex::ItinMatrixIterator rootIter = _root.find(itinRowKey);

  // Return false if the iterator is invalid
  if (UNLIKELY(rootIter == _root.end()))
  {
    return (nullptr);
  }

  // Get the itinRow that was found
  ItinIndex::ItinRow& itinRow = rootIter->second;

  // Return false if the itinRow is invalid
  if (UNLIKELY((itinRow.empty()) || (itinRow.size() == 0)))
  {
    return (nullptr);
  }

  return (findCellInRow(itinRow, checkType));
}

//-------------------------------------------------------------------
// Finds the direct itin for the carrier
// specified or the most direct itin available
//-------------------------------------------------------------------
const ItinIndex::ItinCell*
ItinIndex::retrieveTopItinCell(const ItinIndex::Key& itinRowKey,
                               const ItinIndex::ItinCheckType& checkType) const
{
  // Find the itinRow from the root with the specified key
  ItinIndex::ItinMatrixConstIterator rootIter = _root.find(itinRowKey);

  // Return false if the iterator is invalid
  if (UNLIKELY(rootIter == _root.end()))
  {
    return (nullptr);
  }

  // Get the itinRow that was found
  const ItinIndex::ItinRow& itinRow = rootIter->second;

  // Return false if the itinRow is invalid
  if (UNLIKELY((itinRow.empty()) || (itinRow.size() == 0)))
  {
    return (nullptr);
  }

  return (findCellInRow(itinRow, checkType));
}

//-------------------------------------------------------------------
// Retrieves an itin "row", which are itins grouped into "columns"
// for the specified carrier
//-------------------------------------------------------------------
const ItinIndex::ItinRow*
ItinIndex::retrieveItinRow(const ItinIndex::Key& itinRowKey) const
{
  // Find the root iterator to a itinRow based on the
  // specified key
  ItinIndex::ItinMatrixConstIterator rootIter = _root.find(itinRowKey);

  // If a match was found in the map
  if (LIKELY(rootIter != _root.end()))
  {
    return (static_cast<const ItinIndex::ItinRow*>(&(rootIter->second)));
  }

  return (nullptr);
}

//-------------------------------------------------------------------
// Calculates the number of cells in the entire index
//-------------------------------------------------------------------
uint32_t
ItinIndex::retrieveTotalNumberCells() const
{
  if (_root.empty())
  {
    return (0);
  }

  uint32_t rt = 0;
  ItinIndex::ItinMatrixConstIterator iMIter = _root.begin();
  ItinIndex::ItinMatrixConstIterator iMEIter = _root.end();

  for (; iMIter != iMEIter; ++iMIter)
  {
    const ItinIndex::ItinRow& iR = static_cast<const ItinIndex::ItinRow&>(iMIter->second);

    if (iR.empty())
    {
      continue;
    }

    ItinIndex::ItinRowConstIterator iRIter = iR.begin();
    ItinIndex::ItinRowConstIterator iREIter = iR.end();
    for (; iRIter != iREIter; ++iRIter)
    {
      const ItinIndex::ItinColumn& iC = static_cast<const ItinIndex::ItinColumn&>(iRIter->second);

      if (iC.empty())
      {
        continue;
      }

      ItinIndex::ItinColumnConstIterator iCIter = iC.begin();
      ItinIndex::ItinColumnConstIterator iCEIter = iC.end();
      for (; iCIter != iCEIter; ++iCIter)
      {
        const ItinIndex::ItinCell& curCell = static_cast<const ItinIndex::ItinCell&>(*iCIter);
        if (curCell.first.flags() & ItinIndex::ITININDEXCELLINFO_FAKEDIRECTFLIGHT)
        {
          continue;
        }

        rt++;
      }
    }
  }

  return (rt);
}

//-------------------------------------------------------------------
// Calculates the number of cells in an entire row
//-------------------------------------------------------------------
uint32_t
ItinIndex::retrieveNumberCellsInARow(const ItinIndex::Key& itinRowKey) const
{
  const ItinIndex::ItinRow* iR = retrieveItinRow(itinRowKey);

  if (iR == nullptr)
  {
    return (0);
  }

  uint32_t rt = 0;
  ItinIndex::ItinRowConstIterator iRIter = iR->begin();
  ItinIndex::ItinRowConstIterator iREIter = iR->end();

  for (; iRIter != iREIter; ++iRIter)
  {
    const ItinIndex::ItinColumn& iC = static_cast<const ItinIndex::ItinColumn&>(iRIter->second);

    if (UNLIKELY(iC.empty()))
    {
      continue;
    }

    ItinIndex::ItinColumnConstIterator iCIter = iC.begin();
    ItinIndex::ItinColumnConstIterator iCEIter = iC.end();
    for (; iCIter != iCEIter; ++iCIter)
    {
      const ItinIndex::ItinCell& curCell = static_cast<const ItinIndex::ItinCell&>(*iCIter);
      if (curCell.first.flags() & ItinIndex::ITININDEXCELLINFO_FAKEDIRECTFLIGHT)
      {
        continue;
      }

      rt++;
    }
  }

  return (rt);
}

//-------------------------------------------------------------------
// Creates an iterator that points to the start of the
// row cell base specified by the key
// Note I am following standard library conventions with the
// begin operation -- (begin, end] -- inclusive begin, exclusive end.
// Similar to set notation, begin is a valid part of the range, while
// end sits just outside the end of the valid range
//-------------------------------------------------------------------
ItinIndex::ItinIndexIterator
ItinIndex::beginRow(const ItinIndex::Key& itinRowKey)
{
  ItinIndex::ItinRowCellMapIterator curVIter = _rowCellMap.find(itinRowKey);
  if (UNLIKELY(curVIter == _rowCellMap.end()))
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "Invalid governing carrier specified for leg index iterator");
  }
  ItinIndex::ItinRowCellVector& curV = curVIter->second;
  return (ItinIndex::ItinIndexIterator(itinRowKey, &curV));
}

//-------------------------------------------------------------------
// Creates an iterator that points to the end of the
// row cell base specified by the key
// Note I am following standard library conventions with the
// end operation -- (begin, end] -- inclusive begin, exclusive end.
// Similar to set notation, begin is a valid part of the range, while
// end sits just outside the end of the valid range
//-------------------------------------------------------------------
ItinIndex::ItinIndexIterator
ItinIndex::endRow()
{
  return (ItinIndex::ItinIndexIterator(ItinIndex::ItinIndexIterator::IteratorModeNormalLeg));
}

ItinIndex::ItinIndexIterator
ItinIndex::beginAcrossStopOverRow(ShoppingTrx& trx,
                                  const uint32_t& acrossStopOverLegIdx,
                                  const ItinIndex::Key& govCxr)
{
  return (ItinIndex::ItinIndexIterator(govCxr, acrossStopOverLegIdx, &trx));
}

ItinIndex::ItinIndexIterator
ItinIndex::endAcrossStopOverRow()
{
  return (ItinIndex::ItinIndexIterator(ItinIndex::ItinIndexIterator::IteratorModeAcrossLeg));
}
} // tse
