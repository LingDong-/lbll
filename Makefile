cli0:
	gcc -O3 cli/main.0.c -o lbll
cli1:
	gcc -O3 cli/main.1.c -o lbll
em:
	mkdir -p site;\
	../emsdk/upstream/emscripten/emcc \
	tools/lblljs.c -s WASM=0 -O3 -o site/lbll.js --memory-init-file 0 \
	-s EXPORTED_FUNCTIONS='["_run","_init"]' \
	-s EXPORTED_RUNTIME_METHODS='["cwrap"]' \
	-s MODULARIZE=1 -s 'EXPORT_NAME="LBLL"'
web:
	mkdir -p site;\
	node tools/make_site.js
ino-c:
	cp lbll.h uc/lbll-ino/lbll.h;\
	arduino-cli compile --fqbn $(fqbn) ./uc/lbll-ino;
ino-f:
	cp lbll.h uc/lbll-ino/lbll.h;\
	PORT=$$(ls /dev/cu.usb*);\
	echo $$PORT;\
	arduino-cli compile -p $$PORT -ut --fqbn $(fqbn) ./uc/lbll-ino;
ino-m:
	while [ -z $$PORT ]; do \
	PORT=$$(ls /dev/cu.usb* 2> /dev/null);\
	done;\
	echo $$PORT;\
	arduino-cli monitor -p $$PORT;
ino-fm: ino-f ino-m
vsc-ext:
	mkdir -p ~/.vscode/extensions/lb1l-vsc/syntaxes;\
	node tools/make_vsc.js 1 > ~/.vscode/extensions/lb1l-vsc/package.json;\
	node tools/make_vsc.js 2 > ~/.vscode/extensions/lb1l-vsc/language-configuration.json;\
	node tools/make_vsc.js 3 > ~/.vscode/extensions/lb1l-vsc/syntaxes/lb1l.tmLanguage.json;


fqbn = Fab_SAM_Arduino:samd:x21e:float=default,config=disabled,clock=internal_usb,timer=timer_732Hz,cpu=samd21e18a,bootloader=8kb,serial=one_uart,usb=cdc
