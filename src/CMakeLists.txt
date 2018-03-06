# ray_lib library
include_directories(${CMAKE_SOURCE_DIR}/include)
set(ray_srcs
  ${CMAKE_SOURCE_DIR}/include/ray/ray.h
  implicit.cpp ${CMAKE_SOURCE_DIR}/include/ray/implicit.h
  ray_scene.cpp ${CMAKE_SOURCE_DIR}/include/ray/ray_scene.h
)
set(libs_list glm ${OpenCV_LIBRARIES})
add_library(ray_lib ${ray_srcs})
target_link_libraries(ray_lib ${libs_list})

add_subdirectory(apps)
