#include "vm.h"

int main() {
    
}

// ; allocate 3 bytes
// SET R0 3
// REQ R0 R1
//
// ; Set bytes to "Hi\n\0"
// SET R2 'H'
// STR R2 R1
// INC R1
// SET R2 'i'
// STR R2 R1
// INC R1
// SET R2 '\n'
// STR R2 R1
// INC R1
// SET R2 0
// STR R2 R1
// DEC R1
// DEC R1
// DEC R1
// 
// ; print the string
// PRS R1