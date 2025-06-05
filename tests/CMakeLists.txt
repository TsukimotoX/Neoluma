add_executable(neoluma_tests
    example.nlm
)

target_include_directories(neoluma_tests PRIVATE ../include)
target_link_libraries(neoluma_tests PRIVATE NeolumaLib)

add_test(NAME NeolumaTests COMMAND neoluma_tests)
