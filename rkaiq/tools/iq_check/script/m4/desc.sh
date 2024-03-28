#!/bin/bash -e

handle_size() {
	# Should be int
	echo -n \`\`\"[$1${2:+, $2}]\"\'\'
}

handle_digit() {
	echo -n \`\`\"[$1${2:+, $2}]\"\'\'
}

handle_range() {
	# Max should >= min
	echo -n \`\`\"[$1, $2]\"\'\'
}

handle_hide() {
	# Max should >= min
	echo -n \`\`\"[$1, $2]\"\'\'
}

handle_generic_type() {
	echo -n \`\`${addTag}='\"'\"[$1${2:+, $2}]\"'\"'\'\'
}

handle_struct() {
	echo -n @desc: alias=\"$1\", type=\"struct\", ui_module=\"${2:-normal_ui_style}\", hide=\"${3:-0}\", ro=\"${4:-0}\"
}

handle_struct_list() {
	echo -n @desc: alias=\"$1\", type=\"struct_list\", size=\"$2\", ui_module=\"${3:-normal_ui_style}\", ro=\"${4:-0}\", hide=\"${5:-0}\"
}

handle_number() {
	echo -n @desc: alias=\"$1\", type=\"$2\", size=\"[1,1]\", range=\"$3\", default=\"$4\", digit=\"$5\", hide=\"${6:-0}\", ro=\"${7:-0}\"
}

handle_array() {
	echo -n @desc: alias=\"$1\", type=\"$2\", size=\"$3\", range=\"$4\", default=\"$5\", digit=\"$6\", dynamic=\"$7\", hide=\"${8:-0}\", ro=\"${9:-0}\"
}

handle_array_mark() {
	echo -n @desc: alias=\"$1\", type=\"$2\", size=\"$3\", range=\"$4\", default=\"$5\", digit=\"$6\", dynamic=\"$7\", ui_module=\"${8:-normal_ui_style}\", ro=\"${9:-0}\"
}

handle_string_mark() {
	echo -n @desc: alias=\"$1\", type=\"string\", size=\"$2\", range=\"$3\", default=\"$4\", dynamic=\"$5\", ui_module_param=\"${6:-normal_ui_style}\", ro=\"${7:-0}\"
}

handle_number_mark() {
	echo -n @desc: alias=\"$1\", type=\"$2\", range=\"$3\", default=\"$4\", digit=\"$5\", ui_module_param=\"${6:-normal_ui_style}\", ro=\"${7:-0}\", dynamic=\"${8:-0}\"
}

handle_array_table() {
	echo -n @desc: alias=\"$1\", type=\"struct\", ui_module=\"$2\", index=\"$3\", hide=\"${4:-0}\", ro=\"${5:-0}\"
}

handle_string() {
	echo -n @desc: alias=\"$1\", type=\"string\", size=\"$2\", range=\"$3\", default=\"$4\", dynamic=\"${5:-0}\", hide=\"${6:-0}\", ro=\"${7:-0}\"
}

handle_enum() {
	echo -n @desc: type=\"enum\", alias=\"$1\", enum_def=\"$2\", default=\"$3\", ro=\"${4:-0}\"
}

handle_bool() {
	echo -n @desc: type=\"bool\", alias=\"$1\", default=\"$2\", hide=\"${3:-0}\", ro=\"${4:-0}\"
}

handle_generic_desc() {
	echo -n "@desc: "
	while [ $# != 0 ]; do
		echo -n $1
		if [ $# != 1 ];then
			echo -n ,
		fi
		shift
	done

	#echo -n @desc: $@
}

echo "$@" >> ./m4.log
MACRO=$1
shift
case $MACRO in
	M4_SIZE)
		handle_size "$@"
		;;
	M4_DIGIT)
		handle_digit "$@"
		;;
	M4_RANGE)
		handle_range "$@"
		;;
	M4_STRUCT_DESC)
		handle_struct "$@"
		;;
	M4_STRUCT_LIST_DESC)
		handle_struct_list "$@"
		;;
	M4_NUMBER_DESC)
		handle_number "$@"
		;;
	M4_HIDE)
		handle_hide "$@"
		;;
    M4_ARRAY_DESC)
        handle_array "$@"
        ;;
    M4_ARRAY_MARK_DESC)
        handle_array_mark "$@"
        ;;
    M4_STRING_MARK_DESC)
        handle_string_mark "$@"
        ;;
    M4_NUMBER_MARK_DESC)
        handle_number_mark "$@"
        ;;
    M4_ARRAY_TABLE_DESC)
        handle_array_table "$@"
        ;;
	M4_STRING_DESC)
		handle_string "$@"
		;;
	M4_ENUM_DESC)
		handle_enum "$@"
		;;
	M4_BOOL_DESC)
		handle_bool "$@"
        ;;
	M4_RANGE_EX|M4_SIZE_EX|M4_DIGIT_EX)
		unset addTag
		if [[ $MACRO == M4_RANGE_EX ]]; then
			addTag=range
		elif [[ $MACRO == M4_SIZE_EX ]]; then
			addTag=size
		elif [[ $MACRO == M4_DIGIT_EX ]]; then
			addTag=digit
		else
			exit 0
		fi
		handle_generic_type "$@"
		unset addTag
        ;;
	M4_GENERIC_DESC)
		handle_generic_desc "$@"
        ;;
	*)
		exit 0
		;;
esac
