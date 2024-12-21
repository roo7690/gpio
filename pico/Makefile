.SILENT:

build-led: led/*.c led/*.h ##Pour compiler le programme de la led
	rm -rf "$(WFM)"/pico/led/build; \
	mkdir "$(WFM)"/pico/led/build; \
	cd "$(WFM)"/pico/led/build; \
	cmake ..; \
	make; \
	cd "$(WFM)"; \