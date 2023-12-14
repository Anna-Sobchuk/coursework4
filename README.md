# coursework 4th year
## Basic logic and reasons
The provided repository consists code for implementing inverted index during client-server communication. Main parts are:
* The index itself and tests for it
* Client and server for sending and receiving the index and words that are wanted to be found
* Time and correctness of paralelism tests
* Schema providing steps and the way program works
## Architecture
- ClientProgram.cpp - client part of the communication
- ServerProgram.cpp - server part of the communication
- Tests - tests for the functions of IndexProgram
- TimeAnalysis.cpp - time and work correctness analysis
- IndexProgram.cpp - File manager, index performing and RunIndex which is the main work organizer

![image](https://github.com/Anna-Sobchuk/coursework4/assets/77459095/949d0467-a9f4-4be1-ad38-b6013b425940)

## How to run
* Clone the solution folder
* Navigate to the project directory
* Make sure that CMake is installed on your system
* Make a build directory: mkdir build cd build
* Build the project cmake --build . --config Release
* After build, executable is located in the build dir. Run from the build dir
* Go to the solution folder and first run server, then client
* If you want to check tests, then separately run Tests and TimeAnalysis

## Tests and correctness
- All the tests passed:

![image](https://github.com/Anna-Sobchuk/coursework4/assets/77459095/b5f6b250-1355-4f54-8501-1852ea07fac2)

- Time analysis and equality correctness. At that time and state 12 threads seem to be an optimal solution:

![image](https://github.com/Anna-Sobchuk/coursework4/assets/77459095/6cbbd446-4403-4ac1-9dc0-2c81c811356b)
