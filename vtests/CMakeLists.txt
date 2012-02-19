cmake_minimum_required(VERSION 2.8)

set (TESTS basic_call general t1 opt branch pkg_test multi_test prefix_test)

INCLUDE_DIRECTORIES(${dill_BINARY_DIR} ${dill_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

ADD_CUSTOM_COMMAND(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/general.c
   COMMAND perl -w ${CMAKE_CURRENT_SOURCE_DIR}/general.ops
   MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/general.ops
)

foreach (TEST ${TESTS} )
    ADD_EXECUTABLE(v${TEST} ${TEST}.c)
    TARGET_LINK_LIBRARIES(v${TEST} dill )
    ADD_TEST(vtest_${TEST} "v${TEST}")
endforeach()

SET_TARGET_PROPERTIES(vgeneral PROPERTIES LINKER_LANGUAGE C COMPILE_FLAGS "-O0"
)


