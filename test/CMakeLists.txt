if(LLVM_EXTERNAL_LIT)
  get_filename_component(LLVM_LIT_TOOLS_DIR "${LLVM_EXTERNAL_LIT}" DIRECTORY)
  find_program(FILECHECK_PATH FileCheck HINTS "${LLVM_LIT_TOOLS_DIR}")
else()
  find_program(FILECHECK_PATH FileCheck)
endif()

if(FILECHECK_PATH STREQUAL "FILECHECK_PATH-NOTFOUND")
  message(FATAL_ERROR "Could not find FileCheck")
endif()

configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
  MAIN_CONFIG
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.py
)

set(C_TAINT_TEST_DEPENDS c_taint)

add_lit_testsuite(check-c-taint "Running C_Taint regression tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${C_TAINT_TEST_DEPENDS}
)

set_target_properties(check-c-taint PROPERTIES FOLDER "Tests")

add_lit_testsuites(C_Taint ${CMAKE_CURRENT_SOURCE_DIR} DEPENDS ${C_TAINT_TEST_DEPENDS})
