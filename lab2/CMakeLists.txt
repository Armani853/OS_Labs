project(lab2)

add_executable(lab_02_run 
    src/functions.c src/main.c
)

find_package(Threads REQUIRED)

target_link_libraries(lab_02_run Threads::Threads)

target_link_libraries(lab_02_run m)