divert(-1)

define(`M4_FATAL', `divert(0)errprint(`FATAL: $1
')m4exit(1)')

define(`M4_MACRO', `esyscmd(desc.sh $@) 'dnl
`ifelse(sysval, 0, , `M4_FATAL(`Invalid args for $1:' shift($*))')')

define(`M4_XARGS', `$1 ifelse(eval($# < 2), 1, , `M4_XARGS(shift($@))')')

define(`M4_RANGE', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_SIZE', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_STRUCT_DESC', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_NUMBER_DESC', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_STRING_DESC', `M4_MACRO(M4_XARGS(``$0'' $@))')
define(`M4_ENUM_DESC', `M4_MACRO(M4_XARGS(``$0'' $@))')

divert(0)dnl
