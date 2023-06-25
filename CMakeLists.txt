project(cntr)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED YES)

add_executable(${CMAKE_PROJECT_NAME} main.c cntr.c cntr.h)
