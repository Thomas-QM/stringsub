include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB TESTS "*.cpp")

add_executable(tests ${TESTS})
target_link_libraries(tests stringsub)

add_test(TESTS tests)