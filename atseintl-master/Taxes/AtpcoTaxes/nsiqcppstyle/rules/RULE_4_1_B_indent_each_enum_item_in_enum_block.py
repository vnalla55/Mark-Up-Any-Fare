"""
Indent the each enum item in the enum block.

== Violation ==

    enum A {
    A_A,  <== Violation
    A_B   <== Violation
    }


== Good ==

    enum A {
        A_A,     <== Good
        A_B
    }  
"""
from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *

def RunRule(lexer, typeName, typeFullName, decl, contextStack, typeContext) :
    if not decl and typeName == "ENUM" and typeContext != None:
        column = GetIndentation(lexer.GetCurToken())
        lexer._MoveToToken(typeContext.startToken)
        t2 = typeContext.endToken
        while(True) :
            t = lexer.GetNextTokenSkipWhiteSpaceAndCommentAndPreprocess()
            if t == None or t == t2 :
                break
            if GetRealColumn(t) <= (column + 1):
                nsiqcppstyle_reporter.Error(t, __name__, "Enum block should be indented. But the token(%s) seems to be unindented" % t.value);
ruleManager.AddTypeNameRule(RunRule)




###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddTypeNameRule(RunRule)
    def test1(self):
        self.Analyze("test/thisFile.c", 
"""
enum A {
}
""")
        assert not CheckErrorContent(__name__)    
    def test2(self):
        self.Analyze("test/thisFile.c", 
"""
enum C {
    AA, BB
}
""")
        assert not CheckErrorContent(__name__)    
    def test3(self):
        self.Analyze("test/thisFile.c", 
"""
enum C {
AA = 4, 
    BB
}
""")
        assert CheckErrorContent(__name__)    
    def test4(self):
        self.Analyze("test/thisFile.c", 
"""
enum C {
    AA = 4
,BB
}
""")
        assert CheckErrorContent(__name__)    
    def test5(self):
        self.Analyze("test/thisFile.c", 
"""
enum C {
    AA = 4
/** HELLO */
    ,BB
}
""")
        assert not  CheckErrorContent(__name__)    
    def test6(self):
        self.Analyze("test/thisFile.c", 
"""
typedef enum  {
    AA = 4
/** HELLO */
    ,BB
} DD
""")
        assert not  CheckErrorContent(__name__)    
    def test7(self):
        self.Analyze("test/thisFile.c", 
"""      
typedef enum
{
  SERVICE,
  SERVER,
  BROKER,
  MANAGER,
  REPL_SERVER,
  REPL_AGENT,
  UTIL_HELP,
  UTIL_VERSION,
  ADMIN
} UTIL_SERVICE_INDEX_E;
""") 
        assert not  CheckErrorContent(__name__)        