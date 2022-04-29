# To Build test for ssas SOMEIP & SD

add_executable (service-example service-example.cpp)
target_link_libraries(service-example ${VSOMEIP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_executable (client-example client-example.cpp)
target_link_libraries(client-example ${VSOMEIP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

