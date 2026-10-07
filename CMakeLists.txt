cmake_minimum_required(VERSION 3.16)

project(computer_graphics_lab4)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_executable(computer_graphics_lab4
  main.cpp
  polygonwidget.h
  polygonwidget.cpp
  geometry.h
  geometry.cpp
)
target_link_libraries(computer_graphics_lab4 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

include(CTest)
if(BUILD_TESTING)
  find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)
  add_executable(lab4_tests
    tests/lab4_tests.cpp
    polygonwidget.h
    polygonwidget.cpp
    geometry.h
    geometry.cpp
  )
  target_include_directories(lab4_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(lab4_tests PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Test)
  add_test(NAME lab4_tests COMMAND lab4_tests)
  set_tests_properties(lab4_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()
