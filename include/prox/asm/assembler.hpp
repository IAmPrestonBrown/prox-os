/**
 * ProxOS Assembler (PrASM)
 *
 * Assembles PrASM into PRX bytecode (translation is pretty much1:1)
 *
 * Copyright 2026 Preston Brown
 */

#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include <cstdint>
#include <string>
#include <vector>

std::vector<uint32_t> assemble_program_from_source(std::string value);

#endif
