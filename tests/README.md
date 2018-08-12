# ElectricUI Embedded Library Tests

We use [Unity](http://www.throwtheswitch.org/unity) for unit testing.

I don't want to include Unity in this repo, so you need a copy in this folder called "Unity-master" which can be downloaded from the site (github).  
The makefile is able to download and unzip with bash shells, windows needs some love before it can be automated.  

# Running tests

If Unity hasn't been downloaded/installed yet, run ```make first``` and it will prepare it for you, then run tests!  

- With your shell in /test, run ```make all```.
- The test program will clean any existing artifacts, build and then run the "eui_tests" program.
- Pass fail will be at the end of execution, with specific errors or ignored cases printed to the shell during execution.

# Adding tests

 1. Add a single test inside the runner for a given source file, ```runner_electricui.c``` inside an existing group or create a new group as needed.
 2. Add the test boilerplate to the test file, ```test_electricui.c```.
 3. The main in ```all_tests.c``` runs the test groups if you created a new group, you'll need to ensure its called in there.

Follow the examples on the Unity webpage/docs as needed. It isn't that complicated.

# Other

Running ```make``` will just build the test program, but won't run it.  
To clean build products, ```make clean```.  
To run the built product, ```make run```.  
You can just download unity and get it setup by using ```make download```.  