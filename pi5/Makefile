.SILENT:

micro: services/micro.c
	gcc -o "$(WFM)"/bin/micro ./services/micro.c

led: tools/led/*.c
	gcc -o "$(WFM)"/bin/led abstract/*.c tools/tasks/*.c tools/led/*.c -lwiringPi;

joystick: tools/joystick/*.c
	gcc -o "$(WFM)"/bin/joystick abstract/*.c tools/tasks/*.c tools/joystick/*.c -lwiringPi;