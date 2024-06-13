add_executable(file file.cc)
target_link_libraries(file muduo_net)
target_link_libraries(file /usr/lib/liburing.so)
