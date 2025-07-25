cmake_minimum_required(VERSION 3.17 FATAL_ERROR)
project(opnm CXX)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(INC_PATH  ${PROJECT_SOURCE_DIR}/include)
set(SRC_PATH  ${PROJECT_SOURCE_DIR}/src)

# headers used only by implementation i will change this later
set(INC_PRIVATE_PATH  ${PROJECT_SOURCE_DIR}/src)

# equivalent of: strip --strip-all
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
add_link_options($<$<CONFIG:RELEASE>:-s>)


add_library(${PROJECT_NAME} STATIC
        ${SRC_PATH}/pnm/ppm/PPM.cpp
        ${SRC_PATH}/pnm/pgm/PGM.cpp
        ${SRC_PATH}/pnm/pbm/PBM.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${INC_PATH})
target_include_directories(${PROJECT_NAME} PRIVATE ${INC_PRIVATE_PATH}) # TODO move under ${INC_PATH}/private or similar

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -pedantic
        -Wall -Wextra -Wno-unused-function -pipe
        -fstack-protector-all -fstack-protector-strong
        -O3 -ffast-math -funroll-loops -march=native -mtune=native
        -funswitch-loops -ftree-vectorize -fivopts -fmodulo-sched
)

