add_executable(libbars_test ${CMAKE_SOURCE_DIR}/tests/src/main.cpp)

target_compile_features(libbars_test PUBLIC cxx_std_17)
target_link_libraries(libbars_test PUBLIC bars)
target_link_libraries(libbars_test PUBLIC oead)

add_test(
    NAME libbars_test
    COMMAND libbars_test ${CMAKE_CURRENT_SOURCE_DIR}/FieldBattleBgm.bars
)
