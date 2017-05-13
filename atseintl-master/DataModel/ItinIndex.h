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
#pragma once

#include "Common/Global.h"
#include "Common/Hasher.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <cstring>
#include <map>
#include <vector>

namespace tse
{
class Itin;
class PaxTypeFare;
class ShoppingTrx;

// Itin group constants
const uint32_t ITININDEX_DEFAULT = 0;
const uint32_t ITININDEX_OFFSET = 1;

// Carrier mapped, row major, itin index
/*

        Cxr          Conxn            Index
        Key           Key              Key
-------------------------------------------------------------------
Matrix ----- ItinRow ----- ItinColumn ----- ItinCell(Index,Itin*)
AA - - - - - 0 -  -  -  -  0 -  -  -  -  -  0 - Itin0 --
               \             \  -  -  -  -  1 - Itin1  | - 0 connection point AA Itins
                -             \ -  -  -  -  2 - Itin2 --
                 \
                  -  -  -  1 -  -  -  -  -  0 - Itin3 --
                             \  -  -  -  -  1 - Itin4  | - 1 connection point AA Itins
                              \ -  -  -  -  2 - Itin5 --

-------------------------------------------------------------------
*/
class ItinIndex final
{
public:
  // ItinIndex generic key type
  typedef uint32_t Key;

  // ItinIndex key generator type
  typedef tse::Hasher KeyGenerator;

  // ItinIndex flags for the ItinCellInfo structure
  static const uint32_t ITININDEXCELLINFO_FAKEDIRECTFLIGHT = 0x00000001;

  static const uint32_t ITININDEXCELLINFO_FAKEDFLIGHT = 0x00000002;
  // ItinIndex enumeration for fake direct itin
  enum ItinCheckType
  {
    CHECK_NOTHING = 0,
    CHECK_FAKEDIRECTFLIGHT = 1
  };

  // ItinCell info structure, expandable structure
  // that will contain info about the itin stored in
  // the cell itself
  class ItinCellInfo final
  {
  private:
    uint32_t _sopIndex = 0;
    uint32_t _itinColumnIndex = 0;
    GlobalDirection _globalDirection = GlobalDirection::ZZ;
    uint32_t _flags = 0;
    bool _combineSameCxr = false;
    TravelSeg* _primarySector = nullptr;

  public:
    uint32_t& sopIndex() { return (_sopIndex); }
    const uint32_t& sopIndex() const { return (_sopIndex); }

    uint32_t& itinColumnIndex() { return (_itinColumnIndex); }
    const uint32_t& itinColumnIndex() const { return (_itinColumnIndex); }

    GlobalDirection& globalDirection() { return (_globalDirection); }
    const GlobalDirection& globalDirection() const { return (_globalDirection); }

    uint32_t& flags() { return (_flags); }
    const uint32_t& flags() const { return (_flags); }

    bool& combineSameCxr() { return (_combineSameCxr); }
    const bool& combineSameCxr() const { return (_combineSameCxr); }

    void setPrimarySector(TravelSeg* primarySector) { _primarySector = primarySector; }
    TravelSeg* getPrimarySector() const { return _primarySector; }
  };

  // ItinIndex ItinCell type (sop index, Itin*)
  typedef std::pair<ItinCellInfo, Itin*> ItinCell;

  // ItinIndex ItinColumn type
  typedef std::vector<ItinCell> ItinColumn;
  typedef ItinColumn::iterator ItinColumnIterator;
  typedef ItinColumn::const_iterator ItinColumnConstIterator;

  // ItinIndex ItinRow type
  typedef std::map<Key, ItinColumn> ItinRow;
  typedef ItinRow::iterator ItinRowIterator;
  typedef ItinRow::const_iterator ItinRowConstIterator;

  // ItinIndex ItinRow pair type
  typedef std::pair<Key, ItinColumn> ItinRowPair;

  // ItinIndex ItinMatrix type
  typedef std::map<Key, ItinRow> ItinMatrix;
  typedef ItinMatrix::iterator ItinMatrixIterator;
  typedef ItinMatrix::const_iterator ItinMatrixConstIterator;

  // ItinIndex ItinMatrix pair type
  typedef std::pair<Key, ItinRow> ItinMatrixPair;

  // ItinIndex RowCell vector type
  typedef std::vector<ItinCell> ItinRowCellVector;
  typedef ItinRowCellVector::iterator ItinRowCellVectorIterator;
  typedef ItinRowCellVector::const_iterator ItinRowCellVectorConstIterator;

  // ItinIndex RowCell vector map type
  typedef std::map<Key, ItinRowCellVector> ItinRowCellMap;
  typedef ItinRowCellMap::iterator ItinRowCellMapIterator;
  typedef ItinRowCellMap::const_iterator ItinRowCellMapConstIterator;

  // ItinIndex RowCell pair type
  using ItinRowCellPair = std::pair<Key, ItinRowCellVector>;

  class ItinIndexIterator final
  {
    friend class PaxTypeFareBitmapValidatorTest;

  public:
    enum IteratorMode
    {
      IteratorModeNormalLeg = 0,
      IteratorModeAcrossLeg = 1
    };

    static const uint32_t INVALID_BITINDEX = 0xf0000000;

  private:
    // Hidden functions (inefficient post-operators)
    // DO NOT USE or IMPLEMENT
    // Copy overhead is too expensive for this class
    ItinIndexIterator operator++(int n);
    explicit ItinIndexIterator();

  private:
    void invalidate();
    void prepareCell(uint32_t atIndex = 0);
    void generateItinSet();
    void prepareAcrossLegCell(uint32_t atIndex = 0);
    void initialize(uint32_t atIndex = 0);
    void initializeAcrossLeg(uint32_t atIndex = 0);
    void advance();
    void internalLog(const std::string& desc);

  private:
    IteratorMode _mode = IteratorMode::IteratorModeNormalLeg;

    uint32_t _bitIndex = 0;
    uint32_t _numberBits = 0;
    uint32_t _adoptedLegIndex = 0;
    uint32_t _acrossLegIndex = 0;
    uint32_t _totalTravelSegSize = 0;

    ItinIndex::Key _rowKey = 0;
    std::map<ItinIndex::Key, IndexVectors>* _acrossStopOverCombinations = nullptr;
    ItinIndex::ItinRowCellVector* _cellVector = nullptr;
    ItinIndex::ItinCell* _cell = nullptr;
    ShoppingTrx* _shoppingTrx = nullptr;
    IndexVector* _jumpedLegs = nullptr;
    std::vector<TravelSeg*> _tSegHolder;
    IndexPairVector _currentItinSet;
    SopIdVec _currentSopSet;
    bool _valid = false;

  public:
    explicit ItinIndexIterator(IteratorMode mode);

    explicit ItinIndexIterator(const ItinIndex::Key& rowKey, ItinIndex::ItinRowCellVector* cells);

    explicit ItinIndexIterator(const ItinIndex::Key& rowKey,
                               ItinIndex::ItinRowCellVector* cells,
                               const uint32_t& atIndex);

    explicit ItinIndexIterator(const ItinIndex::Key& rowKey,
                               const uint32_t& acrossLegIndex,
                               ShoppingTrx* shoppingTrx);
    explicit ItinIndexIterator(const ItinIndex::Key& rowKey,
                               const uint32_t& acrossLegIndex,
                               ShoppingTrx* shoppingTrx,
                               const uint32_t& atIndex);

    ItinIndexIterator& operator++();
    bool operator==(const ItinIndexIterator& rhs);
    bool operator!=(const ItinIndexIterator& rhs);
    ItinIndex::ItinCell& operator*() { return (*_cell); }
    ItinIndex::ItinCell* operator->() { return (_cell); }
    const ItinIndex::ItinCell& operator*() const { return *_cell; }
    const ItinIndex::ItinCell* operator->() const { return _cell; }

    void logData(const char* desc);
    void logData(const std::string& desc);

    const bool isValid() const { return (_valid); }
    uint32_t bitIndex() const { return (_bitIndex); }
    const SopIdVec& currentSopSet() const { return (_currentSopSet); }
    const uint32_t& totalTravelSegSize() { return (_totalTravelSegSize); }
  };

private:
  ItinIndex::ItinRow& addItinRow(const ItinIndex::Key& itinRowKey);

  ItinIndex::ItinColumn&
  addItinColumn(ItinIndex::ItinRow& itinRow, const ItinIndex::Key& itinColumnKey);

  void createKey(const std::string& value, ItinIndex::Key& key);

  void createKey(const uint32_t& value, ItinIndex::Key& indexer);

  ItinIndex::ItinCell* findCellLoop(ItinIndex::ItinRowIterator& iRowIter,
                                    const ItinIndex::ItinRowIterator& iRowEIter,
                                    const ItinIndex::Key& targetKey);

  const ItinIndex::ItinCell* findCellLoop(ItinIndex::ItinRowConstIterator& iRowIter,
                                          const ItinIndex::ItinRowConstIterator& iRowEIter,
                                          const ItinIndex::Key& targetKey) const;

  ItinIndex::ItinCell*
  findCellInRow(ItinIndex::ItinRow& itinRow, const ItinIndex::ItinCheckType& checkType);

  const ItinIndex::ItinCell*
  findCellInRow(const ItinIndex::ItinRow& itinRow, const ItinIndex::ItinCheckType& checkType) const;

private:
  ItinIndex::ItinMatrix _root;
  ItinIndex::ItinRowCellMap _rowCellMap;

public:
  // Adds the specified itin
  void addItinCell(Itin*& itin,
                   ItinCellInfo& itinCellInfo,
                   const ItinIndex::Key& itinRowKey,
                   const ItinIndex::Key& itinColumnKey);

  const ItinIndex::ItinRow* retrieveItinRow(const ItinIndex::Key& itinRowKey) const;

  // Finds the direct itin for the carrier
  // specified or the most direct itin available
  ItinIndex::ItinCell*
  retrieveTopItinCell(const ItinIndex::Key& itinRowKey, const ItinIndex::ItinCheckType& checkType);

  // Finds the direct itin for the carrier
  // specified or the most direct itin available
  const ItinIndex::ItinCell* retrieveTopItinCell(const ItinIndex::Key& itinRowKey,
                                                 const ItinIndex::ItinCheckType& checkType) const;

  // Calculates the number of cells in the entire index
  uint32_t retrieveTotalNumberCells() const;

  // Calculates the number of cells in an entire row
  uint32_t retrieveNumberCellsInARow(const ItinIndex::Key& itinRowKey) const;

  // Creates an iterator that points to the start of the
  // row specified by the key for normal leg carrier indices
  ItinIndex::ItinIndexIterator beginRow(const ItinIndex::Key& itinRowKey);

  // Creates an end iterator for normal leg carrier indices
  ItinIndex::ItinIndexIterator endRow();

  // Creates an iterator that points to the start of the
  // row specified by the key for across stop over leg carrier indices
  ItinIndex::ItinIndexIterator beginAcrossStopOverRow(ShoppingTrx& trx,
                                                      const uint32_t& acrossStopOverLegIdx,
                                                      const ItinIndex::Key& itinRowKey);

  // Creates an end iterator for across stop over leg carrier indices
  ItinIndex::ItinIndexIterator endAcrossStopOverRow();

  // Accessors
  ItinIndex::ItinMatrix& root() { return (_root); }
  const ItinIndex::ItinMatrix& root() const { return (_root); }

  ItinIndex::ItinRowCellMap& rowCellMap() { return (_rowCellMap); }
  const ItinIndex::ItinRowCellMap& rowCellMap() const { return (_rowCellMap); }
};
} // End namespace tse
