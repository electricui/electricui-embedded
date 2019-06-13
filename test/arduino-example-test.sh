#!/bin/bash
# Compiles each of the Arduino example sketches in electricui-embedded/examples

# Setup the various paths to the installed location, user's sketchbook and the library/board manager cache
# This is different between Linux, macOS etc, so try and detect where possible.
bin_path="arduino-builder tool directory"
user_sketchbook="sketchbook directory"
user_hidden_path="arduino15 directory"

os_name="unsupported"

unameu="$(tr '[:lower:]' '[:upper:]' <<<$(uname))"
if [[ $unameu == *DARWIN* ]]; then
	os_name="darwin"

	bin_path=/Applications/Arduino.app/Contents/Java
	user_sketchbook=~/Documents/Arduino
	user_hidden_path=~/Library/Arduino15

elif [[ $unameu == *LINUX* ]];then
	os_name="linux"

	bin_path=/opt/arduino
	user_sketchbook=~/Arduino
	user_hidden_path=~/.arduino15

elif [[ $unameu == *WIN* || $unameu == MSYS* ]]; then
	# Should catch cygwin
	os_name="windows"

	bin_path="todo find correct windows arduino-builder path"
	user_sketchbook="todo find correct user sketchbook path"
	user_hidden_path="todo find correct arduino15 path"
else
	echo "Aborted, unsupported or unknown environment"
	return 6
fi

# Other bits we want paths for
hardware_path=$bin_path/hardware
tool_arg=$bin_path/tools-builder
toolchain=arduino-builder

# Platforms are defined with the platform control string expected by arduino-builder
# and then following arguments are sketches which should NOT be tested (blacklist)
# space-delimiting is used.
# Add new platforms by appending new target_platform array indices
declare -a target_platforms
target_platforms[0]="arduino:avr:leonardo esp32-ble.ino esp32-websockets.ino bluefruit-bleuart.ino"
target_platforms[1]="arduino:avr:uno esp32-ble.ino esp32-websockets.ino bluefruit-bleuart.ino" 
target_platforms[2]="arduino:avr:mega:cpu=atmega2560 esp32-ble.ino esp32-websockets.ino bluefruit-bleuart.ino"
target_platforms[3]="arduino:samd:mzero_bl persistence-eeprom.ino esp32-ble.ino esp32-websockets.ino bluefruit-bleuart.ino"
target_platforms[4]="esp32:esp32:esp32thing:FlashFreq=80,UploadSpeed=921600 persistence-eeprom.ino bluefruit-bleuart.ino"

# track test progress
tests_run=0
tests_pass=0
tests_fail=0

# Bash escape codes for coloured output
RED='\033[0;31m'
YELLOW='\033[0;33m'
GREEN='\033[0;32m'
LRED='\033[1;31m'
BOLD='\033[1m'
NF='\033[0m' # No Color

for platform in "${target_platforms[@]}"
do
    # Deconstruct the target_platforms array into elements based on delimiter
    IFS=" " read -r -a arr <<< "${platform}"

	echo -e "Checking examples for ${BOLD}${arr[0]}${NF}"

	# Iterate sketch files in the examples directory
	for sketch in `find ../examples/ -name '*.ino'`
	do
		# Check if the filename is in the blacklist
		sketch_name=$(basename "$sketch")	#just the filename, not whole path
		sketch_valid=0

		for blacklisted in "${arr[@]}"
		do
			if [[ $blacklisted == $sketch_name ]];
			then
				sketch_valid=1
			fi
		done 

		# Try compiling the file if its not blacklisted
		if [[ $sketch_valid == 0 ]];
		then
			tests_run=$((tests_run+1))
			# Run the build against the platform, capture (and suppress) output
			result=`$bin_path/$toolchain                       \
			-hardware $hardware_path                           \
			-hardware $user_hidden_path/packages               \
			-tools $tool_arg                                   \
			-tools $hardware_path/tools/avr                    \
			-tools $user_hidden_path/packages                  \
			-libraries $user_sketchbook/libraries              \
			-libraries ../../                                  \
			-fqbn ${arr[0]} $sketch 2>&1`

			# Check the build pass/fail status
			if [ $? -eq 0 ]
			then
				echo -e "   $(basename -a -s .ext $sketch) ${GREEN}OK${NF}"
				tests_pass=$((tests_pass+1))
			else
				echo -e "   $(basename -a -s .ext $sketch) ${RED}FAIL${NF}" >&2
				tests_fail=$((tests_fail+1))
				echo
				echo -e "Output: ${LRED}$result${NF}"
				echo
			fi
		else
			echo -e "   $(basename -a -s .ext $sketch) ${YELLOW}SKIP${NF}"
		fi

	done

	echo

done

echo "Result:  RAN:$tests_run | PASS:$tests_pass | FAIL:$tests_fail"

if [ $tests_fail -gt 0 ]
then
	exit $tests_fail
else
	echo "Everything passes!"
	exit 0
fi
