from ValgrindFile import *
import os

class CTAnalyzer:
    def __init__(self, filePrefix, directory=os.getcwd()):
        files = os.listdir(directory)
        self.valgrindFiles = self.readFiles(files, filePrefix, directory)
        self.allMethods = self.getAllMethods()
        self.trees = self.getAllTrees()
        self.totalTimesCalled = self.computeTotalTimesCalled()
        self.totalCycles = self.computeTotalCycles()

    def readFiles(self, files, filePrefix, directory):
        valgrindFiles = []
        for file in files:
            if file.startswith(filePrefix) and file.endswith('.txt'):
                valgrindFiles.append(ValgrindFile(directory + '/' + file))
        return valgrindFiles

    def getAllMethods(self):
        allMethods = {}
        for file in self.valgrindFiles:
            for method in file.methodCalls:
                try:
                    allMethods[method.method.signature].append(method)
                except KeyError:
                    allMethods[method.method.signature] = []
                    allMethods[method.method.signature].append(method)
        return allMethods
                    
    def getAllTrees(self):
        trees = []
        map(lambda x: trees.append(x.getTree()), self.valgrindFiles)
        return trees

    def computeTotalTimesCalled(self):
        allTimes = {}
        for m in self.allMethods.values():
            for call in m:
                for callee in call.callees.values():
                    if callee.signature in allTimes.keys():
                        allTimes[callee.signature] += callee.timesCalled
                    else:
                        allTimes[callee.signature] = callee.timesCalled
        return allTimes

    def computeTotalCycles(self):
        totalCycles = {}
        for m in self.allMethods.values():
            for call in m:
                if call.method.signature in totalCycles.keys():
                    totalCycles[call.method.signature] += call.method.totalCycles
                    totalCycles[call.method.signature+' self'] += call.selfCycles()
                else:
                    totalCycles[call.method.signature] = call.method.totalCycles
                    totalCycles[call.method.signature+' self'] = call.selfCycles()
        return totalCycles
                    

class XMLExport:
    def __init__(self, analyzer):
        self.analyzer = analyzer

    def exportTo(self, filename):
        output = file(filename, 'w')
        output.write('<?xml version="1.0" encoding="utf-8" ?>\n')
        output.write('<PerformanceData>\n')
        self.writeTrees(output)
        self.writeAll(output)
        output.write('</PerformanceData>\n')
        output.close()

    def writeTrees(self, output):
        counter = 0
        for file in self.analyzer.valgrindFiles:
            if not file.hasMethodCalls():
                return
            output.write('<Tree id="' + str(counter) + '">\n')
            output.write('  <Root>\n')
            #output.write('    <FunctionSignature>')
            #output.write(file.methodCalls[0].method.signature)
            #output.write('</FunctionSignature>\n')
            output.write('    <FunctionId>')
            output.write(str(file.methodCalls[0].method.hash))
            output.write('</FunctionId>\n')
            output.write('  </Root>\n')
            self.writeAllFunctions(file, output, prefix='  ')
            output.write('</Tree>\n')
            counter += 1

    def writeAll(self, output):
        output.write('  <AllFunctions>\n');
        for methodCall in self.analyzer.allMethods.keys():
            self.writeProcessFunction(methodCall, output, '    ')
        output.write('  </AllFunctions>\n');

    def writeAllFunctions(self, valgrindFile, output, prefix=''):
        output.write(prefix + '<Functions>\n')
        all = valgrindFile.getAllMethods()
        for methodCall in all.values():
            self.writeThreadFunction(methodCall.method.signature, \
                                     all, \
                                     output, \
                                     prefix+'  ')
            #output.write(prefix + '  <FunctionSignature>')
            #output.write(self.translateEntities(methodCall.method.signature))
            #output.write(prefix + '</FunctionSignature>\n')
        output.write(prefix + '</Functions>\n')

    def writeProcessFunction(self, methodSig, output, prefix=''):
        methodCall = self.analyzer.allMethods[methodSig][0]
        totalCalled, totalCycles = self.getTotals(methodSig)
        self.writeFunction(output, prefix, methodCall, totalCalled, totalCycles)

    def writeThreadFunction(self, methodSig, group, output, prefix=''):
        methodCall = group[methodSig]
        self.writeFunction(output, prefix, methodCall)

    def writeFunction(self, output, prefix, methodCall, number=-1, cycles=-1):
        if number == -1:
            totalCalled = methodCall.method.totalTimesCalled
        else:
            totalCalled = number
        if cycles == -1:
            totalCycles = methodCall.method.totalCycles
        else:
            totalCycles = cycles
        selfCycles = methodCall.selfCycles()
        
        bigPrefix = prefix + '  '
        output.write(prefix + '<Function id="')
        output.write(str(methodCall.method.hash)+'">\n')
        output.write(bigPrefix + '<Namespace>')
        output.write(self.translateEntities(methodCall.method.namespace))
        output.write('</Namespace>\n')
        
        output.write(bigPrefix + '<ClassName>')
        output.write(self.translateEntities(methodCall.method.classname))
        output.write('</ClassName>\n')
        
        output.write(bigPrefix + '<FunctionName>')
        output.write(self.translateEntities(methodCall.method.function))
        output.write('</FunctionName>\n')
        
        output.write(bigPrefix + '<FunctionSignature>')
        output.write(self.translateEntities(methodCall.method.signature))
        output.write('</FunctionSignature>\n')

        output.write(bigPrefix + '<Filename>')
        output.write(self.translateEntities(methodCall.method.filename))
        output.write('</Filename>\n')
        
        output.write(bigPrefix + '<NumberTimesCalled>')
        output.write(str(totalCalled))
        output.write('</NumberTimesCalled>\n')
        
        output.write(bigPrefix + '<TotalCycles>')
        output.write(str(totalCycles))
        output.write('</TotalCycles>\n')
        
        output.write(bigPrefix + '<SelfCycles>')
        output.write(str(selfCycles))
        output.write('</SelfCycles>\n')
        
        output.write(bigPrefix + '<FunctionCallTree>\n')
        for child in methodCall.callees.values():
            output.write(bigPrefix + '  <FunctionChild>\n')
            if child.signature in self.analyzer.allMethods:
                output.write(bigPrefix + '    <FunctionId>')
                output.write(str(child.hash))
                output.write('</FunctionId>\n')
            else:
                output.write(bigPrefix + '    <FunctionSignature>')
                output.write(self.translateEntities(child.signature))
                output.write('</FunctionSignature>\n')
            output.write(bigPrefix + '    <NumberTimesCalled>')
            output.write(str(child.timesCalled))
            output.write('</NumberTimesCalled>\n')
            output.write(bigPrefix + '    <Cycles>')
            output.write(str(child.cycles))
            output.write('</Cycles>\n')
            output.write(bigPrefix + '  </FunctionChild>\n')
        output.write(bigPrefix + '</FunctionCallTree>\n')
        output.write(prefix + '</Function>\n')

    def getTotals(self, methodSig):
        try:
            totalCalled = self.analyzer.totalTimesCalled[methodSig]
        except KeyError:
            totalCalled = 0
        try:
            totalCycles = self.analyzer.totalCycles[methodSig]
        except KeyError:
            totalCycles = 0
        return totalCalled, totalCycles

    def getThreadTotals(self, methodSig, group):
        try:
            totalCalled = self.analyzer.totalTimesCalled[methodSig]
        except KeyError:
            totalCalled = 0
        try:
            totalCycles = self.analyzer.totalCycles[methodSig]
        except KeyError:
            totalCycles = 0
        return totalCalled, totalCycles

    def translateEntities(self, string):
        translation = {'&':' &amp; ',
                       '<':' &lt; ',
                       '>':' &gt; ',
                       '"':' &quot; ',
                       "'":' &apos; '}
        def replacer(match):
            return translation[match.group(0)]
        
        pattern = re.compile('(&)|(<)|(>)|(")|(\')')
        return re.sub(pattern, replacer, string)

