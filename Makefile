.SILENT:

help:	##Pour voir les options disponibles, et leurs actions
	chmod +x bin/makefile_help
	./bin/makefile_help

build-led: led/*.c led/*.h ##Pour compiler le programme de la led
	@if [ "$(pico)" = "1" ]; then \
		rm -rf "$(wfm)"/led/build; \
		mkdir "$(wfm)"/led/build; \
		cd "$(wfm)"/led/build; \
		cmake ..; \
		make; \
		cd "$(wfm)"; \
	else \
		gcc -Iabstract/gpio -o "$(wfm)"/bin/led \
		"$(wfm)"/abstract/gpio/gpio_pi5.c "$(wfm)"/abstract/sdk/sdk_pi5.c \
		"$(wfm)"/led/*.c -lwiringPi; \
	fi