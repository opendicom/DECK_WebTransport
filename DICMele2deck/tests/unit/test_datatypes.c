// Unit test of the byte swap functions of src/datatypes.c.
//
// build: gcc -std=gnu99 -Wall -Wextra -Wno-unknown-pragmas -Iinc tests/unit/test_datatypes.c src/datatypes.c
// exit:  0 every check passed, 1 at least one failed

#include <stdio.h>
#include <datatypes.h>

// Compares function_name(input_value) with expected_value and prints both on failure.
// Values are printed as unsigned long long so the same macro works for u16, u32 and u64.
// No return: a failure only adds 1 to failure_count, so a loop can keep checking the next cases.
#define check(function_name,input_value,expected_value,failure_count) do { \
    unsigned long long actual_result_ = function_name(input_value); \
    unsigned long long expected_result_ = (unsigned long long) (expected_value); \
    unsigned long long input_printed_ = (unsigned long long) (input_value); \
    if (actual_result_ == expected_result_) { \
        printf("[PASSED] %s(%#llx) = %#llx\n", #function_name, input_printed_, actual_result_); \
    } else { \
        printf("[FAIL] %s(%#llx): expected %#llx, got %#llx\n", #function_name, \
               input_printed_, expected_result_, actual_result_); \
        (failure_count)++; \
    } \
} while (0)

// Declares the type of one test case: declare_cases(u16) becomes
// "struct case_u16 { u16 input, expected_output; }". ## pastes "case_" and the type name.
// Used outside functions, so the type exists for the whole file.
#define declare_cases(type) struct case_##type { type input, expected_output; }

// Runs check() on every case of an array. The size is computed here because the macro
// receives the name of the array (a function would only receive a pointer).
// Local names end with _ so they do not clash with variables of the caller.
#define run_cases(function_name,test_cases,failure_count) do { \
    int case_count_ = sizeof(test_cases) / sizeof((test_cases)[0]); \
    for (int case_index_ = 0; case_index_ < case_count_; case_index_++) \
        check(function_name, (test_cases)[case_index_].input, \
              (test_cases)[case_index_].expected_output, failure_count); \
} while (0)

declare_cases(u16);
declare_cases(u32);
declare_cases(u64);

static int u16swap_test(void) {
    struct case_u16 test_cases[] = {
        {0xAABB, 0xBBAA},
        {0x1234, 0x3412},
        {0x0000, 0x0000},
        {0xFFFF, 0xFFFF},
    };
    int failure_count = 0;
    run_cases(u16swap, test_cases, failure_count);
    return failure_count;
}

static int u32swap_test(void) {
    struct case_u32 test_cases[] = {
        {0xDDCCAABB, 0xBBAACCDD},
        {0x11223344, 0x44332211},
        {0x00000000, 0x00000000},
    };
    int failure_count = 0;
    run_cases(u32swap, test_cases, failure_count);
    return failure_count;
}

static int u64swap_test(void) {
    struct case_u64 test_cases[] = {
        {0x1100FFEEDDCCAABB, 0xBBAACCDDEEFF0011},
        {0x1122334455667788, 0x8877665544332211},
    };
    int failure_count = 0;
    run_cases(u64swap, test_cases, failure_count);
    return failure_count;
}

static int DICMtagLE2BEswap_test(void) {
    struct case_u32 test_cases[] = {
        {0x0000AABB, 0x0000BBAA},
        {0x00200010, 0x20001000},   // (0010,0020) Patient ID as read from the file
        {0x11223344, 0x22114433},   // every byte different: each 16-bit half swaps its two bytes
    };
    int failure_count = 0;
    run_cases(DICMtagLE2BEswap, test_cases, failure_count);
    return failure_count;
}

int main(void) {
    int total_failure_count = 0;
    total_failure_count += u16swap_test();
    total_failure_count += u32swap_test();
    total_failure_count += u64swap_test();
    total_failure_count += DICMtagLE2BEswap_test();

    if (total_failure_count == 0) {
        printf("[PASSED] All tests completed successfully.\n");
    } else {
        printf("[FAIL] %d check(s) failed.\n", total_failure_count);
    }
    return total_failure_count != 0;
}
