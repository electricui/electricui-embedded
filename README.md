ge# electricui-embedded

ElectricUI communications and handling library for use on simplistic embedded microcontrollers. Communicates with a ElectricUI compatible UI or device.

See the docs or website for more information.

## Getting Started

### Arduino

1. Ensure the Arduino IDE is installed (version >1.6.X tested ok) and the standard Arduino Blink.ino sketch compiles/flashes to the dev board.

2. Clone or download the electricui-embedded repo onto your computer. Put it whereever you like and use symbolic links, or copy the contents into "*/arduino/libraries/electricui*" folder as typically done with Arduino libraries. 

Symbolic folder links are a nice way to separate the library from the Arduino IDE install if you are planning on editing the libraries.

Unix (your Arduino sketchbook location may vary by OS/install):

```
sudo ln -s ~/projects/electricui-embedded/ ~/Arduino/libraries/electricui
```

Windows/Arduino can demonstrate issues with symbolic links across drives.

If you don't plan on developing electricui-embedded, copy or clone it straight into your libraries folder.

3. In the Arduino IDE, one can simply click *File > Examples > electricui > Examples > Basics > hello-blink* to load an example sketch.

4. If using the ElectricUI helloboard, set the board with *Tools > Board > "Arduino Leonardo"*

5. Test building and flashing the firmware to the board!


### Other Microcontrollers 

1. Just clone the repo and use the electricui files. Import into your project as normal and ensure the minimum setup functions are called.

2. The library assumes that you will provide a pointer to the serial tx function which accepts a char, this could be putc() or similar single byte uart_tx_write(uint8) function.

3. For for more detail, follow the docs or example Arduino code which shows the minimum setup and usage examples.

___


# Running tests

Testing uses the [Ceedling](http://www.throwtheswitch.org/ceedling/) (Ruby|rake) based testing framework.

I don't provide Ceedling's vendor files inside this repo, so first runs need to 're-initalise' the test structure.

1. Use the provided `test/setup_tests.sh` script to begin with. 
	- You might need to add execution permission first. From `/test`, run `chmod +x setup_tests.sh`.

2. Run the `setup_tests.sh` script. If you don't have ceedling installed, it will prompt to install the ruby gem.

3. Once setup, run `ceedling` or `ceedling test:all`.

## Coverage Analysis

Run `ceedling gcov:all` to generate the coverage reports.  
Use `ceedling utils:gcov` to generate a pretty HTML report.

___

# Overheads and Benchmarks

*OUTDATED*

TODO: Re-run these and compare.

The baseline "starting consumption without eUI" is shown first, subsequent rows are adding something to the previous setup.

Uses the Arduino avr-g++ compiler with default settings (-0s, C++11)

|                       | Flash           | Global Vars       |
| --------------------- | --------------- | ----------------- |
| No eUI                | 5354            | 327               |
| eUI, 1 conn           | 7956            | 563               |
| +extra conn           | 8054      	  | 706               |
| +track the uint8      | 8084      	  | 716               |
| +track all vars       | 8296            | 908               |

Cost of adding eUI to project               = 2602 bytes of PROGMEM, 236 bytes of RAM
Cost of additional eUI connection methods   = 98 bytes of PROGMEM,   143 bytes of RAM each
Cost of tracking a variable with eUI        = 30 bytes of PROGMEM,   10 bytes of RAM.

These results will vary with quantity and type due to alignment of flash words/pages, compiler optimisation etc.