`g++ imgconv.cpp -o imgconv.o && ./imgconv.o 1 1 ../fonts/fixedsys-excelsior-301.ttf_16x16.png`

`objcopy -I binary -O elf64-x86-64 --rename-section .data=.rodata,alloc,load,readonly,contents fixedsys.bin binary.o`

`g++ binary.o emulator.cpp -o emulator.o console/console.ansi.cpp -g -z noexecstack`