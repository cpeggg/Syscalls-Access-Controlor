cmd_/home/cpegg/fileaudit/test/test.ko := ld -r -m elf_x86_64 -z max-page-size=0x200000 -T ./scripts/module-common.lds --build-id  -o /home/cpegg/fileaudit/test/test.ko /home/cpegg/fileaudit/test/test.o /home/cpegg/fileaudit/test/test.mod.o ;  true
