include(CTest)

configure_file(test_unparse.in test_unparse @ONLY)

add_test(unparse_sanitize_main bash ./test_unparse sanitize_main.c)