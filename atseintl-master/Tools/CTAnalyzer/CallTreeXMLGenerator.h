#ifndef CALL_TREE_XML_GENERATOR
#define CALL_TREE_XML_GENERATOR

#include <string>

namespace tse {

class CallTreeHeader;
class CallTreeMethodTree;
class CallTreeDataAnalyzer;
class CallTreeMethod;

class CallTreeXMLGenerator
{
public:
    CallTreeXMLGenerator( CallTreeHeader* header,
                          const std::vector<CallTreeMethodTree*>* trees,
                          CallTreeDataAnalyzer* analyzer)
        : _header(header),
          _trees(trees),
          _analyzer(analyzer)
        {}

    std::string generate();

protected:
    void generateHeader(std::ostream& output);

    void generateFunctions(std::ostream& output);

    void generateHotspots(std::ostream& output);

    void generateFunction( const CallTreeMethodTree* tree,
                           std::ostream& output);

    void generateFunctionInfo( const CallTreeMethod* method,
                               std::ostream& output );

    std::string translateXMLEntities(std::string raw);
    
private:
    CallTreeHeader* _header;
    const std::vector<CallTreeMethodTree*>* _trees;
    CallTreeDataAnalyzer* _analyzer;
};

} // namespace

#endif // CALL_TREE_XML_GENERATOR
