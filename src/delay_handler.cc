#include "delay_handler.h"
#include <iostream>
#include <unistd.h>

using std::string;


RequestHandler::Status DelayHandler::Init(const std::string&, const NginxConfig& config) {
  delay_ = DEFAULT_DELAY;
  for(size_t i = 0; i < config.statements_.size(); i++) {
    if (config.statements_[i]->tokens_.size() == 2){
        if(config.statements_[i]->tokens_[0] == "delay"){
          delay_ = std::stoi(config.statements_[i]->tokens_[1]);
        }
        else{
          std::cerr << "Invalid Delay Handler 'dealy' parameter." << std::endl;
          return RequestHandler::Error;
        }
      }
    }
    return RequestHandler::OK;
}

RequestHandler::Status DelayHandler::HandleRequest(const Request&, Response* resp) {
    resp->SetStatus(Response::code_200_OK);
    resp->SetBody("Delay Test");
    resp->AddHeader("Content-Type", "text/plain");
    usleep(delay_);
    return RequestHandler::OK;
}
