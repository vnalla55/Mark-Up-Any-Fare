#ifndef CALL_TREE_COMMAND_LINE_PROCESSOR
#define CALL_TREE_COMMAND_LINE_PROCESSOR

namespace tse
{

class CallTreeCommandLineProcessor
{
 private:
  std::string _inputFileName;
  std::string _numberFunctions;
  std::string _outputDir;
  bool _inputValid;

 public:
  explicit CallTreeCommandLineProcessor(int argc, char* argv[]) : 
    _inputFileName(""),
    _numberFunctions(""),
    _outputDir(""),
    _inputValid(true)
  {
    if (argc < static_cast<int>(CTComLineMinimum))
    {
      std::cout << "Invalid syntax:" << std::endl;
      std::cout << "CallTreeAnalyzer options" << std::endl; 
      std::cout << "Options are as follows: " << std::endl
                << CTComLineInput        << " <input-file> "  << std::endl
                << CTComLineNumFunctions << " <# functions> " << std::endl
                << CTComLineOutputDir    << " <output-dir> "  << std::endl;
      _inputValid = false;
      return;
    }
    else
    {
      int j = 1;
      for (j = 1; j < argc-1; j+=2)
      {
        std::string curArg = argv[j];

        if (curArg == CTComLineInput)
        {
          _inputFileName = argv[j+1];
        }
        else if (curArg == CTComLineNumFunctions)
        {
          _numberFunctions = argv[j+1];
        }
        else if (curArg == CTComLineOutputDir)
        {
          _outputDir = argv[j+1];
        }
        else
        {
          std::cerr << "Invalid command line token encountered: " << std::endl 
                    << curArg << " is not a valid input option."  << std::endl;
          _inputValid = false;
          return;
        }
      }

      if (_inputFileName.empty() || _inputFileName.length() == 0)
      {
        std::cerr << "Could not process input filename!" << std::endl;
        _inputValid = false;
      }
      else
      {
        std::cout << "Input file name = " << _inputFileName << std::endl;
      }

      if (_numberFunctions.empty() || _numberFunctions.length() == 0)
      {
        std::cerr << "Could not process number of functions!" << std::endl;
        _inputValid = false;
      }
      else
      {
        std::cout << "Number functions = " << _numberFunctions << std::endl;
      }

      if (_outputDir.empty() || _outputDir.length() == 0)
      {
        std::cerr << "Could not process output directory!" << std::endl;
        _inputValid = false;
      }
      else
      {
        std::cout << "Output directory = " << _outputDir << std::endl;
      }
    }
  }

  //Accessors
  std::string& inputFileName() { return(_inputFileName);};
  const std::string& inputFileName() const { return(_inputFileName);};

  std::string& numberFunctions() { return(_numberFunctions);};
  const std::string& numberFunctions() const { return(_numberFunctions);};

  std::string& outputDir() { return(_outputDir);};
  const std::string& outputDir() const { return(_outputDir);};
  const bool& inputValid() const { return(_inputValid);};

}; 

} //End namespace tse  

#endif //CALL_TREE_COMMAND_LINE_PROCESSOR_H
