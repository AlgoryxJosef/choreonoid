function(CHOREONOID_PYTHON_ADD_MODULE)
  
  set(target ${ARGV0})
  string(REGEX REPLACE "^Py(.+)$" "\\1" module ${target})
  set(sources ${ARGV})
  list(REMOVE_AT sources 0)

  add_library(${target} SHARED ${sources})
    
  if(MSVC)
    set_target_properties(${target} PROPERTIES SUFFIX .pyd)
  else()
    if(ENABLE_GCC_FVISIBILITY_HIDDEN)
      target_compile_options(${target} PRIVATE "-fvisibility=hidden")
    endif()
    if(NOT ${CMAKE_BUILD_TYPE} MATCHES Debug)
      if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(${target} PRIVATE "-flto;-fno-fat-lto-objects")
	set(link_lto_flags "-flto")
      elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	target_compile_options(${target} PRIVATE "-flto=thin")
	set(link_lto_flags "-flto=thin")
      endif()
      get_target_property(existing_link_flags ${target} LINK_FLAGS)
      if(existing_link_flags STREQUAL existing_link_flags-NOTFOUND)
	set(existing_link_flags "")
      endif()
      set_target_properties(${target} PROPERTIES LINK_FLAGS "${existing_link_flags} ${link_lto_flags}")
      #add_custom_command(TARGET ${target} POST_BUILD COMMAND strip $<TARGET_FILE:${target}>)
    endif()
  endif()

  set_target_properties(${target}  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CHOREONOID_PYTHON_SUBDIR}/cnoid
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CHOREONOID_PYTHON_SUBDIR}/cnoid
    RUNTIME_OUTPUT_NAME ${module}
    LIBRARY_OUTPUT_NAME ${module}
    PREFIX "")

  if(ENABLE_INSTALL_RPATH)
    set_target_properties(${target} PROPERTIES INSTALL_RPATH "$ORIGIN:$ORIGIN/../..:$ORIGIN/../../..")
  endif()

  install(TARGETS ${target}
    RUNTIME DESTINATION ${CHOREONOID_PYTHON_SUBDIR}/cnoid CONFIGURATIONS Release Debug RelWithDebInfo MinSizeRel
    LIBRARY DESTINATION ${CHOREONOID_PYTHON_SUBDIR}/cnoid CONFIGURATIONS Release Debug RelWithDebInfo MinSizeRel)
endfunction()

# Deprecated.
function(add_cnoid_python_module)
  CHOREONOID_PYTHON_ADD_MODULE(${ARGV})
endfunction()
