/*
 * Testing module borrowed from https://github.com/metthal/IFJ-Projekt
 */

#include "test.h"
#include <unistd.h>

TestEP testSuites[256];
uint8_t testSuiteCount = 0;
uint32_t testCountOk = 0, testCountFailed = 0;
TestFlags testFlags = None;

// Here add test suite so test.c knows what test suites it can expect
TEST_SUITE(StringTest)

TEST_SUITE(TokenTest)

TEST_SUITE(SymbolHashTableTest)

TEST_SUITE(InstructionListTest)

TEST_SUITE(ConversionsTest)

TEST_SUITE(ScannerTest)

TEST_SUITE(InterpretTEST)

int main(int argc, char **argv)
{
    int32_t opt;
    while ((opt = getopt(argc, argv, "hfvb")) != -1)
    {
        switch (opt) {
            case 'h':
                printf("Usage:\n");
                printf("    ini-test [-h] | [[-f] [-v]]\n\n");
                printf("    -h                     Print help\n");
                printf("    -f                     Don't show passed tests\n");
                printf("    -ff                    Show only failed tests\n");
                printf("    -v                     Verbose output\n");
                printf("    -b                     SIGINT on failed test.\n");
                return 0;
            case 'f':
                if (testFlags & NotPassed) {
                    testFlags |= OnlyFailed;
                } else {
                    testFlags |= NotPassed;
                }
                break;
            case 'v':
                testFlags |= VerboseOut;
                break;
            case 'b':
                testFlags |= BreakOnFail;
                break;
            default:
                return 1;
        }
    }

    if (!(testFlags & VerboseOut))
        freopen("/dev/null", "w", stderr); // redirects stderr into /dev/null

    // Register new test suite here if you want to run the test suite
    REGISTER_TEST_SUITE(StringTest)
    REGISTER_TEST_SUITE(TokenTest)
    REGISTER_TEST_SUITE(SymbolHashTableTest)
    REGISTER_TEST_SUITE(InstructionListTest)
    REGISTER_TEST_SUITE(ConversionsTest)
    //REGISTER_TEST_SUITE(ScannerTest)
    REGISTER_TEST_SUITE(InterpretTEST)
    RUN_TEST_SUITES

    if (!(testFlags & OnlyFailed)) {
        printfc(1, 33, "\nResult:\n");
        printf("Total tests passed: ");
        printfc(1, 32, "%38u\n", testCountOk);

        printf("Total tests failed: ");
        printfc(1, 31, "%38u\n", testCountFailed);
    }

    return (testCountFailed > 0);
}
