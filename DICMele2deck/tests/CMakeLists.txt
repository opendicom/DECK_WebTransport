# Regression tests.
#
#   cmake -S . -B /tmp/deck-build -G Ninja
#   cmake --build /tmp/deck-build --target check
#
# "check" builds only what the tests need (some parser targets do not build yet,
# so a plain "cmake --build" stops on them) and then runs every test with ctest.
# The include folders (inc, thirdparty/blake3) come from include_directories() in the
# parent CMakeLists.txt.

set(TEST_WARNINGS -Wall -Wextra -Wshadow -Wno-unknown-pragmas)
set(TEST_SANITIZERS -fsanitize=address,undefined)

# ---------------------------------------------------------------------------
# unit test: byte swaps of datatypes.c
# Only datatypes.c is compiled in: dicmlib would also bring main.c and its main().
# ---------------------------------------------------------------------------
add_executable(test_datatypes
        unit/test_datatypes.c
        ${PROJECT_SOURCE_DIR}/src/datatypes.c
)
target_compile_options(test_datatypes PRIVATE ${TEST_WARNINGS} ${TEST_SANITIZERS})
target_link_options(test_datatypes PRIVATE ${TEST_SANITIZERS})

add_test(NAME datatypes_swaps COMMAND test_datatypes)

# ---------------------------------------------------------------------------
# end-to-end test: ESIPFfiled exit status, group hashes and frame count
# ---------------------------------------------------------------------------
add_executable(run_esipffiled
        e2e/run_esipffiled.c
)
target_compile_options(run_esipffiled PRIVATE ${TEST_WARNINGS} ${TEST_SANITIZERS})
target_link_options(run_esipffiled PRIVATE ${TEST_SANITIZERS})
target_link_libraries(run_esipffiled libblake3)

# One line per sample: test name, file (relative to the project folder), expected exit status,
# expected frames. Expected frames come from pydicom (NumberOfFrames, 1 if absent,
# 0 without Pixel Data), not from the parser, so a parser bug cannot hide itself.
function(esipffiled_case case_name dicm_file expected_exit_status expected_frame_count)
    add_test(NAME esipffiled_${case_name}
             COMMAND run_esipffiled $<TARGET_FILE:ESIPFfiled>
                     ${PROJECT_SOURCE_DIR}/${dicm_file}
                     ${expected_exit_status} ${expected_frame_count})
endfunction()

# public samples of the repository
esipffiled_case(NM1      Testing/NM1.dcm      0 1)
esipffiled_case(US1      Testing/US1.dcm      0 1)
esipffiled_case(VL1      Testing/VL1.dcm      0 1)
esipffiled_case(VL2      Testing/VL2.dcm      0 1)
esipffiled_case(VL3      Testing/VL3.dcm      0 1)
esipffiled_case(VL6      Testing/VL6.dcm      0 1)
esipffiled_case(XA       Testing/XA.dcm       0 1)
esipffiled_case(arabic   Testing/arabic.dcm   0 1)
esipffiled_case(dscd     Testing/dscd.dcm     0 0)   # encapsulated CDA document: no Pixel Data
esipffiled_case(french   Testing/french.dcm   0 1)
esipffiled_case(greek    Testing/greek.dcm    0 1)
esipffiled_case(hebrew   Testing/hebrew.dcm   0 1)
esipffiled_case(russian  Testing/russian.dcm  0 1)

# edge cases built from Testing/US1.dcm
esipffiled_case(planar_configuration_1  tests/data/us1_planar1.dcm     0 1)   # RGB stored as R…, G…, B… planes
esipffiled_case(study_uid_64_characters tests/data/us1_uid64.dcm       0 1)   # longest UID allowed
esipffiled_case(procedure_code_sequence tests/data/us1_procedure_code_sequence.dcm 0 1)   # sequence with defined lengths
esipffiled_case(two_item_sequence       tests/data/us1_two_items.dcm     0 1)   # items ending with an empty attribute
esipffiled_case(item_longer_than_sequence tests/data/us1_item_too_long.dcm 249 0) # item length beyond its sequence: exitErrorITtruncated (-7)

# ---------------------------------------------------------------------------
# check: build what the tests need, then run them all
# ---------------------------------------------------------------------------
add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS test_datatypes run_esipffiled ESIPFfiled
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL
)
