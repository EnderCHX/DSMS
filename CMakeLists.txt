cmake_minimum_required(VERSION 3.29)
project(Distributed_Simulation_Management_System)

set(CMAKE_CXX_STANDARD 20)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_FILES)

#头文件
include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})

find_package(boost_asio REQUIRED CONFIG)
find_package(nlohmann_json CONFIG REQUIRED)


if (${WIN32})
    target_link_libraries(${CMAKE_PROJECT_NAME}
            ws2_32
            mswsock
            Boost::asio
            nlohmann_json::nlohmann_json
    )
else()
    target_link_libraries(${CMAKE_PROJECT_NAME}
            Boost::asio
            nlohmann_json::nlohmann_json
    )
endif()