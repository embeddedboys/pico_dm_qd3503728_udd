add_library(tjpg_decoder)

target_sources(tjpg_decoder PRIVATE
#   TJpg_Decoder.cpp
    tjpgd.c
)

target_include_directories(tjpg_decoder PUBLIC ${CMAKE_CURRENT_LIST_DIR})
