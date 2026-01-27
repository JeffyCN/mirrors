#!/bin/bash

choices=(rv1126b rk3588)

function usage() {
	echo "Usage: source envsetup.sh"
}

function print_soc_menu() {
  echo
  echo "Select a soc to build rkfec for: "
  echo

  local i=1
  local choice
  for choice in ${choices[@]}
  do
    echo "     $i. $choice"
    i=$(($i+1))
  done
}

function set_fec_hw() {
	unset RKFEC_HW_VERSION
	export RKFEC_HW_VERSION=-DRKFEC_HW_V${1}
	env | grep -q RKFEC_HW_VERSION && echo "Success !" || echo "Failed !"
}

function lunch() {
  local answer

  if [ "$1" ]; then
    answer=$1
  else
    print_soc_menu
    echo -n "Which would you like? [rv1126b] "
    read answer
  fi

  local selection
  if [ -z "$answer" ]
  then
    selection="rv1126b"
	elif (echo -n $answer | grep -q -e "^[0-9][0-9]*$")
	then
		if [ $answer -le ${#choices[@]} ]
		then
			# array in zsh starts from 1 instead of 0.
			if [ -n "$ZSH_VERSION" ]
			then
				selection=${choices[$(($answer))]}
			else
				selection=${choices[$(($answer-1))]}
			fi
		fi
  else
    selection=$answer
  fi

  case $selection in
    rv1126b)
      set_fec_hw 20
      ;;
    rk3588)
      set_fec_hw 10
      ;;
    *)
      echo "Not supported SoC yet!"
      usage
      ;;
  esac

  unset RKFEC_TARGET_SOC
  export RKFEC_TARGET_SOC=$selection
}

lunch $1
