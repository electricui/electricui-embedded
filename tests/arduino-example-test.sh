#!/bin/bash
# Compiles each of the Arduino example sketches in electricui-embedded/examples

# Setup the various paths to the installed location, user's sketchbook and the library/board manager cache
bin_path=/opt/arduino/arduino-1.8.4
user_sketchbook=~/Arduino
user_hidden_path=~/.arduino15

# Other bits we want helpers for
hardware_path=$bin_path/hardware
tool_arg=$bin_path/tools-builder
toolchain=arduino-builder
declare -a target_platform=(	"arduino:avr:leonardo" 
								"arduino:avr:uno" 
								"arduino:avr:mega:cpu=atmega2560"
								"esp32:esp32:esp32thing:FlashFreq=80,UploadSpeed=921600"
							)

# todo test the esp32 and 8266 as it uses g++ toolchain!

# track test progress
tests_run=0
tests_pass=0
tests_fail=0

## Run tests for each platform
for platform in "${target_platform[@]}"
do
	echo "Checking examples for $platform"
	for sketch in `find ../examples/ -name '*.ino'`
	do
		tests_run=$((tests_run+1))
		# Run the build against the platform, capture (and suppress) output
		result=`$bin_path/$toolchain                       \
		-hardware $hardware_path                           \
		-hardware $user_hidden_path/packages               \
		-tools $tool_arg                                   \
		-tools $hardware_path/tools/avr                    \
		-tools $user_hidden_path/packages                  \
		-libraries $user_sketchbook/libraries              \
		-libraries ../                                     \
		-fqbn $platform $sketch 2>&1`

		# Check the build pass/fail status
		if [ $? -eq 0 ]
		then
			echo "$(basename -a -s .ext $sketch) OK"
			tests_pass=$((tests_pass+1))
		else
			echo "$(basename -a -s .ext $sketch) FAIL" >&2
			tests_fail=$((tests_fail+1))
			echo "Output: $result"
		fi
	done
done

echo "Result:  RAN:$tests_run | PASS:$tests_pass | FAIL:$tests_fail"

if [ $tests_fail -gt 0 ]
then
	exit $tests_fail
else
	echo "Everything passes!"
	exit 0
fi
