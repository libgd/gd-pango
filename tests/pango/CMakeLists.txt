# $id$

set(TESTS_FILES
    units
)

foreach(test_name ${TESTS_FILES})
  add_executable(${test_name} "${test_name}.c")
  target_link_libraries(${test_name} gd_pango)
  add_test(${test_name} "${EXECUTABLE_OUTPUT_PATH}/${test_name}")
endforeach(test_name)
