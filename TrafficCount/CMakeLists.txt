project (main)
cmake_minimum_required(VERSION 3.15)

# configure OpenCV
find_package(OpenCV REQUIRED)

# create create individual projects
add_executable(main main.cpp)
target_link_libraries(main ${OpenCV_LIBS})

add_executable(traffic traffic.cpp)
target_link_libraries(traffic ${OpenCV_LIBS})
