project(lab3)

add_executable(parent_run 
    src/parent.c
    src/utils.c
)

add_executable(child_run 
    src/child.c
    src/utils.c
)