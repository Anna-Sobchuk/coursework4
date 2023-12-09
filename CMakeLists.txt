cmake_minimum_required(VERSION 3.21)
project(coursework4)

set(CMAKE_CXX_STANDARD 17)
set(ASIO_INCLUDE_DIR "C://Users//Anna//Downloads//asio-1.28.0//asio-1.28.0//include")
set(CEREAL_INCLUDE_DIR "C://Users//Anna//Downloads//cereal-master//cereal-master//include//cereal")

include_directories(
        .
        .idea
        Client
        Server
        ${ASIO_INCLUDE_DIR}
        ${CEREAL_INCLUDE_DIR}
)

set(
        coursework4
        .idea/.gitignore
        .idea/coursework4.iml
        .idea/modules.xml
        .idea/vcs.xml
        .idea/workspace.xml
        Client/ClientProgram.cpp
        Server/ServerProgram.cpp
        Index/IndexProgram.cpp
        README.md
        Schema.jpg
)

add_executable(Client Client/ClientProgram.cpp)
target_link_libraries(Client PRIVATE ws2_32)

add_executable(Server
        Server/ServerProgram.cpp
        Index/IndexProgram.cpp
        )
target_link_libraries(Server PRIVATE ws2_32)

add_executable(TimeAnalysis
        Index/IndexProgram.cpp
        TimeAnalysis.cpp
        )

set(CMAKE_CXX_STANDARD_LIBRARIES "-lstdc++fs ${CMAKE_CXX_STANDARD_LIBRARIES}")
