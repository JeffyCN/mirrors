#!/bin/bash -e

handle_size() {
	[ $# -eq 2 -o $# -eq 1 ] || exit 1

	# Should be int
	for arg in $@; do
		echo $arg |grep -Eq "^[0-9]+$" || exit 1
	done

	echo -n \`\`\"[$1${2:+, $2}]\"\'\'
}

handle_range() {
	[ $# -eq 2 ] || exit 1

	# Should be number
	for arg in $@; do
		echo $arg |grep -Eq "^-?[0-9]+(\.[0-9]+)?$" || exit 1
	done

	# Max should >= min
	echo "$2 >= $1" |bc |grep -wq 1 || exit 1

	echo -n \`\`\"[$1, $2]\"\'\'
}

handle_struct() {
	[ $# -eq 3 -o $# -eq 2 ] || exit 1

	echo -n @desc: alias=\"$1\", type=\"struct_list\", size=\"$2\", ui_module=\"${3:-normal_ui_style}\"
}

handle_number() {
	[ $# -eq 5 ] || exit 1

	case $2 in
		bool)
			case $5 in
				0|1|true|false) ;;
				*) exit 1
			esac
			;;
		u8|u16|u32|u64)
			echo $5 |grep -Eq "^[0-9]+$" || exit 1
			;;
		s8|s16|s32|s64)
			echo $5 |grep -Eq "^-?[0-9]+$" || exit 1
			;;
		f32|f64)
			echo $5 |grep -Eq "^-?[0-9]+(\.[0-9]+)?$" || exit 1
			;;
		*)
			exit 1
	esac

	echo -n @desc: alias=\"$1\", type=\"$2\", size=\"$3\", range=\"$4\", default=\"$5\"
}

handle_string() {
	[ $# -eq 4 ] || exit 1

	echo -n @desc: alias=\"$1\", type=\"string\", size=\"$2\", range=\"$3\", default=\"$4\"
}

handle_enum() {
	[ $# -eq 3 ] || exit 1

	echo -n @desc: type=\"enum\", alias=\"$1\", enum_def=\"$2\", default=\"$3\"
}

MACRO=$1
shift
case $MACRO in
	M4_SIZE)
		handle_size "$@"
		;;
	M4_RANGE)
		handle_range "$@"
		;;
	M4_STRUCT_DESC)
		handle_struct "$@"
		;;
	M4_NUMBER_DESC)
		handle_number "$@"
		;;
	M4_STRING_DESC)
		handle_string "$@"
		;;
	M4_ENUM_DESC)
		handle_enum "$@"
		;;
	*)
		exit 1
		;;
esac
