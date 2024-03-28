#!/bin/bash

choices=(rk3576 rv1106 rk3588 rk356x rv1109 rk3562)

function usage() {
	echo "Usage: source envsetup.sh"
}

function print_soc_menu() {
  echo
  echo "Select a soc to build rkaiq for: "
  echo

  local i=1
  local choice
  for choice in ${choices[@]}
  do
    echo "     $i. $choice"
    i=$(($i+1))
  done
}

function set_isp_hw() {
	unset ISP_HW_VERSION
	export ISP_HW_VERSION=-DISP_HW_V${1}
	env | grep -q ISP_HW_VERSION && echo "Success !" || echo "Failed !"
}

function lunch() {
  local answer

  if [ "$1" ]; then
    answer=$1
  else
    print_soc_menu
    echo -n "Which would you like? [rv1106] "
    read answer
  fi

  local selection
  if [ -z "$answer" ]
  then
    selection="rv1106"
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
    rk3576)
      set_isp_hw 39
      ;;
    rk3562)
      set_isp_hw 32_LITE
      ;;
    rv1106)
      set_isp_hw 32
      ;;
    rk3588)
      set_isp_hw 30
      ;;
    rk356x)
      set_isp_hw 21
      ;;
    rv1109)
      set_isp_hw 20
      ;;
    *)
      echo "Not supported SoC yet!"
      usage
      ;;
  esac

  unset RKAIQ_TARGET_SOC
  export RKAIQ_TARGET_SOC=$selection
}

lunch $1
