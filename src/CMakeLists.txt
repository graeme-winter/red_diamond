set(LZ4_LIB_SRC
  lz4.c 
)

add_library(libz4 STATIC ${LZ4_LIB_SRC})
set_target_properties(libz4 PROPERTIES OUTPUT_NAME z4)
target_include_directories(libz4 PUBLIC ${PROJECT_SOURCE_DIR}/include)

#target_include_directories(libpennant PUBLIC ${MPI_CXX_INCLUDE_DIRS})
#target_link_libraries(libpennant PUBLIC ${MPI_CXX_LIBRARIES})

set(RED_DIAMOND_LIB_SRC
  bitshuffle_core.c 
  bitshuffle.c 
  iochain.c 
  red_diamond_vds.c
)

add_library(libreddiamond STATIC ${RED_DIAMOND_LIB_SRC})
set_target_properties(libreddiamond PROPERTIES OUTPUT_NAME red_diamond)

target_include_directories(libreddiamond PUBLIC ${PROJECT_SOURCE_DIR}/include
                                ${HDF5_C_INCLUDE_DIRS} ${HDF5_CXX_INCLUDE_DIRS})

#target_include_directories(libreddiamond PUBLIC ${PROJECT_SOURCE_DIR}/include
#                            )

target_link_libraries(libreddiamond PUBLIC Threads::Threads)