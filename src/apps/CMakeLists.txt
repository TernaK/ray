set(libs_list ray_lib ${OpenCV_LIBRARIES})

add_executable(ray_scene ray_scene.cpp)
target_link_libraries(ray_scene ${libs_list})
