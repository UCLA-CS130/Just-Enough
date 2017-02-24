#include <string>
#include <fstream>
#include <memory>
#include <vector>

#include "options.h"

using std::string;
typedef std::map<string, string> stringMap;

const int MAX_PORT = 65535;
const int MIN_PORT = 1024;

const int KEY = 0;
const int VAL = 1;
const int HANDLER = 2;
const int DEFAULT_HANDLER = 1;

const int STATEMENT_SIZE = 2;
const int PATH_SIZE = 3;
const int DEFAULT_PATH_SIZE = 2;

// pass empty path to act as catch-all/default handler
bool Options::addHandler(string type, string path, const NginxConfig& handler_config) {
    // handler is owned by Options
    RequestHandler* handler = RequestHandler::CreateByName(type.c_str());
    if ( ! handler) {
        return false;
    }
    RequestHandler::Status status = handler->Init(path, handler_config);
    if (status != RequestHandler::OK) {
        delete handler;
        return false;
    }

    if (path == "") {
        defaultHandler = handler;
    } else {
        handlerMap[path] = handler;
    }
    return true;
}

bool Options::addPath(std::shared_ptr<NginxConfigStatement> handler_config) {
    if (handler_config->tokens_.size() != 3) {
        std::cerr << "Not a valid path statement. Need " << PATH_SIZE << " tokens.\n";
        return false;
    }

    string type = handler_config->tokens_[HANDLER];
    string path = handler_config->tokens_[VAL];

    return addHandler(type, path, *handler_config->child_block_);
}

bool Options::addDefaultHandler(std::shared_ptr<NginxConfigStatement> handler_config) {
    if (handler_config->tokens_.size() != DEFAULT_PATH_SIZE) {
        std::cerr << "Not a valid default statement. Need " << DEFAULT_PATH_SIZE << " tokens.\n";
        return false;
    }

    string type = handler_config->tokens_[DEFAULT_HANDLER];
    string path = ""; // accept any path

    return addHandler(type, path, *handler_config->child_block_);
}


bool Options::addPort(std::shared_ptr<NginxConfigStatement> port_config) {
    if (port_config->tokens_.size() != STATEMENT_SIZE) {
        std::cerr << "Incorrect number of tokens. (For port)\n";
        return false;
    }

    string port = port_config->tokens_[VAL];
    if ((unsigned int) std::stoi(port) > MAX_PORT || (unsigned int) std::stoi(port) < MIN_PORT) {
        std::cerr << "Invalid port number.\n";
        return false;
    }
    this->port = (unsigned short) std::stoi(port);
    return true;
}

bool Options::loadOptionsFromStream(std::istream* config_file) {

    NginxConfigParser parser;
    NginxConfig config;
    if (parser.Parse(config_file, &config) == false) {
        return false;
    }

    defaultHandler = nullptr;
    bool issetPort = false;
    for (unsigned int i =0; i < config.statements_.size(); i++) {
        std::shared_ptr<NginxConfigStatement> temp_config = config.statements_[i];

        // Sets port.
        if (temp_config->tokens_.size() > 1 && temp_config->tokens_[KEY] == "port") {
            if (issetPort) {
                std::cerr << "Multiple ports in config file.\n";
                return false;
            } else {
                issetPort = addPort(temp_config);
                if ( ! issetPort) {
                    return false;
                }
            }
        } 

        // Sets handler.
        else if (temp_config->tokens_.size() > 1 && temp_config->tokens_[KEY] == "path") { 
            if (addPath(temp_config) == false) {
                std::cerr << "Failed to add handler.\n";
                return false;
            }
        }

        // Sets default handler.
        else if (temp_config->tokens_.size() > 1 && temp_config->tokens_[KEY] == "default") {
            if (addDefaultHandler(temp_config) == false) {
                std::cerr << "Failed to add default handler.\n";
                return false;
            }
        }

        // For future statements in config file.
        else {
            std::cerr << "Unknown specifier " << temp_config->tokens_[KEY] << " .\n";
            return false;
        }
    }

    if ( ! issetPort) {
        std::cerr << "A port was not specified.\n";
        return false;
    }

    if ( ! defaultHandler) {
        std::cerr << "Default handler not specified, using NotFoundHandler.\n";
        NginxConfig empty_config;
        addHandler("NotFoundHandler", "", empty_config);
    }

    return true;
}

bool Options::loadOptionsFromFile(const char* filename) {
    //TODO: Does parse return error for bad filename
    std::ifstream config_file;
    config_file.open(filename);
    if ( ! config_file.good()) {
        std::cerr << "Failed to open config file.\n";
        return false;
    }

    bool val = loadOptionsFromStream(dynamic_cast<std::istream*>(&config_file));
    config_file.close();
    return val;
}

Options::~Options() {
    // free allocated handlers
    for (auto& h: handlerMap) {
        delete h.second;
    }
}
