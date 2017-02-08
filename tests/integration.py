#!/usr/bin/env python3
# integration test for the webserver
# target platform: Ubuntu14.04 with python 3.4.3

import os
import subprocess
import sys
import time
import urllib.request

# display settings, in case you want to debug a test
SHOW_WEBSERVER_OUTPUT_INLINE = True
SHOW_GENERATED_CONFIG_INLINE = False

def getScriptPath():
    return os.path.dirname(os.path.realpath(sys.argv[0]))

def getProjectRoot():
    return os.path.dirname(getScriptPath())

def getPathInProject(*path):
    return os.path.join(getProjectRoot(), *path)

RED_ESCAPE = '\033[31m'
GREEN_ESCAPE = '\033[32m'
CLR_ESACPE = '\033[0m'

def die(error):
    """ test apparatus failed entirely...
        print error and exit
    """
    print("%s%s%s" % (RED_ESCAPE, error, CLR_ESACPE))
    sys.exit(1)

def buildBinary():
    buildProcess = subprocess.Popen(['make'])
    buildProcess.wait()
    if buildProcess.returncode != 0:
        die("build failed")

def makeModuleString(m): #dictionary cotaining all values
    modString = 'module {\n'
    for key, value in m.items():
        modString += key + ' = ' + value + '; \n'
    modString += '\n}'
    return modString



class TemporaryConfigFile:
    """ context manager for creating a temporary config file on disk """
    def __init__(self, config):
        self.config = config

    def __enter__(self):
        modStrings = [makeModuleString(m) for m in self.config['modules']]
        cnfg = (
                'server {\n'
                '    port = %d;\n'
                '    %s           '
                '}\n'
                % (self.config['port'], '\n'.join(modStrings))
                )

        if SHOW_GENERATED_CONFIG_INLINE:
            print(cnfg)

        # create temp file inside a temporary directory, to prevent accidental deletions
        TMP_DIR_NAME = 'tmp'
        self.tmpDirPath = getPathInProject(TMP_DIR_NAME)
        os.makedirs(TMP_DIR_NAME, exist_ok=True)
        self.realFilepath = getPathInProject(TMP_DIR_NAME, self.config['filename'])

        self.fp = open(self.realFilepath, 'w')
        self.fp.write(cnfg)
        self.fp.flush() # force config to be written now

        return self.realFilepath

    def __exit__(self, exceptionType, exceptionValue, traceback):
        self.fp.close()
        os.remove(self.realFilepath)
        os.rmdir(self.tmpDirPath)

def runWebserver(*args):
    """ run webserver with args in a new process
        make sure you terminate the process
        if you use without WebserverRunningContext
    """
    WEBSERVER_EXECUTABLE = getPathInProject('webserver')
    command = [WEBSERVER_EXECUTABLE]
    command.extend(list(args))
    stdout = subprocess.DEVNULL # eat webserver's output
    if SHOW_WEBSERVER_OUTPUT_INLINE:
        stdout = None # use same stdout as this program for webserver's output

    process = subprocess.Popen(command, stdout=stdout, stderr=stdout)
    time.sleep(0.1) # give time for webserver to open
    return process


class WebserverRunningContext:
    """ context manager for running an instance of the webserver """
    def __init__(self, configName):
        self.configName = configName

    def __enter__(self):
        # start running webserver with given config
        self.process = runWebserver(self.configName)
        if self.process.poll():
            raise Exception("Webserver exited with status %d" % self.process.returncode)

    def __exit__(self, exceptionType, exceptionValue, traceback):
        self.process.terminate() # kill server
        self.process.wait() # hang on, are you sure it's dead?

def makeWebserverRequest(config, path, headers={}):
    """ given a config, make an HTTP request to given path
        note: expects running webserver and existing config
    """

    port = config['port']

    url = 'http://localhost:%d%s' % (port, path)
    print("    requesting %s" % url)

    respcode = None
    respbody = None
    try:
        headers = headers or {'User-Agent': 'MyUserAgent'}
        req = urllib.request.Request(url=url, headers=headers)
        respbody = urllib.request.urlopen(req).read()
        respcode = 200 # TODO: is this valid? HTTP301 etc probably don't throw?
    except urllib.error.HTTPError as error:
        respcode = error.code
        respbody = error.reason
    finally:
        return (respcode, respbody)


class Test:
    """ tests will be automatically collected from this container class """
    PASS = None

    def test_basic():
        """ test that server can run and recieve requests,
            and for now, that the request is correctly echo'd back
        """
        config = {
                'filename': 'temp_config',
                'port': 8080,
                'modules': [
                            {'type': 'echo', 'path': '/echo'},
                            {'type':'static', 'path':'/static', 'filebase':'testFiles1'}
                           ],
                }

        with TemporaryConfigFile(config) as filepath:
            with WebserverRunningContext(filepath):
                code, content = makeWebserverRequest(config, '/echo')
                if not code: return "request failed"
                if code != 200:
                    return "expected 200 OK, but got: " + str(code) + ": " + content

                content = content.decode('utf-8')
                expectedOutputLines = {
                        'GET /echo HTTP/1.1',
                        'User-Agent: MyUserAgent',
                        'Host: localhost:%d' % config['port'],
                        '', # request ends in a blank line

                        # requests from urllib contain the following additional headers,
                        # however we just want to check that our server seems to be echoing the request,
                        # not depend explicitly on what format urllib requests in...
                        # 'Accept-Encoding: identity',
                        # 'Connection: close',
                        }

                for line in expectedOutputLines:
                    if line not in content.split('\r\n'):
                        return "response does not contain expected '%s'" % line

                return Test.PASS

    def test_port():
        """ test that server can run on arbitary ports """
        config = {
                'filename': 'temp_config',
                'port': 14151,
                'modules': [
                            {'type': 'echo', 'path': '/echo'},
                            {'type':'static', 'path':'/static', 'filebase':'testFiles1'}
                           ],
                }

        with TemporaryConfigFile(config) as filepath:
            with WebserverRunningContext(filepath):
                code, content = makeWebserverRequest(config, '/echo')
                if not code: return "request failed"
                if code != 200:
                    return "expected 200 OK, but got: " + str(code) + ": " + content

                return Test.PASS

    def test_invalid_config():
        """ test that server fails when given no config """
        process = runWebserver()
        if process.poll():
            return Test.PASS
        else:
            process.terminate() # don't leave the process open
        return "webserver started unexpectedly when given bad input"


    def test_invalid_port():
        """ test that server won't run on invalid ports """
        config = {
                'filename': 'temp_config',
                'port': 67536, # note: larger than max port of 65535
                'modules': [
                            {'type': 'echo', 'path': '/echo'},
                            {'type':'static', 'path':'/static', 'filebase':'testFiles1'}
                           ]
                }

        with TemporaryConfigFile(config) as filepath:
            process = runWebserver(filepath)
            if process.poll():
                return Test.PASS
            else:
                process.terminate() # don't leave the process open
            return "webserver started unexpectedly when given bad input port"

    def test_multiple():
        """ test that multiple modules work """

        config = {
                'filename': 'temp_config',
                'port': 8080,
                'modules': [
                            {'type': 'echo', 'path': '/echo'},
                            {'type':'static', 'path':'/static', 'filebase':'testFiles1'}
                           ],
                }

        with TemporaryConfigFile(config) as filepath:
            with WebserverRunningContext(filepath):
                code, content = makeWebserverRequest(config, '/echo')
                if not code: return "request failed"
                if code != 200:
                    return "expected 200 OK, but got: " + str(code) + ": " + content

                content = content.decode('utf-8')
                expectedOutputLines = {
                        'GET /echo HTTP/1.1',
                        'User-Agent: MyUserAgent',
                        'Host: localhost:%d' % config['port'],
                        '', # request ends in a blank line
                        }
                for line in expectedOutputLines:
                    if line not in content.split('\r\n'):
                        return "response does not contain expected '%s'" % line

                code, content = makeWebserverRequest(config, '/static/cat.gif')
                if not code: return "request failed"
                if code != 200:
                    return "expected 200 OK, but got: " + str(code) + ": " + content


                return Test.PASS


def runTests():
    # get all methods of Test container
    # sorted so they're in a consistant order
    tests = sorted(list(filter(callable, Test.__dict__.values())), key=lambda x: x.__name__)

    passed = 0
    failed = 0
    for test in tests:
        testName = test.__name__
        testInfo = test.__doc__.strip() if test.__doc__ else ''
        print('[ RUN      ]  %s' % testName)
        for testInfoLine in testInfo.split('\n'):
            print('              %s' % testInfoLine.strip())
        try:
            error = test()
        except Exception as e:
            error = str(e)
        if error:
            print('%s[   FAILED ]%s %s' % (RED_ESCAPE, CLR_ESACPE, testName))
            for testInfoLine in testInfo.split('\n'):
                print("%s >>> %s%s" % (RED_ESCAPE, CLR_ESACPE, testInfoLine.strip()))
            for errorLine in error.split('\n'):
                print("%s >>> %s%s" % (RED_ESCAPE, CLR_ESACPE, errorLine.strip()))
            failed += 1
        else:
            print('%s[       OK ]%s %s' % (GREEN_ESCAPE, CLR_ESACPE, testName))
            passed += 1

    total = passed + failed
    print('[----------]')
    print('[==========] Ran %d tests' % (total))
    print('%s[  PASSED  ]%s %d of %d tests.' % (GREEN_ESCAPE, CLR_ESACPE, passed, total))
    if failed > 0:
        print('%s[  FAILED  ]%s %d of %d tests.' % (RED_ESCAPE, CLR_ESACPE, failed, total))

    if failed > 0:
        exit(-1)

if __name__ == '__main__':
    if not '--no-build' in sys.argv:
        buildBinary()
    runTests()
