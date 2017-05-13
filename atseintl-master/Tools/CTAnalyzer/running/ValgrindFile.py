import re
import string

class ValgrindFile:
    def __init__(self, filename):
        annotatedFile = self.loadFile(filename)
        header, self.rest = self.headerTree(annotatedFile)
        self.header = Header(header)
        if self.rest is not []:
	    self.trees = self.divideIntoTrees(self.rest)
    	    self.methodCalls = self.getMethodCalls(self.trees)
    	    self.computePerThreadStatistics()
        else:
            self.trees = self.methodCalls = []

    def hasMethodCalls(self):
	return len(self.methodCalls) > 0
    
    def loadFile(self, filename):
        return file(filename).readlines()

    def headerTree(self, filelines):
        counter = 0
        for line in filelines:
            if self.callTreeFormat(line):
                return filelines[:counter], filelines[counter:]
            counter += 1
        # if I get here, there is only header in the file
        return filelines, []

    def callTreeFormat(self, line):
        expr = re.compile('^\s*(\d{1,3},?)+\s+\*')
        return expr.match(line) is not None

    def divideIntoTrees(self, lines):
        counter = 0
        oldcounter = 0
        divided = []
        for line in lines:
            if line == '\n':
                new = lines[oldcounter:counter]
                if len(new) > 0:
                    divided.append(new)
                    oldcounter = counter + 1 # To omit empty line
            counter += 1
        return divided

    def getMethodCalls(self, trees):
        calls = []
        map(lambda x: calls.append(MethodCall(x)), trees)
        return calls

    def getTree(self):
        allMethods = self.getAllMethods()
	tree = []
	if len(self.methodCalls)>0:
	    tree = CallTree(self.methodCalls[0], allMethods, [])
        return tree

    def getAllMethods(self):
        allMethods = {}
        for method in self.methodCalls:
            allMethods[method.method.signature] = method
        return allMethods

    def computePerThreadStatistics(self):
        allMethods = self.getAllMethods()
        perThreadCalls = {}
        perThreadCycles = {}
        for method in allMethods.values():
            for callee in method.callees.values():
                try:
                    perThreadCalls[callee.signature] += callee.timesCalled
                except KeyError:
                    perThreadCalls[callee.signature] = callee.timesCalled
                try:
                    perThreadCycles[callee.signature] += callee.cycles
                except KeyError:
                    perThreadCycles[callee.signature] = callee.cycles
        for method in allMethods.values():
            try:
                method.method.totalTimesCalled = perThreadCalls[method.method.signature]
                method.method.totalCycles = perThreadCycles[method.method.signature]
            except KeyError:
                pass


class Header:
    def __init__(self, headerLines):
        self.fields = {}
        for line in headerLines:
            if line == '\n':
                continue
            if line[0] == '-': # probably line with dashes only
                continue
            if line.find(':') == -1: # Not parameter: value 
                continue
            values = line.split(':')
            self.fields[string.strip(values[0])] = string.strip(values[1])

            

class MethodCall:
    def __init__(self, treelines):
        self.method = Caller(treelines[0])
        self.callees = self.getCallees(treelines[1:])

    def getCallees(self, lines):
        callees = {}
        for line in lines:
            callee = Callee(line)
            callees[callee.signature] = callee
        return callees

    def selfCycles(self):
        selfCycles = self.method.totalCycles
        for callee in self.callees.values():
            selfCycles -= callee.cycles
        return selfCycles


class Method:
    def stringCallsToNumber(self, stringCalls):
        commas = re.compile(',')
        return long(commas.sub('',stringCalls))

    def getMethodInfo(self, aLine):
        linePattern = self.getRegexp()
        matched = linePattern.match(aLine)
        if matched is None:
            raise Exception, 'Invalid method line'
        return matched.groups()

    def divideName(self, name):
        nameParams = string.split(name, '(')
        self.divideParams(nameParams)
        namespaces = string.split(nameParams[0], '::')
        self.divideNamespaces(namespaces)

    def divideParams(self, nameParams):
        self.params = []
        if len(nameParams) < 2:
            return
        params = nameParams[1][:-1] # chopping off closing parenthesis
        p = string.split(params, ',')
        for param in p:
            self.params.append(string.strip(param))

    def divideNamespaces(self, namespaces):
        numberOfColons = len(namespaces)
        if numberOfColons >2 : #namespace::whatever::class::method
            self.namespace = namespaces[0]
            self.classname = string.join(namespaces[1:-1], '::')
            self.function = namespaces[-1]
        elif numberOfColons == 2: #class::method
            self.namespace = ''
            self.classname = namespaces[0]
            self.function = namespaces[1]
        elif numberOfColons == 1: # method, no colons
            self.function = namespaces[0]
            self.namespace = ''
            self.classname = ''
 

class Caller(Method):
    def __init__(self, aLine):
        groups = self.getMethodInfo(aLine)
        self.totalCycles = self.stringCallsToNumber(groups[0])
        self.filename = groups[2]
        self.signature = string.strip(groups[4])
        self.totalTimesCalled = 0
        self.divideName(self.signature)
        self.hash = hash(self.signature)
        
    def getRegexp(self):
        return re.compile('\s*((\d{1,3},?)+)\s+\*\s+((\w|[\.\?/\+-_])+?)\:([^\[\n]+)')

class Callee(Method):
    def __init__(self, aLine):
        groups = self.getMethodInfo(aLine)
        self.cycles = self.stringCallsToNumber(groups[0])
        self.filename = groups[2]
        self.signature = string.strip(groups[4])
        self.timesCalled = long(groups[5])
        self.divideName(self.signature)
        self.hash = hash(self.signature)

    def getRegexp(self):
        return re.compile('\s*((\d{1,3},?)+)\s+\>\s+((\w|[\.\?/\+-_])+?)\:(.*)\((\d*)x\)')


class CallTree:
    def __init__(self, root, allMethods, alreadyCreated=[]):
        self.method = root.method
        self.callees = []
        for callee in root.callees.values():
            try:
                if callee.signature in alreadyCreated:
                    self.callees.append(callee.signature)
                else:
                    alreadyCreated.append(callee.signature)
                    self.callees.append(CallTree(allMethods[callee.signature],\
                                                 allMethods, \
                                                 alreadyCreated))
            except KeyError:
                pass
        
