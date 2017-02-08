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
const int EQUALS = 1;
const int VAL = 2;

const int STATEMENT_SIZE = 3;

bool Options::addModule(std::shared_ptr<NginxConfigStatement> module_config) {
    std::shared_ptr<stringMap> params = std::make_shared<stringMap>();

    for(size_t i = 0; i < module_config->child_block_->statements_.size(); i++) {

        // For now, not allowed to go into another module/server within a module
        if(module_config->child_block_->statements_[i]->tokens_.size() != STATEMENT_SIZE) {
            std::cerr << "Not a valid module statement. Need three tokens.\n";
            return false;
        }
        string firstStatement = module_config->child_block_->statements_[i]->tokens_[KEY];
        if(module_config->child_block_->statements_[i]->tokens_[EQUALS] != "=") {
            std::cerr << "Module statement needs \"=\" between tokens.\n";
            return false;
        }
        string secondStatement = module_config->child_block_->statements_[i]->tokens_[VAL];

        if (secondStatement.size() > 1
                && secondStatement[0] == '"' && secondStatement[secondStatement.size()-1] == '"') {
            // remove surrounding quotes
            secondStatement = secondStatement.substr(1, secondStatement.size()-2);
        }

        params->insert(std::pair<string,string>(firstStatement, secondStatement));
    }

    Module* mod = createModuleFromParameters(params);
    if (!mod) {
        return false;
    }

    modules.push_back(mod);
    return true;
}

bool Options::addPort(std::shared_ptr<NginxConfigStatement> port_config) {
    if(port_config->tokens_.size() != STATEMENT_SIZE) {
        std::cerr << "Incorrect number of tokens. (For port)\n";
        return false;
    }
    if(port_config->tokens_[EQUALS] != "=") {
        std::cerr << "Port statement needs \"=\" between tokens.\n";
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
        if (config.statements_[i]->tokens_[KEY] == "server") {
            std::shared_ptr<NginxConfigStatement> temp_config = config.statements_[i];
            for (unsigned int j = 0; j < temp_config->child_block_->statements_.size(); j++) {

                // Sets port.
                if(temp_config->child_block_->statements_[j]->tokens_[KEY] == "port") {
                    if(issetPort) {
                        std::cerr << "Multiple ports in config file.\n";
                        return false;
                    }
                    else {
                        issetPort = addPort(temp_config->child_block_->statements_[j]);
                        if(!issetPort) {
                            return false;
                        }
                    }
                }

                // Sets modules.
                else if (temp_config->child_block_->statements_[j]->tokens_[KEY] == "module"){
                    if(addModule(temp_config->child_block_->statements_[j]) == false) {
                        return false;
                    }
                }

                // For future child blocks in config file.
                else{
                    return false;
                }
            }
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
    std::vector<Module*>::iterator it;
    for (it = modules.begin(); it != modules.end(); it++) {
        delete *it;
    }
}
