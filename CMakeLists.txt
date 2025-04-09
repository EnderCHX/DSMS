cmake_minimum_required(VERSION 3.29)
project(Distributed_Simulation_Management_System)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
include("$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

find_package(nlohmann_json CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_FILES)

#头文件
include_directories(${PROJECT_SOURCE_DIR}/include $ENV{VCPKG_ROOT}/installed/x64-windows/include)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} ws2_32 mswsock)
