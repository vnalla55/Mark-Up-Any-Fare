#pragma once

#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"
#include "DataModel/BrandingTrx.h"
#include "DataModel/FareComponentShoppingContext.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexShoppingTrx.h"
#include "DataModel/SettlementTypesTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/DataHandle.h"
#include "Xform/CustomXMLParser/IBaseHandler.h"
#include "Xform/CustomXMLParser/ICString.h"
#include "Xform/PreferredCabin.h"
#include "Xform/ShoppingSchemaNames.h"

#include <boost/optional.hpp>
#include <map>
#include <stack>

namespace tse
{
class AirSeg;
class Logger;
class TravelSeg;

class XMLShoppingHandler : public IBaseHandler
{
  friend class BffAltDatesTest;
  friend class BffXmlParserTest;
  friend class XMLShoppingHandlerTest;

public:
  XMLShoppingHandler(DataHandle& dataHandle);

  virtual bool startElement(int idx, const IAttributes& attrs) override;
  virtual void characters(const char* pValue, size_t length) override;
  virtual bool endElement(int idx) override;

  virtual bool startElement(const IKeyString&, const IAttributes& attrs) override { return false; }
  virtual bool endElement(const IKeyString&) override { return false; }

  PricingTrx* trx() { return _pricingTrx; }
  Trx* trxResult() { return _trxResult; }
  DateTime& bookingDT() { return _bookingDT; }
  const std::map<int, Itin*>& getItinMap() const { return _itnParser.getItinFamilyMap(); }

private:
  static Logger _logger;
  // The interface for a class which parses an element that contains
  // other elements.
  // To add support for a new type of element, make a class derived
  // from this class.
  class ElementParser
  {
  protected:
    ElementParser(XMLShoppingHandler& parser) : _parser(parser) {}

    XMLShoppingHandler& parser() { return _parser; }
    PricingTrx& trx();
    ShoppingTrx* shoppingTrx();

    DataHandle& dataHandle() { return _parser._dataHandle; }
    DateTime& bookingDT() { return _parser._bookingDT; }
    bool getNumEstimatesFromConfig() const { return _parser._getNumEstimatesFromConfig; }
    void setNumEstimatesFromConfig(bool value) { _parser._getNumEstimatesFromConfig = value; }
    void setEstimatedSolutions(const char* estimatedConfigName, const char* accuredConfigName);

    std::vector<DateTime>& itinDateTime() { return _parser._itinDateTime; }
    std::vector<ShoppingTrx::Leg>& legs();
    void setAAFIndex(uint32_t index, uint32_t mapto);
    uint32_t getAAFIndex(uint32_t index) const;
    void setLEGIndex(uint32_t index, uint32_t mapto);
    uint32_t getLEGIndex(uint32_t index) const;
    void setSOPIndex(uint32_t leg, uint32_t sop, uint32_t mapto);
    uint32_t getSOPIndex(uint32_t leg, uint32_t sop);
    void validateInput(bool isValid, const char* errorMessage, bool doAlert = false) const;
    void paxTypesInitialize();

  public:
    virtual ~ElementParser() = default;
    // function which is called when an element that this class
    // is designed to parse is encountered

    virtual void startParsing(const IAttributes& attrs) {}

    // function which is called when the element this class is
    // parsing is closed.
    virtual void endParsing(const IValueString& text) {}

    uint16_t determinePaxAge(PaxTypeCode& work);

    FlightFinderTrx* ffinderTrx();
    RexPricingTrx* rexPricingTrx() { return _parser._rexPricingTrx; }
    RexShoppingTrx* rexShoppingTrx() { return _parser._rexShoppingTrx; }
    RexExchangeTrx* rexExchangeTrx() { return _parser._rexExchangeTrx; }

    // function which is called when a new element is encountered
    // while parsing this object's child nodes. If the element
    // found is a complex element that contains children, then
    // this function should return a pointer to another parser
    // that is capable of parsing that element type.
    //
    // If the element is a simple element that only has attributes,
    // then the parser may parse it directly and return NULL.
    //
    // If the element is unrecognized, NULL should be returned,
    // and the entire sub-tree will be ignored.
    //
    // The possible parser objects that this parser can return
    // are contained within the XMLShoppingHandler object. This
    // avoids dynamic memory allocation every time an element
    // is encountered
    virtual ElementParser* newElement(int idx, const IAttributes& attrs) { return nullptr; }
    // common parsing functions
  protected:
    DateTime parseDateTime(const IValueString& date, const IValueString& time);
    DateTime parseDateTime(const IValueString& date);
    void setTimeInHistoricalDate(const IAttributes& attrs,
                                 DateTime& histDate,
                                 shopping::_AttributeNameIdx_ timeSource);
    void excludePaxTypeForIS(PaxType *paxType);
    void excludePaxTypesForIS(std::vector<PaxType*> &paxTypes);

  private:
    XMLShoppingHandler& _parser;

    ElementParser(const ElementParser&);
    void operator=(const ElementParser&);

    static ConfigurableValue<ConfigSet<PaxTypeCode>> _isExcludeMultiPaxType;
    void parseB12B13Attribute(const IAttributes& attrs, std::vector<CarrierCode>& nonPreferredVC, int attrType); //Preferred, Non-Preferred VC
  };

  // a parser for <ShoppingRequest> elements
  class ShoppingRequestParser : public ElementParser
  {
  protected:
    static Logger _logger;

  public:
    ShoppingRequestParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;
    virtual void endParsing(const IValueString& text) override;
    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;

    static void saveCountryAndStateRegionCodes(const PricingTrx& trx,
                                               const IAttributes& attrs,
                                               NationCode& employment,
                                               NationCode& nationality,
                                               LocCode& residency);
  protected:
    void parseAGIType(const IAttributes& attrs);
    void parseDURType(const IAttributes& attrs);
    void parsePDTType(const IAttributes& attrs);
    void parseBILType(const IAttributes& attrs);
    void parseODDType(const IAttributes& attrs);
    void parseONDType(const IAttributes& attrs);
    void parseFFGType(const IAttributes& attrs);
    void parseFFYType(const IAttributes& attrs);
    void parseRFGType(const IAttributes& attrs);
    void parseXRAType(const IAttributes& attrs);
    void parseDynamicConfigType(const IAttributes& attrs);
    void printTravelSegsCOS();
    void rejectUnsupportedRequest(char requestType) const;
    std::string serverType() const;
  };

  class PXITypeParser : public ElementParser
  {
  public:
    PXITypeParser(XMLShoppingHandler& parser) : ElementParser(parser), _paxInputOrder(0) {}

    virtual void startParsing(const IAttributes& attrs) override;
    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;
    virtual void endParsing(const IValueString& text) override;

  private:
    uint16_t _paxInputOrder;
    boost::optional<smp::Mode> _mpo;
  };


  class PENTypeParser : public ElementParser
  {
  public:
    PENTypeParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

    void startParsing(const IAttributes& attrs);

    boost::optional<MaxPenaltyInfo::Filter> _changeFilter;
    boost::optional<MaxPenaltyInfo::Filter> _refundFilter;

  protected:
    MaxPenaltyInfo::Filter getPenaltyInfo(const PricingTrx& trx, const IAttributes& attrs) const;
  };

  class OPTTypeParser : public ElementParser
  {
  public:
    OPTTypeParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

    void startParsing(const IAttributes& attrs);
  };

  class FGGTypeParser : public ElementParser
  {
  public:
    FGGTypeParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override {}
    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;

  private:
    void parseFGIType(const IAttributes& attrs);
  };

  // a parser for <SPV> elements

  class SPVTypeParser : public ElementParser
  {
    public:
     SPVTypeParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

      virtual void startParsing(const IAttributes& attrs) override;
      virtual void endParsing(const IValueString& text) override {}

    private:
      void parseSPVCodes(const std::string& strAttr, int attrType );
  };

  // a parser for <PRO> elements
  class PROTypeParser : public ElementParser
  {
  public:
    PROTypeParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;
    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;
    virtual void endParsing(const IValueString& text) override {}

  private:
    bool isFamilyGroupingDisable(const ShoppingTrx& trx);
    bool isFamilyGroupingDisableForPCC(const std::string& pcc);
    void parseVISParameters(const IAttributes& attrs);
    void parseTimeBins(const IAttributes& attrs,
                       int attributeName,
                       std::vector<VISTimeBin>& timeOfDayBinsVec);
    void validateTicketingDate(const DateTime& ticketingDT);
    void parseESVOptions(const IAttributes& attrs);
    void parseBrandingOptions(const IAttributes& attrs);
    void parseIBFOptions(const IAttributes& attrs);
    void parseTNBrandOptions(const IAttributes& attrs);
    void parseGSAOptions(const IAttributes &attrs);
    void parseGSAOptionsNew(const IAttributes &attrs);
    void parseCalendarShoppingOptions(const IAttributes& attrs);
    void parseExchangeOptions(const IAttributes& attrs);

    void parseS15XFFOptionsForFareFocus(const IAttributes& attrs);
    void restrictExcludeFareFocusRule();

    void parseS15PDOOptions(const IAttributes& attrs);
    void checkEprForPDOorXRS();
    void checkCominationPDOPDRXRSOptionalParameters();
    void parseS15PDROptions(const IAttributes& attrs);
    void checkEprForPDR();

    void parseS15XRSOptions(const IAttributes& attrs);

    void parseBI0CabinOptions(const IAttributes& attrs);

    void parsePFFAttribute(const IAttributes& attrs, PricingOptions* const options, PricingRequest* const request);
    void parseQ17Attribute(const IAttributes& attrs);
    void parseQ6FAttribute(const IAttributes& attrs, PricingOptions* const options, PricingRequest* const request);
    void parseOutboundDepartureDate(const IAttributes& attrs);
    void parseInboundDepartureDate(const IAttributes& attrs);
    void parseCollectOCFees(const IAttributes& attrs, PricingOptions* const options, PricingRequest* const request);
    void parseTicketingDateTimeOverride(const IAttributes& attrs, PricingOptions* const options, PricingRequest* const request);
    void parseCorporateIDs(const IAttributes& attrs, PricingRequest* const request);
    void parseAccountCodes(const IAttributes& attrs, PricingRequest* const request);
    void parseFLPAttribute(const IAttributes& attrs);
    void restrictNumberOfSolutions();
    void parseDynamicPriceDeviation(const IAttributes& attrs);
    static ConfigurableValue<ConfigSet<PseudoCityCode>> _pccsWithoutFamilyLogic;
    void parseB12B13Attribute(const IAttributes& attrs, std::vector<CarrierCode>& nonPreferredVC, int attrType); //Preferred, Non-Preferred VC
    void parseRetailerCode(const IAttributes& attrs);
  };

  // branded fare element
  class BRNTypeParser : public ElementParser
  {
  public:
    BRNTypeParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;

  private:
    void parseBookingCode(const BookingCode& bkc,
                          const BookingCode& secondaryBkc,
                          char indicator,
                          bool excludeFlag);
    void parseFareBasisOrFamily(const IValueString&, char, bool);

    template <typename T>
    bool isUniqe(const std::vector<T>& codes, const T& code) const;

    template <typename T>
    void collect(const T& code,
                 bool isExclude,
                 char indicator,
                 const char* msg,
                 std::vector<T>& codes,
                 std::vector<T>& excludeCodes,
                 std::map<T, char>& codeData) const;
  };

  // a parser for <BH0> elements
  class BH0TypeParser : public ElementParser
  {
  public:
    BH0TypeParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override {}
    virtual void endParsing(const IValueString& text) override;
  };

  // a parser for <MTY> elements
  class MTYTypeParser : public ElementParser
  {
  public:
    MTYTypeParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override {}
    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;

  private:
    void parseTXSType(const IAttributes& attrs);
  };

  // a parser for <XPN> elements
  class XPNTypeParser : public ElementParser
  {
  public:
    XPNTypeParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;
    virtual void endParsing(const IValueString& text) override {}
  };

  // a parser for <AAF> elements at the top level
  class AAFTypeParser : public ElementParser
  {
  public:
    AAFTypeParser(XMLShoppingHandler& handler);

    virtual void startParsing(const IAttributes& attrs) override;
    virtual void endParsing(const IValueString& text) override;
    virtual void setEstimatedSolutionsForInternational();
    virtual void setUpBoardOffMultiCities(AirSeg* seg);

    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;

  private:
    bool _isInternational;
    bool _interItin;
    void parseHSPType(const IAttributes& attrs, AirSeg* seg);
    void parseBRDType(const IAttributes& attrs, AirSeg* seg);
    void parseOFFType(const IAttributes& attrs, AirSeg* seg);
    void parseMILType(const IAttributes& attrs);
    void parsePUPType(const IAttributes& attrs);
    void parseSURType(const IAttributes& attrs);
    void startParsingRexAAF(const IAttributes& attrs, TravelSeg* const seg);
    void saveFareComponent(const IAttributes& attrs, TravelSeg* const seg);
    bool isExcArunkSegment(const IAttributes& attrs);
    void parseArunkSegment(const IAttributes& attrs);
    void startParsingContextShoppingAAF(const IAttributes& attrs, TravelSeg* const seg);

    using LocationMap = std::map<LocCode, const Loc*>;
    using LocationCache = std::map<int64_t, LocationMap>;
    LocationCache _locCache;
    const Loc* getLoc(const LocCode& city, const DateTime& dt);
    bool _sideTripStart;
    bool _sideTripEnd;
    uint16_t _sideTripNumber;
    FareCompInfo* _prevFareCompInfo;
    std::stack<FareCompInfo*> _fareCompInfoStack;
    const std::string _pssOpenDateBase;
    TravelSeg* _currentSeg;
  };
  // a parser for <AVL> elements at the top level
  class AVLTypeParser : public ElementParser
  {
  public:
    AVLTypeParser(XMLShoppingHandler& handler);

    virtual void startParsing(const IAttributes& attrs) override;
    virtual void endParsing(const IValueString& text) override;

    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;

    PricingTrx::ClassOfServiceKey& key() { return _classOfServiceKey; }
    std::vector<ClassOfServiceList>* value() { return _classOfServiceListVect; }

    std::vector<bool>& availabilityStatusESV() { return _availabilityStatusESV; }

    void endParsingESV(const IValueString& text);

  private:
    void addThruFareClassOfService(TravelSeg* seg, ClassOfService* cos);

    PricingTrx::ClassOfServiceKey _classOfServiceKey;
    std::vector<ClassOfServiceList>* _classOfServiceListVect;
    std::vector<bool> _availabilityStatusESV;
    uint32_t _q16;
  };

  // a parser for <FBK> elements
  class FBKTypeParser : public ElementParser
  {
  public:
    FBKTypeParser(XMLShoppingHandler& parser, AVLTypeParser& avlParser)
      : ElementParser(parser), _avlParser(avlParser)
    {
    }

    virtual void startParsing(const IAttributes& attrs) override;
    virtual void endParsing(const IValueString& text) override;

    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;
    bool isSetFBK() { return _setFBK; }
    void setFBK() { _setFBK = true; }

    void startParsingESV(const IAttributes& attrs);

    const Cabin* getCabin(const CarrierCode& cxr, const BookingCode& bkc, const DateTime& dt, const AirSeg* seg=nullptr);

  private:
    class ClassOfService* parseClassOfService(const IAttributes& attrs);
    void parseBKCType(const IAttributes& attrs);

    AVLTypeParser& _avlParser;
    bool _setFBK = false;

    using CabinKey = std::pair<CarrierCode, BookingCode>;
    using CabinMap = std::map<CabinKey, const Cabin*>;
    using CabinCache = std::map<int64_t, CabinMap>;
    CabinCache _cabinCache;
  };
  // a parser for <ITN> elements
  class ITNTypeParser : public ElementParser
  {
  public:
    ITNTypeParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

    virtual void endParsing(const IValueString& text) override;
    virtual void startParsing(const IAttributes& attrs) override;
    virtual void startParsingRexITN(const IAttributes& attrs);
    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;
    void cloneStopOverSeg(std::vector<TravelSeg*>& travelSeg);
    virtual const std::map<int, Itin*>& getItinFamilyMap() const { return _itinFamily; }

    std::map<const TravelSeg*, bool> travelSegStopOver;
    std::map<const TravelSeg*, TravelSeg*> clonedTravelSeg;

  private:
    std::map<int, Itin*> _itinFamily;
  };

  // a parser of <SID> elements
  class SIDTypeParser : public ElementParser
  {
  public:
    SIDTypeParser(XMLShoppingHandler& parser) : ElementParser(parser), _travelSegNumber(0) {}

    virtual void startParsing(const IAttributes& attrs) override;

    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;
    virtual void endParsing(const IValueString& text) override {}

  private:
    uint16_t _travelSegNumber;
    void parseBKKType(const IAttributes& attrs);
  };

  // a parser of <BRI> elements
  class BRITypeParser : public ElementParser
  {
  public:
    BRITypeParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;

    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;
    virtual void endParsing(const IValueString& text) override {}

  private:
    void parsePRGType(const IAttributes& attrs);
    BrandCode _brandCode;
  };

  // a parser for <SOP> elements
  class SOPTypeParser : public ElementParser
  {
  public:
    SOPTypeParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;
    virtual void endParsing(const IValueString& text) override;

    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;

  private:
    void parseFIDType(const IAttributes& attrs);
    void parseAIDType(const IAttributes& attrs);
    void adjustLegIdForTaxEnhancement(TravelSeg& seg);
  };

  // a parser for <LEG> elements
  class LEGTypeParser : public ElementParser
  {
  public:
    LEGTypeParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;
    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;
    virtual void endParsing(const IValueString& text) override;

  private:
    virtual void parseLEGBKKType(const IAttributes& attrs);
    virtual void parseLEGBKKForRBDAnswerTable(const IAttributes& attrs);
    PreferredCabin _cabinSelector;
    uint32_t _legId = 0;
  };

  // a parser for <DIA> elements
  class DIATypeParser : public ElementParser
  {
  public:
    DIATypeParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;
    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;
    void endParsing(const IValueString& text) override;

  private:
    void parseARGType(const IAttributes& attrs);
  };

  // a parser for <BFF> elements
  class BFFTypeParser : public ElementParser
  {
  public:
    BFFTypeParser(XMLShoppingHandler& parser)
      : ElementParser(parser), _isRoundTripRequested(false), _optInboundSeg(nullptr)
    {
    }

    virtual void startParsing(const IAttributes& attrs) override;
    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;
    virtual void endParsing(const IValueString&) override;

    void createBffAltDatesAndJourneyItin();

    bool _isRoundTripRequested;
    std::vector<DateTime> _bffOutboundDates;

  protected:
    void parseRQFType(const IAttributes& attrs);
    void parsePSGType(const IAttributes& attrs);
    void parseDRGType(const IAttributes& attrs);

    AirSeg* _optInboundSeg;
  };

  class DTLTypeParser : public ElementParser
  {
  public:
    DTLTypeParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;
    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;

    virtual void endParsing(const IValueString& text) override;
    void parseIBLType(const IAttributes& attrs);

    void setAltDatesDuration(DatePair& inOutDatePair, PricingTrx::AltDateInfo* altInfo);

  private:
    std::vector<DateTime> _outboundDateList;
    std::vector<DateTime> _inboundDateList;
  };

  class CMDTypeParser : public ElementParser
  {
  public:
    CMDTypeParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

    virtual void endParsing(const IValueString& text) override;
  };

  // a parser for <EXC> elements
  class EXCTypeParser : public ShoppingRequestParser
  {
  public:
    EXCTypeParser(XMLShoppingHandler& parser) : ShoppingRequestParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;
    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;

    virtual void endParsing(const IValueString& text) override;

  protected:
    void parseEXCPROType(const IAttributes& attrs);
  };

  // parser for <PCL> elements
  class PCLTypeParser : public ShoppingRequestParser
  {
  public:
    PCLTypeParser(XMLShoppingHandler& parser) : ShoppingRequestParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;
  };

  // parser for <FCL> elements
  class FCLTypeParser : public ShoppingRequestParser
  {
  public:
    FCLTypeParser(XMLShoppingHandler& parser) : ShoppingRequestParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;
  };

  // parser for <FCI> elements
  class FCITypeParser : public ShoppingRequestParser
  {
  public:
    FCITypeParser(XMLShoppingHandler& parser) : ShoppingRequestParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;
    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;
  };

  // parser for <VCT> elements
  class VCTTypeParser : public ShoppingRequestParser
  {
  public:
    VCTTypeParser(XMLShoppingHandler& parser) : ShoppingRequestParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;
  };

  // parser for <R3I> elements
  class R3ITypeParser : public ShoppingRequestParser
  {
  public:
    R3ITypeParser(XMLShoppingHandler& parser) : ShoppingRequestParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;
    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;
  };

  // parser for <SEQ> elements
  class SEQTypeParser : public ShoppingRequestParser
  {
  public:
    SEQTypeParser(XMLShoppingHandler& parser) : ShoppingRequestParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;
  };

  // brand info
  class BRSTypeParser : public ElementParser
  {
  public:
    BRSTypeParser(XMLShoppingHandler& parser) : XMLShoppingHandler::ElementParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;
    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;
  };

  class DIVTypeParser : public ElementParser
  {
  public:
    DIVTypeParser(XMLShoppingHandler& handler) : ElementParser(handler) {}

    void startParsing(const IAttributes& attrs) override;
    void parseCXPType(const IAttributes& attrs);
    ElementParser* newElement(int idx, const IAttributes& attrs) override;

  };

  // a parser for <FFG> elements
  class FFGTypeParser : public ElementParser
  {
  public:
    FFGTypeParser(XMLShoppingHandler& parser) : ElementParser(parser) {}

    virtual void startParsing(const IAttributes& attrs) override;
    virtual ElementParser* newElement(int idx, const IAttributes& attrs) override;
    virtual void endParsing(const IValueString& text) override;
  private:
    boost::optional<smp::Mode> _ffgMPO;
  };

  ShoppingTrx* shoppingTrx = nullptr;

  // the parsers that can be returned from 'newElement()'
  PXITypeParser _pxiParser{*this};
  PENTypeParser _penParser{*this};
  SPVTypeParser _spvParser{*this};
  PROTypeParser _proParser{*this};
  BRNTypeParser _brnParser{*this};
  AAFTypeParser _aafParser{*this};
  AVLTypeParser _avlParser{*this};
  FBKTypeParser _fbkParser{*this, _avlParser};
  LEGTypeParser _legParser{*this};
  ITNTypeParser _itnParser{*this};
  DIATypeParser _diaParser{*this};
  CMDTypeParser _cmdParser{*this};

  BH0TypeParser _bh0Parser{*this};
  MTYTypeParser _mtyParser{*this};
  OPTTypeParser _optParser{*this};
  XPNTypeParser _xpnParser{*this};
  FGGTypeParser _fggParser{*this};
  SOPTypeParser _sopParser{*this};
  SIDTypeParser _sidParser{*this};
  BRITypeParser _briParser{*this};

  BFFTypeParser _bffParser{*this};
  DTLTypeParser _dtlParser{*this};

  EXCTypeParser _excParser{*this};
  PCLTypeParser _pclParser{*this};
  FCLTypeParser _fclParser{*this};
  FCITypeParser _fciParser{*this};
  VCTTypeParser _vctParser{*this};
  R3ITypeParser _r3iParser{*this};
  SEQTypeParser _seqParser{*this};
  BRSTypeParser _brsParser{*this};
  DIVTypeParser _divParser{*this};
  FFGTypeParser _ffgParser{*this};

  ShoppingRequestParser _shoppingRequestParser{*this};

  void createShoppingTrx();
  void createPricingTrx();
  void createBrandingTrx();
  void createFFinderTrx();
  void createRexPricingTrx();
  void createRexShoppingTrx();
  void createRexExchangeTrx();
  void createSettlementTypesTrx();

  void initTrx();
  // the pointers to the transaction.
  // if the transaction hasn't been initialized, both will be NULL.
  // if we have a PricingTrx, then _shoppingTrx will be NULL.
  // if we have a ShoppingTrx, then both pointers will point to it.

  void updateDefaultFFGData(const flexFares::Attribute& aValue, const std::string& sValue = "");
  PricingTrx* _pricingTrx = nullptr;
  BrandingTrx* _brandingTrx = nullptr;
  SettlementTypesTrx* _settlementTypesTrx = nullptr;
  ShoppingTrx* _shoppingTrx = nullptr;
  FlightFinderTrx* _ffinderTrx = nullptr;
  RexPricingTrx* _rexPricingTrx = nullptr;
  RexExchangeTrx* _rexExchangeTrx = nullptr;
  RexShoppingTrx* _rexShoppingTrx = nullptr;
  Trx* _trxResult = nullptr;
  DataHandle& _dataHandle;

  DateTime _bookingDT;
  bool _getNumEstimatesFromConfig = false;
  std::vector<DateTime> _itinDateTime;

  // pointers to the stack of parsers. The parser on the top of
  // the stack is the parser currently active. A parser is pushed
  // onto the stack every time an element is opened, and popped
  // off the stack every time an element is closed.
  std::stack<ElementParser*> _parsers;

  // a set of legs to use if we have a PricingTrx. The ShoppingTrx has
  // its own legs, but the PricingTrx doesn't, and they aren't needed
  // after parsing is finished, so they are stored here.
  std::vector<ShoppingTrx::Leg> _pricingLegs;
  // set of leg to use in historical mip
  std::vector<ShoppingTrx::Leg> _excLegs;

  std::map<uint32_t, uint32_t> _aafIndices;
  std::map<uint32_t, uint32_t> _legIndices;

  std::map<uint32_t, uint32_t> _aafExcIndices; // for Exchange Itinerary
  std::map<uint32_t, uint32_t> _legExcIndices; // for Exchange Itinerary

  // Context Shopping
  skipper::FareComponentShoppingContextsByFareCompNo _fcShoppingContexts;

  IValueString _value;

private:
  bool _parsingExchangeInformation = false;
  bool _parsingAccompanyPassenger = false;
  std::vector<TravelSeg*> _excTravelSeg;
  int _numberOfEXCITNTypes = 0;
  int _numberOfNEWITNTypes = 0;
  CurrencyCode _newItinFareCalcCurrency;
  int16_t _numberOfSolutions = 0;
  bool _disableFamilyLogic = false; // PRO/DFL
  bool _isLegIdAdjusted = false;
  static flexFares::GroupId _defaultFlexFaresGroupId;
  flexFares::GroupId _defaultFlexFaresGroupID = 0;
  bool _isTagPROProcessed = false;
  bool _isS15PROProcessed = false;
  static flexFares::GroupId _flexFaresGroupId;
  flexFares::GroupId _flexFaresGroupID = 0;
  bool _isValidFlexFareGroupCreated = false;
  bool _isTagPXIProcessed = false;

public:
  bool& parsingExchangeInformation() { return (_parsingExchangeInformation); }
  const bool& parsingExchangeInformation() const { return (_parsingExchangeInformation); }

  bool& parsingAccompanyPassenger() { return (_parsingAccompanyPassenger); }
  const bool& parsingAccompanyPassenger() const { return (_parsingAccompanyPassenger); }

  std::vector<TravelSeg*>& excTravelSeg() { return _excTravelSeg; }
  const std::vector<TravelSeg*>& excTravelSeg() const { return _excTravelSeg; }

  int& numberOfEXCITNTypes() { return _numberOfEXCITNTypes; }
  const int& numberOfEXCITNTypes() const { return _numberOfEXCITNTypes; }

  int& numberOfNEWITNTypes() { return _numberOfNEWITNTypes; }
  const int& numberOfNEWITNTypes() const { return _numberOfNEWITNTypes; }

  flexFares::GroupId& defaultFlexFaresGroupID(PricingTrx& trx);
  const flexFares::GroupId& defaultFlexFaresGroupID(PricingTrx& trx) const;

  flexFares::GroupId& flexFaresGroupID(PricingTrx& trx);
  const flexFares::GroupId& flexFaresGroupID(PricingTrx& trx) const;

  std::vector<ShoppingTrx::Leg>& excLegs() { return _excLegs; }
  const std::vector<ShoppingTrx::Leg>& excLegs() const { return _excLegs; }

  CurrencyCode& newItinCurrency() { return _newItinFareCalcCurrency; }
  const CurrencyCode& newItinCurrency() const { return _newItinFareCalcCurrency; }

  void setNumberOfSolutions(int16_t number) { _numberOfSolutions = number; }
  int16_t getNumberOfSolutions() const { return _numberOfSolutions; }

  void readShoppingConfigParameters(PricingTrx& pricingTrx);
  void determineCosExceptionFixEnabled(PricingTrx& pricingTrx);

  bool isDisableFamilyLogic() const { return _disableFamilyLogic; }
  void setDisableFamilyLogic(bool value) { _disableFamilyLogic = value; }

  // Indicate in options that the request is a
  // S8 Branded Fare request. Turn off
  // family logic at the level of Itinerary Selector
  // when dealing with an IBF request.
  void markBrandedFaresRequest(PricingRequest& request)
  {
    request.setBrandedFaresRequest(true);
    request.setParityBrandsPath(true);
  }

  void enableDropResultsOnTimeout(PricingRequest& request)
  {
    request.setDropResultsOnTimeout(true);
  }

  void markSettlementTypesRequest(PricingRequest& request)
 {
   request.setSettlementTypesRequest(true);
 }
  void setCatchAllBucket(PricingRequest& request, bool value)
  {
    request.setCatchAllBucketRequest(value);
  }
  void enableCheapestWithLegParityPath(PricingRequest& request)
  {
    request.setBrandedFaresRequest(true);
    request.setCheapestWithLegParityPath(true);
  }
  void markChangeSoldoutBrand(PricingRequest& request) { request.setChangeSoldoutBrand(true); }
  void setValidatingCarrierRequest(PricingRequest& request, bool flag)
  {
    request.setValidatingCarrierRequest(flag);
  }
  void setAlternateValidatingCarrierRequest(PricingRequest& request, bool flag)
  {
    request.setAlternateValidatingCarrierRequest(flag);
  }
  void setSettlementMethodOverride(PricingRequest& request, std::string& value)
  {
    request.setSettlementMethod(value.c_str());
  }
  void markAllFlightsRepresented(PricingRequest& request)
  {
    request.setAllFlightsRepresented(true);
  }
  void markReturnIllogicalFlights(PricingRequest& request)
  {
    request.setReturnIllogicalFlights(true);
  }
  void setScheduleRepeatLimit(PricingRequest& request, uint16_t value)
  {
    request.setScheduleRepeatLimit(value);
  }
  void setAllFlightsData(PricingRequest& request)
  {
    request.setAllFlightsData(true);
  }
  bool isLegIdAdjusted() const { return _isLegIdAdjusted; }
  void setLegIdAdjusted(bool flag) { _isLegIdAdjusted = flag; }

  bool isTagPROProcessed() const { return _isTagPROProcessed; }
  void setTagPROProcessed() { _isTagPROProcessed = true; }

  bool isS15PROProcessed() const { return _isS15PROProcessed; }
  void setS15PROProcessed() { _isS15PROProcessed = true; }

  bool isValidFlexFareGroupCreated() const { return _isValidFlexFareGroupCreated; }
  void setValidFlexFareGroupCreated(bool status) { _isValidFlexFareGroupCreated = status; }

  bool isTagPXIProcessed() const { return _isTagPXIProcessed; }
  void setTagPXIProcessed() { _isTagPXIProcessed = true; }
};

} // tse

