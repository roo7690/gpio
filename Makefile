.SILENT:

help:	##Pour voir les options disponibles, et leurs actions
	chmod +x bin/makefile_help
	./bin/makefile_help

build-led: led/*.c led/*.h ##Pour compiler le programme de la led
	@if [ "$(PICO)" = "1" ]; then \
		rm -rf "$(wf)"/led/build; \
		mkdir "$(wf)"/led/build; \
		cd "$(wf)"/led/build; \
		cmake ..; \
		make; \
		cd "$(wf)"; \
	else \
		gcc -o "$(wf)"/bin/led "$(wf)"/led/*.c -lwiringPi; \
	fi