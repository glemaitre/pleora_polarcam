cmake_minimum_required(VERSION 2.4)
project( pix2image )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
find_package( OpenCV REQUIRED)
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)
add_executable( pix2image pix2image.cpp)
target_link_libraries( pix2image ${OpenCV_LIBS} )
