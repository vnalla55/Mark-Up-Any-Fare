#ifndef CALL_TREE_DATA_COLLECTOR_H
#define CALL_TREE_DATA_COLLECTOR_H

#include <fstream>

namespace tse
{

class CallTreeDataCollector
{
 public:

  typedef enum CollectionState
  {
    CTCS_INITIALSETUP   = 0,
    CTCS_GETLINE        = 1,
    CTCS_PROCESSLINE    = 2,
    CTCS_LINESTARTTREE  = 3,
    CTCS_LINEMIDTREE    = 4,
    CTCS_FINISHED       = 5,
    CTCS_NUMBERSTATES
  };

  typedef enum AdvanceLineMode
  {
    ADVANCELINE_NOSKIP = 0,
    ADVANCELINE_SKIPBLANK = 1
  };

 private:
  char _innerBuffer[INNERBUFFER_SIZE];
  std::string _innerBufferStr;
  CallTreeHeader _callTreeHeader;
  CallTreeMethodTree* _currentMethodTree;
  std::map<std::string, CallTreeMethodTree*> _callTreeMethodTrees;
  std::vector<CallTreeMethodTree*> _highestToLowestCostMethods;
  std::string _fileName;
  std::ifstream _file;
  std::string _buffer;
  std::vector<CTTokenVector> _lines;
  CTTokenVector* _currentLineData;
  unsigned int _currentLineCount;  
  unsigned int _currentLine;
  CollectionState _lastState;
  CollectionState _state;
  bool _isFileOpen;
  bool _startedTree;
  bool _blankLine;

 private:
  CallTreeDataCollector(const CallTreeDataCollector& rhs);
  CallTreeDataCollector operator=(const CallTreeDataCollector& rhs);

 private:
  //For reading the next line from the file,
  //only used in the initial phase of reading in 
  //all of the lines
  bool readNextLine();

  //For advancing to the next stored line
  unsigned int advanceLine(AdvanceLineMode aLMode);

  //For use in processing the current state 
  bool processMode();

  //Create the method tree
  bool createEntireMethodTree();
  void recursiveTreeConstruct(CallTreeMethodTree* parent);
  CallTreeMethodTree* findHighestCostTree(std::map<std::string, CallTreeMethodTree*>& trees);
  void outputTree(CallTreeMethodTree* parent);

  //Header functions
  bool readHeaderLine(const std::pair<uint32_t,uint32_t>& range, std::string& outData);
  bool readHeaderLine(const uint32_t& index, std::string& outData);

  //Multi-line advance function
  void multiAdvanceLine(const uint32_t& numTimes, AdvanceLineMode aLMode);

  //Processing functions
  void endCurrentMethodTree();
  void startNewMethodTree();
  void addToCurrentMethodTree();
  CallTreeMethodTree* createMethodTree();
  
  CallTreeMethod::CallTreeMethodType determineMethodType();

 public:
  explicit CallTreeDataCollector(std::string& fileName) :
    _innerBuffer(),
    _innerBufferStr(""),
    _callTreeHeader(),
    _currentMethodTree(nullptr),
    _callTreeMethodTrees(),
    _highestToLowestCostMethods(),
    _fileName(fileName), 
    _file(_fileName.c_str()),
    _buffer(""),
    _lines(),
    _currentLineData(nullptr),
    _currentLineCount(0),
    _currentLine(0),
    _lastState(CollectionState(0)),
    _state(CollectionState(0)),
    _isFileOpen(_file.is_open()),
    _startedTree(false),
    _blankLine(false)
  {    
  }

  ~CallTreeDataCollector()
  {
    if (_isFileOpen)
    {
      _file.close();
      _currentLineCount = 0;
      _isFileOpen = false;
    }
  }

  bool processFile();
  bool processData();

  //Accessors
  std::vector<CTTokenVector>& lines() {return(_lines);}
  const std::vector<CTTokenVector>& lines() const {return(_lines);};

  std::vector<CallTreeMethodTree*>& highestToLowestCostMethods() { return(_highestToLowestCostMethods);};
  const std::vector<CallTreeMethodTree*>& highestToLowestCostMethods() const { return(_highestToLowestCostMethods);};

  CallTreeHeader& callTreeHeader() { return(_callTreeHeader);};
  const CallTreeHeader& callTreeHeader() const { return(_callTreeHeader);};

  const std::string& fileName() const {return(_fileName);};
  const unsigned int& currentLineCount() const {return(_currentLineCount);};
};

}//End namespace tse

#endif //CALL_TREE_DATA_COLLECTOR_H
