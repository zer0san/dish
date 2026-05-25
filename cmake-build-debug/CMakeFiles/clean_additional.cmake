# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\dish_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\dish_autogen.dir\\ParseCache.txt"
  "dish_autogen"
  )
endif()
