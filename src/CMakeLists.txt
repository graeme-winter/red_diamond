set(LZ4_LIB_SRC
  lz4.c
  bitshuffle.c
  bitshuffle_core.c
  iochain.c
)

add_library(libz4 STATIC ${LZ4_LIB_SRC})
set_target_properties(libz4 PROPERTIES OUTPUT_NAME z4)
target_include_directories(libz4 PUBLIC ${PROJECT_SOURCE_DIR}/include)

if (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    target_compile_options(libz4 PUBLIC -Wall -Wextra -pedantic)
    target_compile_options(libz4 PUBLIC -xHost)
    target_compile_definitions(libz4 PRIVATE -DAVX2 -DSSE2)
endif (CMAKE_CXX_COMPILER_ID MATCHES "Intel")

set(RED_DIAMOND_LIB_SRC 
  red_diamond_vds.c
)

add_library(libreddiamond STATIC ${RED_DIAMOND_LIB_SRC})
set_target_properties(libreddiamond PROPERTIES OUTPUT_NAME red_diamond)

if (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    target_compile_options(libreddiamond PUBLIC -Wall -Wextra -pedantic)
    target_compile_options(libreddiamond PUBLIC -xHost)
    target_compile_definitions(libreddiamond PRIVATE -DAVX2 -DSSE2)
endif (CMAKE_CXX_COMPILER_ID MATCHES "Intel")

target_include_directories(libreddiamond PUBLIC ${PROJECT_SOURCE_DIR}/include
                                ${HDF5_C_INCLUDE_DIRS} ${HDF5_CXX_INCLUDE_DIRS})

target_link_libraries(libreddiamond PUBLIC Threads::Threads)
