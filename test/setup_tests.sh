#!/bin/bash

echo "Checking if ceedling already exists on the system..."

# Check the ruby gem ceedling is installed
GEM_INSTALLED=`gem list -i ceedling`

if [ $GEM_INSTALLED == "true" ]
then
	echo "Ceedling gem found, awesome!"
else
	# Prompt to install it
	read -p "Install the ceedling ruby gem?" -n 1 -r
	echo    # (optional) move to a new line
	if [[ $REPLY =~ ^[Yy]$ ]]
	then
		# install it
		sudo gem install ceedling
	else
		echo "Error: We need ceedling for test harnesses."
		exit 0
	fi		
fi

# Setup project
echo "Populating the test folder with vendor files..."
sleep 1 # Let the user read the line
cd ../
echo "n" | ceedling new test

echo " - Execute 'ceedling test:all' to run all tests"