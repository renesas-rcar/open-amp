collector_create (APP_COMMON_SOURCES "")
collector_create (APP_LIB_DIRS "")
collector_create (APP_INC_DIRS "")
collector_create (APP_LIB_DEPS "")

collector_create (APP_EXTRA_C_FLAGS "")
set (APPS_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set (APPS_SHARE_DIR "${CMAKE_CURRENT_BINARY_DIR}/share")

collect (APP_INC_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory (machine)
add_subdirectory (system)
add_subdirectory (tests)
add_subdirectory (examples)

# To build libraries for app/machine
if (LIB_APP_MACHINE)
    collector_list(_source APP_COMMON_SOURCES)
    collector_list(_header APP_INC_DIRS)

    include_directories(${_header})

    set(_type_lib "")

    if (WITH_STATIC_LIB)
        list(APPEND _type_lib STATIC)
    endif(WITH_STATIC_LIB)

    if (WITH_SHARED_LIB)
        list(APPEND _type_lib SHARED)
    endif(WITH_SHARED_LIB)

    foreach(_type ${_type_lib})
        set(_lib open_amp_${PROJECT_MACHINE}_${_type})
        add_library(${_lib} ${_type} ${_source} ${_header})
        set_target_properties(${_lib} PROPERTIES OUTPUT_NAME "open_amp_${PROJECT_MACHINE}")
        install (TARGETS ${_lib} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endforeach(_type)

    foreach(_inc_dir ${_header})
        file(GLOB tmp_hdrs "${_inc_dir}/*.h")
        list(APPEND _inc_files ${tmp_hdrs})
    endforeach(_inc_dir)

    install(FILES ${_inc_files} DESTINATION include/${PROJECT_MACHINE})
endif(LIB_APP_MACHINE)
