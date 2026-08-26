
/**
 * Prox CLI
 *
 * Used to operate the ProxVM and PrASM from non-proxos systems (i.e. windows, linux, macos, etc.):
 * prox vm FILE.prx
 * -- OR --
 * prox asm FILE.prasm
 *
 * Note: Both the VM and Assembler have low-quality error reporting. This is done on intention as they are supposed to be embedded libraries.
 *       Ensure your input to this program is proper before using, otherwise you will get unexepcted errors and bugs.
 *
 * Copyright 2026 Preston Brown
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <prox/asm/assembler.hpp>
#include <prox/vm/vm.h>

std::optional<std::string> read_to_string(std::string path) {
    // read file and check existance
    std::ifstream file(path);
    if(!file) {
        return std::nullopt;
    }

    std::stringstream file_contents_buffer;
    file_contents_buffer << file.rdbuf();

    return file_contents_buffer.str();
}

std::optional<std::vector<uint32_t>> read_to_ints(std::string path) {
    // read file and check existance
    std::ifstream file(path);
    if(!file) {
        return std::nullopt;
    }

    // ensure file contains enough bytes to have an even number of 32 bit integers
    // 32 bit integer == 4 bytes (most of the time)
    file.seekg(0, std::ios::end);
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    if(file_size % sizeof(uint32_t) != 0) {
        return std::nullopt;
    }

    // try to read the bytes
    std::vector<uint32_t> program_ints(file_size / sizeof(uint32_t));
    file.read(
        reinterpret_cast<char*>(program_ints.data()),
        file_size);

    // final check if there was any failures in the read
    if(!file) {
        return std::nullopt;
    }

    return program_ints;
}

int write_program(std::string path, std::vector<uint32_t> program) {
    // try to open the file in write mod
    std::ofstream file(path, std::ios::binary);
    if(!file) {
        return -1;
    }

    // write the integer bytes
    file.write(reinterpret_cast<const char*>(program.data()), program.size() * sizeof(uint32_t));

    // final catch
    if(!file) {
        return -1;
    }

    return 0;
}

int main(int argc, const char** argv) {
    // Every proper invocation will be expected to be 3 args ([exec] vm|asm [file])
    // dead simple implementation
    if(argc != 3 || (strcmp(argv[1], "vm") != 0 && strcmp(argv[1], "asm") != 0)) {
        std::cerr << "Error: incorrect usage of the Prox CLI." << std::endl;
        std::cout << "Correct usage: " << std::endl
                  << "\t" << argv[0] << " vm FILE.prx" << std::endl
                  << "\t" << argv[0] << " asm FILE.prasm" << std::endl;
        return 1;
    }

    if(strcmp(argv[1], "vm") == 0) {
        auto program_ints = read_to_ints(std::string(argv[2]));
        if(!program_ints) {
            std::cerr << "Error: could not read file '" << argv[2] << "'" << std::endl;
            return 1;
        }

        VirtualMachine* vm = create_vm();
        if(execute_program(vm, program_ints.value().data(), program_ints.value().size()) == -1) {
            std::cerr << "Error: an error occured while executing the bytecode" << std::endl;
            return 1;
        }

    } else {
        auto program_string = read_to_string(std::string(argv[2]));
        if(!program_string) {
            std::cerr << "Error: could not read file '" << argv[2] << "'" << std::endl;
            return 1;
        }

        if(write_program("./out.prx", assemble_program_from_source(program_string.value())) == -1) {
            std::cerr << "Error: an error occured while writing the program" << std::endl;
            return 1;
        }
    }

    return 0;
}
