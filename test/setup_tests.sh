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
		PATH="`ruby -e 'puts Gem.user_dir'`/bin:$PATH"
		export PATH
		gem install --user-install ceedling
	else
		echo "Error: We need ceedling for test harnesses."
		exit 1
	fi		
fi

# Setup project
echo "Populating the test folder with vendor files..."
sleep 1 # Let the user read the line
cd ../
echo "n" | ceedling new test


echo " - Execute 'ceedling test:all' to run all tests"
echo ""
echo "If ceedling was installed by this script, its only available for your current user."
echo "If ceedling can't be found by your shell, ensure `ruby -e 'puts Gem.user_dir'`/bin is in your PATH."

exit 0