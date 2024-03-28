divert(-1)

define(`M4_FATAL', `divert(0)errprint(`FATAL: $1
')m4exit(1)')

define(`M4_MACRO', `esyscmd(./desc.sh $@) 'dnl
`ifelse(sysval, 0, , `M4_FATAL(`Invalid args for $1:' shift($*))')')

define(`M4_XARGS', `$1 ifelse(eval($# < 2), 1, , `M4_XARGS(shift($@))')')

define(`M4_INDEX_DEFAULT', ``none'')
define(`M4_SIZE_DYNAMIC', ``dynamic'')

define(`M4_RANGE', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_SIZE', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_DIGIT', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_DYNAMIC', ``$@'')
define(`M4_HIDE', ``$@'')
define(`M4_STRUCT_DESC', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_NUMBER_DESC', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_STRING_DESC', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_ENUM_DESC', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_ARRAY_DESC', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_ARRAY_MARK_DESC', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_STRING_MARK_DESC', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_NUMBER_MARK_DESC', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_BOOL_DESC', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_STRUCT_LIST_DESC', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_ARRAY_TABLE_DESC', `M4_MACRO(M4_XARGS(``$0'' $@))')

define(`M4_RANGE_EX', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_SIZE_EX', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_DIGIT_EX', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_FP_EX', `fp=M4_XARGS(\"$@\")')
define(`M4_DYNAMIC_EX', `dynamic=M4_XARGS(\"$@\")')
define(`M4_HIDE_EX', `hide=M4_XARGS(\"$@\")')
define(`M4_INDEX_DEFAULT_EX', `index=M4_XARGS(\"none\")')
define(`M4_ORDER', `order=M4_XARGS(\"$@\")')
define(`M4_RO', `ro=M4_XARGS(\"$@\")')
define(`M4_TYPE', `type=M4_XARGS(\"$@\")')
define(`M4_ALIAS', `alias=M4_XARGS(\"$@\")')
define(`M4_DEFAULT', `default=M4_XARGS(\"$@\")')
define(`M4_ENUM_DEF', `enum_def=M4_XARGS(\"$@\")')
define(`M4_UI_MODULE', `ui_module=M4_XARGS(\"$@\")')
define(`M4_UI_PARAM', `ui_module_param=M4_XARGS(\"$@\")')
define(`M4_NOTES', `notes=M4_XARGS(\""$@"\")')
define(`M4_REF', `ref=M4_XARGS(\""$@"\")')
define(`M4_DATAX', `data_x=M4_XARGS(\""$@"\")')
define(`M4_GENERIC_DESC', `M4_MACRO(M4_XARGS(``$0'' $@))')

divert(0)dnl
