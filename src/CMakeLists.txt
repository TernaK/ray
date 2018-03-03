# ray_lib library
include_directories(${CMAKE_SOURCE_DIR}/include)
set(ray_srcs
  types.cpp ${CMAKE_SOURCE_DIR}/include/ray/types.h
)
set(libs_list glm ${OpenCV_LIBRARIES})
add_library(ray_lib ${ray_srcs})
target_link_libraries(ray_lib ${libs_list})

add_subdirectory(apps)
