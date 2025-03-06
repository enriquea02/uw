# Compiler and Flags
CC = g++
LDFLAGS = -lwiringPi

# Source and Object Files
SOURCES = laser_proj_top.cc rpi-lasershow/ABE_ADCDACPi.cpp rpi-lasershow/IldaReader.cpp rpi-lasershow/Frame.cpp rpi-lasershow/FrameData.cpp rpi-lasershow/Points.cpp
OBJECTS = $(SOURCES:%.cc=../bld/%.o) # Object files in ../bld directory

# Output Executable Name and Path
EXECUTABLE = ../bld/laser_proj_top

# Header Include Paths
INCLUDE_PATHS = -I./rpi-lasershow -I../lib

# Motor Driver Target (for user argument)
MOTOR_DRIVER_TARGET = motor_driver
MOTOR_DRIVER_SOURCES = motor_drvr_lib.cc rpi-lasershow/ABE_ADCDACPi.cpp
MOTOR_DRIVER_OBJECTS = $(MOTOR_DRIVER_SOURCES:%.cc=../bld/%.o)

MOTOR_DRIVER_EXECUTABLE = ../bld/motor_driver

# Rule to build the main executable
$(EXECUTABLE): $(OBJECTS)
		$(CC) $(INCLUDE_PATHS) $(OBJECTS) $(LDFLAGS) -o $(EXECUTABLE)

# Rule to compile object files
../bld/%.o: %.cc
	mkdir -p ../bld
	$(CC) $(INCLUDE_PATHS) -c $< -o ../bld/$(notdir $@)

# Rule to build motor_driver executable
$(MOTOR_DRIVER_TARGET): $(MOTOR_DRIVER_OBJECTS)
	$(CC) $(INCLUDE_PATHS) $(MOTOR_DRIVER_OBJECTS) $(LDFLAGS) -o $(MOTOR_DRIVER_EXECUTABLE)
		
../bld/%.o: %.cpp
	mkdir -p ../bld
	$(CC) $(INCLUDE_PATHS) -c $< -o ../bld/$(notdir $@)

# Rule to clean up object files and executable
clean:
		rm -rf ../bld/* $(MOTOR_DRIVER_TARGET)

# Rule to run the program
run: $(EXECUTABLE)
		$(EXECUTABLE)

# Rule to rebuild the executable
rebuild: clean $(EXECUTABLE)

# Rule to remove the old executable on each run
.PHONY: run