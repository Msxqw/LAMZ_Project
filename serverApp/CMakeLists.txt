cmake_minimum_required(VERSION 3.14)

project(serverApp LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core)
find_package(Qt5 REQUIRED COMPONENTS Core)
find_package(Qt5 REQUIRED COMPONENTS Network)

add_executable(serverApp
  main.cpp
  protocol.h
  spi_controller.h
  spi_controller.cpp
  tcp_server.h
  tcp_server.cpp
  parser.h
  parser.cpp

)
target_link_libraries(serverApp Qt5::Core Qt5::Network)

include(GNUInstallDirs)
install(TARGETS serverApp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
