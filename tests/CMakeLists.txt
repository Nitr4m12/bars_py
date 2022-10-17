add_executable(libbars_test src/main.cpp)
target_compile_features(libbars_test PRIVATE cxx_auto_type)
target_link_libraries(libbars_test bars)

add_test(
    NAME libbars_test
    COMMAND libbars_test
)
