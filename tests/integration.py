#!/usr/bin/env python3
# integration test for the webserver
# target platform: Ubuntu14.04 with python 3.4.3

import os
import subprocess
import sys
import time
import urllib.request
from threading import Thread

# display settings, in case you want to debug a test
SHOW_WEBSERVER_OUTPUT_INLINE = False
SHOW_GENERATED_CONFIG_INLINE = False

def getScriptPath():
    return os.path.dirname(os.path.realpath(sys.argv[0]))

def getProjectRoot():
    return os.path.dirname(getScriptPath())

def getPathInProject(*path):
    return os.path.join(getProjectRoot(), *path)

RED_ESCAPE = '\033[31m'
GREEN_ESCAPE = '\033[32m'
CLR_ESCAPE = '\033[0m'

def die(error):
    """ test apparatus failed entirely...
        print error and exit
    """
    print("%s%s%s" % (RED_ESCAPE, error, CLR_ESCAPE))
    sys.exit(1)

def buildBinary():
    buildProcess = subprocess.Popen(['make'])
    buildProcess.wait()
    if buildProcess.returncode != 0:
        die("build failed")

def makeHandlerString(uri, name, l): # l: list of config for handler
    handlerString = 'path %s %s {\n' % (uri, name)
    for s in l:
        handlerString += s + ';\n'
    handlerString += '\n}'
    return handlerString

def makeDefaultHandlerString(name, l): # l: list of config for handler
    handlerString = 'default %s {\n' % (name)
    for s in l:
        handlerString += s + ';\n'
    handlerString += '\n}'
    return handlerString


class TemporaryConfigFile:
    """ context manager for creating a temporary config file on disk """
    def __init__(self, config):
        self.config = config

    def __enter__(self):
        handlerStrings = [makeHandlerString(uri, name, l) for (uri,name,l) in self.config['handlers']]
        if 'default' in self.config:
            defaultHandlerString = makeDefaultHandlerString(self.config['default'][0], self.config['default'][1])
        else:
            defaultHandlerString = ''

        threadsString = ""
        if 'threads' in self.config:
            threadsString = 'threads %d;' %self.config['threads']
        cnfg = (
                'port %d;\n'
                '%s\n'
                '%s\n'
                '%s\n'
                % (self.config['port'], threadsString, '\n'.join(handlerStrings), defaultHandlerString,)
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
    SKIP = "SKIP" # special value for tests to return when the test should be skipped

    def test_basic():
        """ test that server can run and receive requests,
            and for now, that the request is correctly echo'd back
        """
        config = {
                'filename': 'temp_config',
                'port': 8080,
                'handlers': [
                    ('/echo', 'EchoHandler', []),
                    ('/static', 'StaticHandler', ['root testFiles1']),
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

    def test_not_found():
        """ test the NotFound handler
        """
        config = {
                'filename': 'temp_config',
                'port': 8080,
                'handlers': [
                    ('/badness', 'NotFoundHandler', []),
                    ('/ba', 'EchoHandler', []),
                    ],
                }

        with TemporaryConfigFile(config) as filepath:
            with WebserverRunningContext(filepath):
                code, content = makeWebserverRequest(config, '/ba')
                if not code: return "request failed"
                if code != 200:
                    return "expected 200 OK, but got: " + str(code) + ": " + content

                code, content = makeWebserverRequest(config, '/badness')
                if not code: return "request failed"
                if code != 404:
                    return "expected 404 Not Found, but got: " + str(code) + ": " + content

                return Test.PASS

    def test_status():
        """ test the Status Handler
        """
        config = {
                'filename': 'temp_config',
                'port': 8080,
                'handlers': [
                    ('/status', 'StatusHandler', []),
                    ('/echo', 'EchoHandler', []),
                    ],
                }

        with TemporaryConfigFile(config) as filepath:
            with WebserverRunningContext(filepath):
                for i in range(13):
                    code, content = makeWebserverRequest(config, '/echo')
                    if not code: return "request failed"
                    if code != 200:
                        return "expected 200 OK, but got: " + str(code) + ": " + content

                for i in range(17):
                    code, content = makeWebserverRequest(config, '/bad')
                    if not code: return "request failed"
                    if code != 404:
                        return "expected 404 Not Found, but got: " + str(code) + ": " + content

                code, content = makeWebserverRequest(config, '/status')
                if not code: return "request failed"
                if code != 200:
                    return "expected 200 OK, but got: " + str(code) + ": " + content

                content = content.decode('utf-8')
                expectedOutputChunks = {
                        '/echo',
                        '13',
                        '/bad',
                        '17',
                        'EchoHandler',
                        'StatusHandler',
                        'default',
                        'NotFoundHandler',
                        }

                for chunk in expectedOutputChunks:
                    if chunk not in content:
                        return "response does not contain expected '%s'" % chunk

                return Test.PASS

    def test_default():
        """ test the default handler
        """
        config = {
                'filename': 'temp_config',
                'port': 8080,
                'handlers': [
                    ('/bad', 'NotFoundHandler', []),
                    ('/echo', 'EchoHandler', []),
                    ],
                'default': ('EchoHandler', []),
                }

        with TemporaryConfigFile(config) as filepath:
            with WebserverRunningContext(filepath):
                code, content = makeWebserverRequest(config, '/ba')
                if not code: return "request failed"
                if code != 200:
                    return "expected 200 OK, but got: " + str(code) + ": " + content

                code, content = makeWebserverRequest(config, '/badness')
                if not code: return "request failed"
                if code != 404:
                    return "expected 404 Not Found, but got: " + str(code) + ": " + content


                return Test.PASS

    def test_port():
        """ test that server can run on arbitary ports """
        config = {
                'filename': 'temp_config',
                'port': 14151,
                'handlers': [
                    ('/echo', 'EchoHandler', []),
                    ('/static', 'StaticHandler', ['root testFiles1']),
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
                'handlers': [
                    ('/echo', 'EchoHandler', []),
                    ('/static', 'StaticHandler', ['root testFiles1']),
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
        """ test that multiple handlers work """

        config = {
                'filename': 'temp_config',
                'port': 8080,
                'handlers': [
                    ('/echo', 'EchoHandler', []),
                    ('/static', 'StaticHandler', ['root testFiles1']),
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

    def test_basic_proxy():
        """ test that ProxyHandler works as expected with a basic
        (non-redirecting) webpage """

        print("Warning: skipping flakey proxy test connecting to remote host")
        return Test.SKIP

        remote_host = 'ipecho.net'

        config = {
                'filename': 'temp_config',
                'port': 8080, # note: larger than max port of 65535
                'handlers': [
                    ('/echo', 'EchoHandler', []),
                    ('/static', 'StaticHandler', ['root testFiles1']),
                    ('/proxy', 'ProxyHandler',
                        ['remote_host %s' % remote_host,
                        'remote_port 80']),
                    ]
                }

        with TemporaryConfigFile(config) as filepath:
            with WebserverRunningContext(filepath):
                code, content = makeWebserverRequest(config, '/proxy/plain')
                if not code: return "request failed"
                if code != 200:
                    return "expected 200 OK, but got: " + str(code) + ": " + content

                content = content.decode('utf-8')
                if len(content) < 7 or len(content) > 15:
                    return "expected IP address, but got: " + str(content)
                return Test.PASS

    def test_redirecting_proxy():
        """ test that ProxyHandler works as expected with a redirecting webpage """
        config = {
                'filename': 'temp_config',
                'port': 8080, # note: larger than max port of 65535
                'handlers': [
                    ('/echo', 'EchoHandler', []),
                    ('/static', 'StaticHandler', ['root testFiles1']),
                    ('/proxy', 'ProxyHandler',
                        ['remote_host google.com',
                        'remote_port 80']),
                    ]
                }

        with TemporaryConfigFile(config) as filepath:
            with WebserverRunningContext(filepath):
                code, content = makeWebserverRequest(config, '/proxy')
                if not code: return "request failed"
                if code != 200:
                    return "expected 200 OK, but got: " + str(code) + ": " + content

                if len(content) == 0:
                    return "got empty response"
                return Test.PASS

    def test_multithreaded():
        """ tests if multithreading works correctly
        """
        sec_to_ms = 1000000
        delaytime = 0.2 * sec_to_ms
        config = {
                'filename': 'temp_config',
                'port': 8080,
                'handlers': [
                    ('/delay','DelayHandler',['delay %s'%delaytime]),
                    ],
                }

        with TemporaryConfigFile(config) as filepath:
            def thread_func():
                code, content = makeWebserverRequest(config, '/delay')
                if not code: return "request failed"
                if code != 200:
                    return "expected 200 OK, but got: " + str(code) + ": " + content
            with WebserverRunningContext(filepath):
                threads = []
                for i in range(5):
                    thread = Thread(target = thread_func)
                    threads.append(thread)
                #timing this loop
                time1 = time.time()
                for thread in threads:
                    thread.start()
                for thread in threads:
                    thread.join()
                time2 = time.time()
                timems = delaytime/sec_to_ms
                print(time2-time1)
                if(time2-time1> 2*timems):
                    return "multithreading didnt work"

                return Test.PASS



    def test_multithreaded2():
        """ tests if multithreading works correctly
        """
        sec_to_ms = 1000000
        delaytime = 1.0 * sec_to_ms
        config = {
                'filename': 'temp_config',
                'port': 8080,
                'threads': 2,
                'handlers': [
                    ('/delay','DelayHandler',['delay %s'%delaytime]),
                    ],
                }

        with TemporaryConfigFile(config) as filepath:
            def thread_func():
                code, content = makeWebserverRequest(config, '/delay')
                if not code: return "request failed"
                if code != 200:
                    return "expected 200 OK, but got: " + str(code) + ": " + content
            with WebserverRunningContext(filepath):
                threads = []
                for i in range(2):
                    thread = Thread(target = thread_func)
                    threads.append(thread)
                #timing this loop
                time1 = time.time()
                for thread in threads:
                    thread.start()
                for thread in threads:
                    thread.join()
                time2 = time.time()
                timems = delaytime/sec_to_ms
                print(time2-time1)
                if(time2-time1> 1.5*timems):
                    return "multithreading didnt work"

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
        print('%s[ RUN      ]%s  %s' % (GREEN_ESCAPE, CLR_ESCAPE, testName))
        for testInfoLine in testInfo.split('\n'):
            print('              %s' % testInfoLine.strip())
        try:
            error = test()
        except Exception as e:
            error = str(e)
        if error == Test.SKIP:
            pass
        elif error:
            print('%s[   FAILED ]%s %s' % (RED_ESCAPE, CLR_ESCAPE, testName))
            for testInfoLine in testInfo.split('\n'):
                print("%s >>> %s%s" % (RED_ESCAPE, CLR_ESCAPE, testInfoLine.strip()))
            for errorLine in error.split('\n'):
                print("%s >>> %s%s" % (RED_ESCAPE, CLR_ESCAPE, errorLine.strip()))
            failed += 1
        else:
            print('%s[       OK ]%s %s' % (GREEN_ESCAPE, CLR_ESCAPE, testName))
            passed += 1

    total = passed + failed
    print('%s[----------]%s' % (GREEN_ESCAPE, CLR_ESCAPE))
    print('%s[==========]%s Ran %d tests' % (GREEN_ESCAPE, CLR_ESCAPE, total))
    print('%s[  PASSED  ]%s %d of %d tests.' % (GREEN_ESCAPE, CLR_ESCAPE, passed, total))
    if failed > 0:
        print('%s[  FAILED  ]%s %d of %d tests.' % (RED_ESCAPE, CLR_ESCAPE, failed, total))

    if failed > 0:
        exit(-1)

if __name__ == '__main__':
    if not '--no-build' in sys.argv:
        buildBinary()
    runTests()
