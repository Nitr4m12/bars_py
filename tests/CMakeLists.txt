add_executable(libbars_test src/main.cpp)
target_compile_features(libbars_test PRIVATE cxx_auto_type)
target_link_libraries(libbars_test bars)

add_test(
    NAME libbars_test
    COMMAND libbars_test ${CMAKE_CURRENT_SOURCE_DIR}/FieldBattleBgm.bars
)

set_property(
    TEST libbars_test
    PROPERTY ENVIRONMENT CTEST_OUTPUT_ON_FAILURE=1
)
