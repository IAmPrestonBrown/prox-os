/**
 * ProxOS Assembler (PrASM)
 *
 * Copyright 2026 Preston Brown
 */

#include <prox/asm/assembler.hpp>
#include <prox/vm/vm.h>

#include <string>
#include <map>
#include <iterator>

// A map of every operator and its integer/bytecode value
const std::map<std::string, uint32_t> operator_string_values{{"NOP", 0x00}, {"SET", 0x01 | TWO_FLAG}, {"INC", 0x02}, {"DEC", 0x03}, {"LOD", 0x04 | TWO_FLAG}, {"STR", 0x05 | TWO_FLAG}, {"PRI", 0x06}, {"PRC", 0x07}, {"REQ", 0x08 | TWO_FLAG}, {"DEL", 0x09}, {"CPY", 0x10 | TWO_FLAG}, {"LBL", 0x11}, {"JMP", 0x12}, {"JMZ", 0x13 | TWO_FLAG}, {"CAL", 0x14}, {"RET", 0x15}, {"RTR", 0x16}};

std::vector<uint32_t> assemble_program_from_source(std::string value) {
    std::vector<uint32_t> program{80, 82, 79, 88, 0};

    for(auto current = value.begin(); current != value.end(); current++) {
        // skip any whitespace or comments (comments are ASM style ; comment)
        while(current != value.end() && (*current == ' ' || *current == '\n' || *current == '\t' || *current == '\r' || *current == ';' || *current == ':')) {
            if(*current == ';') {
                while(current != value.end() && *current != '\n')
                    current++;
            } else {
                current++;
            }
        }
        if(current == value.end())
            break;

        // handle operators
        if(value.end() - current >= 3) {
            // get the string value
            std::string token_operator_value = std::string(current, current + 3);

            // map it out to the token list, if a valid operator than lex it
            auto token_operator = operator_string_values.find(token_operator_value);
            if(token_operator != operator_string_values.end()) {
                if(token_operator_value == "LBL")
                    program[4]++;
                program.push_back(token_operator->second);
                if(token_operator_value == "NOP" || token_operator_value == "RET")
                    program.push_back(0);
                current = std::ranges::next(current, 2, value.end());
                continue;
            }
        }

        if(*current == 'R') {
            current = std::ranges::next(current, 1, value.end());
        }

        // Any other token will be a number
        std::string num = std::string();
        for(; current != value.end(); current++) {
            if(*current < 48 || *current > 57) // 0-9 in ASCII is 48-57
                break;
            num.push_back(*current);
        }

        // Handle cases where no number was parsed
        // If error handling is ever added, this is where it should go
        if(num.empty()) {
            if(current == value.end())
                break;
            continue;
        }

        program.push_back(static_cast<uint32_t>(std::strtoul(num.c_str(), nullptr, 10)));
        if(current == value.end())
            break;
        current--;
    }

    return program;
}
