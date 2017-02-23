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

const int STATEMENT_SIZE = 2;
const int PATH_SIZE = 3;

bool Options::addHandler(std::shared_ptr<NginxConfigStatement> handler_config) {
    std::shared_ptr<stringMap> params = std::make_shared<stringMap>();

    if(handler_config->tokens_.size() != PATH_SIZE) {
        std::cerr << "Not a valid path statement. Need " << PATH_SIZE << " tokens.\n";
        return false;
    }

    // TODO: do we need to validate handler and path here?
    string type = handler_config->tokens_[HANDLER];
    string path = handler_config->tokens_[VAL];

    for(size_t i = 0; i < handler_config->child_block_->statements_.size(); i++) {

        // For now, not allowed to go into another module/server within a module
        if(handler_config->child_block_->statements_[i]->tokens_.size() != STATEMENT_SIZE) {
            std::cerr << "Not a valid handler statement. Need " << STATEMENT_SIZE << " tokens.\n";
            return false;
        }

        string firstStatement = handler_config->child_block_->statements_[i]->tokens_[KEY];
        string secondStatement = handler_config->child_block_->statements_[i]->tokens_[VAL];

        if (secondStatement.size() > 1
                && secondStatement[0] == '"' && secondStatement[secondStatement.size()-1] == '"') {
            // remove surrounding quotes
            secondStatement = secondStatement.substr(1, secondStatement.size()-2);
        }

        params->insert(std::pair<string,string>(firstStatement, secondStatement));
    }

    // handler is owned by Options
    RequestHandler* handler = RequestHandler::CreateByName(type.c_str());
    if ( ! handler) {
        return false;
    }
    RequestHandler::Status status = handler->Init(path, *handler_config->child_block_);
    if (status != RequestHandler::OK) {
        delete handler;
        return false;
    }

    handlerMap[path] = handler;
    return true;
}

bool Options::addPort(std::shared_ptr<NginxConfigStatement> port_config) {
    if(port_config->tokens_.size() != STATEMENT_SIZE) {
        std::cerr << "Incorrect number of tokens. (For port)\n";
        return false;
    }

    string port = port_config->tokens_[VAL];
    if((unsigned int) std::stoi(port) > MAX_PORT || (unsigned int) std::stoi(port) < MIN_PORT) {
        std::cerr << "Invalid port number.\n";
        return false;
    }
    this->port = (unsigned short) std::stoi(port);
    return true;
}

bool Options::loadOptionsFromStream(std::istream* config_file) {

    NginxConfigParser parser;
    NginxConfig config;
    if(parser.Parse(config_file, &config) == false) {
        return false;
    }

    bool issetPort = false;
    for (unsigned int i =0; i < config.statements_.size(); i++) {
        std::shared_ptr<NginxConfigStatement> temp_config = config.statements_[i];

        // Sets port.
        if(temp_config->tokens_[KEY] == "port") {
            if(issetPort) {
                std::cerr << "Multiple ports in config file.\n";
                return false;
            } else {
                issetPort = addPort(temp_config);
                if(!issetPort) {
                    return false;
                }
            }
        } 

        // Sets handler.
        else if (temp_config->tokens_[KEY] == "path") { 
            if(addHandler(temp_config) == false) {
                std::cerr << "Failed to add handler.\n";
            }
        } 

        // For future statements in config file.
        else {
            std::cerr << "Unknown specifier " << temp_config->tokens_[KEY] << " .\n";
            return false;
        }
    }
        
    return true;
}

bool Options::loadOptionsFromFile(const char* filename) {
    //TODO: Does parse return error for bad filename
    std::ifstream config_file;
    config_file.open(filename);
    if (!config_file.good()) {
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
