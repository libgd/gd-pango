# $id$

SET(TESTS_SUBDIRS
    pango
)

if(BUILD_TESTING)
  include_directories(BEFORE "${GD_PANGO_SOURCE_DIR}")
  foreach(tests_subdir ${TESTS_SUBDIRS})
    add_subdirectory(${tests_subdir})
  endforeach(tests_subdir)
endif(BUILD_TESTING)
