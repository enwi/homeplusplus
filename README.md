# homeplusplus
This is Home++ a smart home system based on C++ with a focus on ease of use (setup, user interaction, ...) though at the moment lacking a bit on the front end side.

Features:
*  Actions
   * Set device properties
   * Toggle device properties
   * Display notifications
*  Rules (lacking easy creation through GUI)
   * Set device properties
   * Toggle device properties
   * Display notifications
   * Call other actions
*  Notifications
   * Informations
   * Warnings
*  Multiple users with different rights (lacking configuration in GUI)
   * password protected

Supported devices:
*  Philips Hue lights
*  Tasmota devices (almost all sensors and actuators)
*  Remote sockets (433MHz only on Raspberry Pi)

## Installation process

### Prerequisites
Home++ needs [mosquitto](https://mosquitto.org/), so install it:
*  Linux: `sudo apt-get install mosquitto`
*  MacOs: `brew install mosquitto`
*  Windows: [Download](https://mosquitto.org/download/) and install/run exe

### Build Home++ backend
Create a new folder inside the `backend` folder, in this example we are using `bin` as our 'binary' folder
*  `cd backend`
*  `mkdir bin`
*  `cd bin`

Then decide whether you want support for remote sockets (433MHz) aka use the remote socket API or not (only on Raspberry Pi, requires RCSwitch-Pi).
If so run cmake like this:
*  `cmake .. -DCMAKE_BUILD_TYPE=Release -DHomePlusPlus_REMOTE_SOCKET=ON`

If not like this:
*  `cmake .. -DCMAKE_BUILD_TYPE=Release`

Then compile the backend with:
*  `make HomePlusPlus`

### Run Home++ backend
You can then run the smart home with:
*  `./HomePlusPlus -logDir /path/to/log/folder -dir /path/to/resources -cLogL 1 -logL 1`

#### Explanation of parameters
*  `-logDir <path>`: absolute path to a folder where logfiles are stored
*  `-dir <path>`: absolute path to the resource folder contained in this repository (this folder can be moved elsewhere)
*  `-cLogL <0-5>`: the console log level (0: debug, 1: info, 2: warning, 3: error, 4: severe, 5: none)
*  `-logL <0-5>`: the file log level (0: debug, 1: info, 2: warning, 3: error, 4: severe, 5: none)
*  `-debug`: run in debug mode


### Build Home++ frontend
If not installed, install `npm`
*  https://www.npmjs.com/get-npm

Switch to the `frontend` folder:
*  `cd frontend`

Install all necessary packages:
*  `npm install`

Compile the frontend:
*  `ng build --prod`

For a German frontend run:
*  `ng build --prod --configuration=de`

This will generate a new folder called `dist`. 
Inside it you will find the folder `app` in which the compiled frontend lies. 
These files can then be served/hosted with a simple node server.
> A setup of said server will follow soon


## Debugging

### Backend
Run `cmake` with option `-DCMAKE_BUILD_TYPE=Debug` instead of `-DCMAKE_BUILD_TYPE=Release`

### Frontend
Instead of building with `ng build --prod --configuration=de` 
run `ng serve --host 0.0.0.0` with or without the option `--configuration=de` to serve and debug the webpage.


## Testing

### Backend
Run `cmake` with option `-DBUILD_TESTING=ON` instead of `-DBUILD_TESTING=OFF` to generally enable testing and coverage tests.
For address sanitizer tests also add the option `-DHomePlusPlus_Test_ASAN=ON`.
For undefined behavior sanitizer tests also add the option `-DHomePlusPlus_Test_UBSAN=ON`.

Compile the coverage tests with:
*  `make HomePlusPlus_Test`

Execute the coverage tests with:
*  `./tests/HomePlusPlus_Test`

Compile the address sanitizer tests with:
*  `make HomePlusPlus_ATest`

Execute the address sanitizer tests with:
*  `./tests/HomePlusPlus_ATest`

Compile the undefined behavior sanitizer tests with:
*  `make HomePlusPlus_UTest`

Execute the undefined behavior sanitizer tests with:
*  `./tests/HomePlusPlus_UTest`

### Frontend
Currently there are no tests, but feel free to add some and create a pull request.
