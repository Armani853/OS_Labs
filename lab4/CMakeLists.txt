project(lab4)

add_library(Lab4_Headers INTERFACE) 

target_include_directories(
    Lab4_Headers INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(convert_bin SHARED lib/lib1_v1.c) 
target_link_libraries(convert_bin INTERFACE Lab4_Headers)
set_target_properties(convert_bin PROPERTIES OUTPUT_NAME "convert_bin")

add_library(convert_tri SHARED lib/lib1_v2.c) 
target_link_libraries(convert_tri INTERFACE Lab4_Headers)
set_target_properties(convert_tri PROPERTIES OUTPUT_NAME "convert_tri")

add_library(sort_bubble SHARED lib/lib2_v1.c) 
target_link_libraries(sort_bubble INTERFACE Lab4_Headers)
set_target_properties(sort_bubble PROPERTIES OUTPUT_NAME "sort_bubble")

add_library(sort_hoare SHARED lib/lib2_v2.c) 
target_link_libraries(sort_hoare INTERFACE Lab4_Headers)
set_target_properties(sort_hoare PROPERTIES OUTPUT_NAME "sort_hoare")

add_executable(lab4_prog1 prog/prog1_static.c)

target_link_libraries(lab4_prog1 PRIVATE convert_bin sort_bubble Lab4_Headers) 

set_target_properties(lab4_prog1 PROPERTIES BUILD_RPATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

add_executable(lab4_prog2 prog/prog2_dynamic.c)

target_link_libraries(lab4_prog2 PRIVATE dl Lab4_Headers) 

set_target_properties(lab4_prog2 PROPERTIES BUILD_RPATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")