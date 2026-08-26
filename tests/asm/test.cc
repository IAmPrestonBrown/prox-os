/**
 * ProxOS Assembler test
 *
 * Copyright 2026 Preston Brown
 */

#include <vector>
#include <string>
#include <cstdint>
#include <cstdio>

#include <prox/asm/assembler.hpp>

static int tests_run = 0;
static int tests_passed = 0;

static void print_program(const char* label, const std::vector<uint32_t>& program) {
    printf("%s", label);
    for(size_t i = 0; i < program.size(); i++)
        printf("%u%s", program[i], i + 1 < program.size() ? "," : "");
    printf("\n");
}

static void run_test(const char* name, std::string source, std::vector<uint32_t> expected) {
    std::vector<uint32_t> actual;
    std::string thrown;
    bool threw = false;

    tests_run++;
    printf("--- Running test: %s ---\n", name);

    try {
        actual = assemble_program_from_source(source);
    } catch(const std::exception& error) {
        threw = true;
        thrown = error.what();
    }

    print_program("Expected: ", expected);
    if(threw) {
        printf("Actual:   threw std::exception: %s\n", thrown.c_str());
        printf("FAIL\n");
    } else {
        print_program("Actual:   ", actual);
        if(actual == expected) {
            tests_passed++;
            printf("PASS\n");
        } else {
            printf("FAIL\n");
        }
    }

    printf("--------------------\n");
}

int main() {
    setvbuf(stdout, nullptr, _IOLBF, 0);

    run_test("empty source",
             "",
             {80, 82, 79, 88, 0});

    run_test("whitespace only",
             "   \n\t\r  \n",
             {80, 82, 79, 88, 0});

    run_test("comment to end of file",
             "; a comment with no trailing newline",
             {80, 82, 79, 88, 0});

    run_test("comment then code",
             "; leading comment\nNOP\n",
             {80, 82, 79, 88, 0, 0, 0});

    run_test("trailing comment on a line",
             "NOP ; trailing comment\n",
             {80, 82, 79, 88, 0, 0, 0});

    run_test("zero argument operator",
             "NOP\nRET\n",
             {80, 82, 79, 88, 0, 0, 0, 21, 0});

    run_test("one argument operator",
             "INC R0\nDEC R1\nPRI R2\nPRC R3\nDEL R4\n",
             {80, 82, 79, 88, 0, 2, 0, 3, 1, 6, 2, 7, 3, 9, 4});

    run_test("two argument operator",
             "SET R0 3\nLOD R1 R0\nSTR R1 R0\nREQ R0 R1\nCPY R2 R1\n",
             {80, 82, 79, 88, 0, 129, 0, 3, 132, 1, 0, 133, 1, 0, 136, 0, 1, 144, 2, 1});

    run_test("every operator once",
             "NOP\nSET R0 1\nINC R0\nDEC R0\nLOD R0 R1\nSTR R0 R1\nPRI R0\nPRC R0\n"
             "REQ R0 R1\nDEL R0\nCPY R0 R1\nLBL 1\nJMP 1\nJMZ R0 1\nCAL 1\nRET\nRTR R0\n",
             {80, 82, 79, 88, 1,
              0, 0, 129, 0, 1, 2, 0, 3, 0, 132, 0, 1, 133, 0, 1, 6, 0, 7, 0,
              136, 0, 1, 9, 0, 144, 0, 1, 17, 1, 18, 1, 147, 0, 1, 20, 1, 21, 0, 22, 0});

    run_test("label count in header",
             "LBL 1\nLBL 2\nLBL 3\nJMP 1\n",
             {80, 82, 79, 88, 3, 17, 1, 17, 2, 17, 3, 18, 1});

    run_test("colon separator after label",
             "LBL: 1\nJMP: 1\n",
             {80, 82, 79, 88, 1, 17, 1, 18, 1});

    run_test("tab and carriage return separators",
             "SET\tR0\t3\r\nINC\tR0\r\n",
             {80, 82, 79, 88, 0, 129, 0, 3, 2, 0});

    run_test("immediate without register prefix",
             "SET 0 3\n",
             {80, 82, 79, 88, 0, 129, 0, 3});

    run_test("maximum unsigned immediate",
             "SET R0 4294967295\n",
             {80, 82, 79, 88, 0, 129, 0, 4294967295u});

    run_test("print a string program",
             "SET R0 72\nPRC R0\nSET R0 105\nPRC R0\nSET R0 10\nPRC R0\n",
             {80, 82, 79, 88, 0, 129, 0, 72, 7, 0, 129, 0, 105, 7, 0, 129, 0, 10, 7, 0});

    run_test("countdown loop program",
             "SET R0 3\n"
             "LBL 1\n"
             "PRI R0\n"
             "DEC R0\n"
             "JMZ R0 2\n"
             "JMP 1\n"
             "LBL 2\n",
             {80, 82, 79, 88, 2, 129, 0, 3, 17, 1, 6, 0, 3, 0, 147, 0, 2, 18, 1, 17, 2});

    run_test("subroutine call program",
             "CAL 1\n"
             "PRI R0\n"
             "JMP 2\n"
             "LBL 1\n"
             "SET R1 42\n"
             "RTR R1\n"
             "LBL 2\n",
             {80, 82, 79, 88, 2, 20, 1, 6, 0, 18, 2, 17, 1, 129, 1, 42, 22, 1, 17, 2});

    run_test("operator at end of file without newline",
             "PRI R0\nRET",
             {80, 82, 79, 88, 0, 6, 0, 21, 0});

    run_test("operand at end of file without newline",
             "SET R0 3",
             {80, 82, 79, 88, 0, 129, 0, 3});

    run_test("unknown mnemonic is skipped",
             "XYZ\n",
             {80, 82, 79, 88, 0});

    run_test("register prefix at end of file",
             "SET R0 3\nR",
             {80, 82, 79, 88, 0, 129, 0, 3});

    printf("%i/%i tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
