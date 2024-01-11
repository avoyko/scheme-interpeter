#pragma once
#include <sstream>
#include <string>
#include <parser.h>
#include <helpers.h>

class Interpreter {
public:
    std::string Run(const std::string& input);
    std::string PerformOutput(std::shared_ptr<Object> ast);
    void Serialize(std::shared_ptr<Object> ast, std::string& ans);
};
