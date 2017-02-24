# Just Enough
## Simple HTTP Server

[![Build Status](https://travis-ci.org/UCLA-CS130/Just-Enough.svg?branch=master)](https://travis-ci.org/UCLA-CS130/Just-Enough)

## To build:
* `make` will build the webserver executable
* `make test` will run unit tests
* `make integration` will run integration tests
* `make test-all` will run both unit and integration tests
* `make clean` will remove all .o, .gcno, etc. files.
	If your build is not working, try cleaning and rebuilding.
	Make may not always notice dependency changes, especially if changing branches.
* `make coverage` will run unit tests and track coverage information with gcov.
	Note: coverage statistics will be printed and coverage files will be placed in the root directory.
	Note: this will first run `make clean` and rebuild with special flags. After looking at the .gcov files,
	you should run `make clean` before building again.

## Running:
run `./webserver example_config` to run using `example_config`. We recommend you create local config files
to test with that are not added to source control. `example_config` we try to have as a simple basic config example.

## File Layout:
All server source code is located in the `src` directory.
Unit tests are in the `tests` directory.
When adding new unit tests, use `*_test.cc` for each `src/*.cc`.
Integration tests are all within `tests/integration.py`, and more can be easily added by
adding method definitions in the `Test` class.
Please try to keep test coverage ~90% when possible.

`Webserver` (webserver.h) handles making connections and handling requests.
One instance is created when the server is started. It is not enforced as a singleton,
but we only use one instance and assume so in some places.
However, the most recently constructed instance can be globally found using Webserver::instance,
as needed for e.g. the StatusHandler.

`Options` (options.h) is in charge of making sense of the config file.
It validates and loads information like the port and Handlers.

`RequestHandler` (request_handler.h) is the base class for Handlers, as defined by the API from class.
There are only a few small changes from the given API: a virtual destructor, a `std::string type()`,
and a static `CreateByName` to statically register handlers.
To add a new handler, use the `REGISTER_REQUEST_HANDLER` macro with the classname.

`Request` and `Response` (also in request_handler.h) are similar to the given API. Refer to the header to see changes.

