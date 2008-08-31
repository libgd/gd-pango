# $id$

SET(GD_PANGO_EXAMPLES
    rotated
    simple
)

include_directories(BEFORE "${GD_PANGO_SOURCE_DIR}")
foreach(example ${GD_PANGO_EXAMPLES})
  add_executable(${example} "${example}.c")
  target_link_libraries(${example} gd_pango)
endforeach(example)
