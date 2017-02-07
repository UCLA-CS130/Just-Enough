#include <string>
#include <fstream>
#include <memory>
#include <vector>

#include "options.h"

using std::string;
typedef std::map<string, string> stringMap;

const int MAX_PORT = 65535;
const int MIN_PORT = 1024;

const int COM = 0;
const int ANS = 1;
const int MODEQ = 1;
const int MOD = 2;

bool Options::addModuleToVec(Module* module) {
    //What checks should I have here if I need any?
    modules.push_back(module);
    return true;
}

bool Options::addModule(std::shared_ptr<NginxConfigStatement> module_config) {
    std::shared_ptr<stringMap> params = std::make_shared<stringMap>();

    for(size_t i = 0; i < module_config->child_block_->statements_.size(); i++) {
        if(module_config->child_block_->statements_[i]->tokens_.size() != 3) {
            std::cerr << "Not a valid module statement. Need three tokens.\n";
            return false;
        }
        string firstStatement = module_config->child_block_->statements_[i]->tokens_[0];
        if(module_config->child_block_->statements_[i]->tokens_[MODEQ] != "=") {
            std::cerr << "Module statement needs \"=\" between tokens.\n";
            return false;
        }
        string secondStatement = module_config->child_block_->statements_[i]->tokens_[MOD];

        params->insert(std::pair<string,string>(firstStatement, secondStatement));
    }

    Module* mod = createModuleFromParameters(params);
    if ( ! mod) {
        return false;
    }

    if(addModuleToVec(mod))
        return true;
    return false;
}

bool Options::loadOptionsFromStream(std::istream* config_file) {

    NginxConfigParser parser;
    NginxConfig config;
    if(parser.Parse(config_file, &config) == false) {
        return false;
    }

    bool issetPort = false;
    for (unsigned int i =0; i < config.statements_.size();i++) {
        if (config.statements_[i]->tokens_[COM] == "server") {
            std::shared_ptr<NginxConfigStatement> temp_config = config.statements_[i];
            for (unsigned int j = 0; j < temp_config->child_block_->statements_.size(); j++) {
                // Sets the port
                if(temp_config->child_block_->statements_[j]->tokens_[COM] == "listen") {
                    if(issetPort) {
                        std::cerr << "Multiple ports in config file.\n";
                        return false;
                    }
                    if(temp_config->child_block_->statements_[j]->tokens_.size() != 2) {
                        std::cerr << "Incorrect number of tokens. (For port)\n";
                        return false;
                    }
                    string port = temp_config->child_block_->statements_[j]->tokens_[ANS];
                    if((unsigned int) std::stoi(port) > MAX_PORT || (unsigned int) std::stoi(port) < MIN_PORT) {
                        std::cerr << "Invalid port number.\n";
                        return false;
                    }
                    this->port = (unsigned short) std::stoi(port);
                    issetPort = true;
                }
                //sets modules
                else if (temp_config->child_block_->statements_[j]->tokens_[0] == "module"){
                    if(addModule(temp_config->child_block_->statements_[j]) == false) {
                        //maybe specify the line in config.file?
                        return false;
                    }
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
