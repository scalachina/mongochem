# find Qt
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5WebKitWidgets REQUIRED)

# find molequeue
find_package(MoleQueue REQUIRED)
include_directories(${MoleQueue_INCLUDE_DIRS})

set(tests
  get-chemical-json
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

foreach(test ${tests})
  add_executable(mongochem-rpc-${test} ${test}.cpp)
  set_target_properties(mongochem-rpc-${test} PROPERTIES AUTOMOC TRUE)
  qt5_use_modules(mongochem-rpc-${test} Widgets Network WebKitWidgets)
  add_test(NAME mongochem-rpc-${test} COMMAND mongochem-rpc-${test} $<TARGET_FILE:mongochem>)
  target_link_libraries(mongochem-rpc-${test} MoleQueueClient)
#    COMMAND ${PYTHON_EXECUTABLE}
#      ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py
#      $<TARGET_FILE:mongochem>)
#  set_property(TEST mongochem-rpc-${test} PROPERTY RUN_SERIAL 1)
endforeach()
