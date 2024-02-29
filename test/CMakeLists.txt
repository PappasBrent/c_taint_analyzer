include(CTest)

configure_file(test_unparse.in test_unparse @ONLY)

add_test(unparse_sanitize_main bash ./test_unparse sanitize_main.c)
add_test(unparse_scanf_sanitize bash ./test_unparse scanf_sanitize.c)
add_test(unparse_assign bash ./test_unparse assign.c)
add_test(unparse_skip bash ./test_unparse skip.c)
add_test(unparse_if_then_else bash ./test_unparse if_then_else.c)
add_test(unparse_while bash ./test_unparse while.c)
add_test(unparse_decls bash ./test_unparse decls.c)
add_test(unparse_bin_expr bash ./test_unparse bin_expr.c)
add_test(unparse_returns bash ./test_unparse returns.c)