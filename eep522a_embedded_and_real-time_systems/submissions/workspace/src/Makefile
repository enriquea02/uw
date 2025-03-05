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
MOTOR_DRIVER_SOURCE = motor_drvr_lib.c
MOTOR_DRIVER_OBJECT = ../bld/motor_drvr_lib.o

# Rule to build the main executable
$(EXECUTABLE): $(OBJECTS)
		$(CC) $(INCLUDE_PATHS) $(OBJECTS) $(LDFLAGS) -o $(EXECUTABLE)

# Rule to compile object files
../bld/%.o: %.cc
		mkdir -p ../bld
		$(CC) $(INCLUDE_PATHS) -c $< -o $@

# Rule to build motor_driver executable (when MOTOR_DRIVER=1)
$(MOTOR_DRIVER_TARGET): $(MOTOR_DRIVER_OBJECT)
	$(CC) $(INCLUDE_PATHS) $(MOTOR_DRIVER_OBJECT) $(LDFLAGS) -o $(MOTOR_DRIVER_TARGET)

../bld/motor_drvr_lib.o: $(MOTOR_DRIVER_SOURCE)
	mkdir -p ../bld
	$(CC) $(INCLUDE_PATHS) -c $< -o $@


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
run:
		rm -f ../bld/laser_proj_top
		make $(EXECUTABLE)
		$(EXECUTABLE)