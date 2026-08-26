
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

int main(int argc, char** argv) {
    if(argc != 3) {
        std::cerr << "Error: incorrect usage of the Prox CLI." << std::endl;
        std::cout << "Correct usage: " << std::endl
                  << "\t" << argv[0] << " vm FILE.prx" << std::endl
                  << "\t" << argv[0] << " asm FILE.prasm" << std::endl;
        return 1;
    }

    return 0;
}
