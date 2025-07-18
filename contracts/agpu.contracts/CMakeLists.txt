add_contract(agpucontracts agpu.contracts ${CMAKE_CURRENT_SOURCE_DIR}/src/agpu.contracts.cpp)

target_include_directories(agpu.contracts
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include )

set_target_properties(agpu.contracts
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

target_compile_options( agpu.contracts PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
