CC=gcc
AS=as
CFLAGS=-O1 -std=c99 -Wall -Werror -nostdinc -Iinclude -msoft-float -mno-sse -mno-red-zone -fno-builtin -fPIC -mtune=amdfam10 -g3 -include include/sys/config.h
#CFLAGS=-O1 -std=c99 -Wall -nostdinc -Iinclude -msoft-float -mno-sse -mno-red-zone -fno-builtin -fPIC -mtune=amdfam10 -g3 -include include/sys/config.h
LD=ld
LDLAGS=-nostdlib
AR=ar

ROOTFS=rootfs
ROOTBIN=$(ROOTFS)/bin
ROOTLIB=$(ROOTFS)/lib
ROOTBOOT=$(ROOTFS)/boot

KERN_SRCS:=$(wildcard sys/*.c sys/*.s sys/*/*.c sys/*/*.s)
BIN_SRCS:=$(wildcard bin/*/*.c)
LIBC_SRCS:=$(wildcard libc/*.c libc/*/*.c)
LD_SRCS:=$(wildcard ld/*.c)
BINS:=$(addprefix $(ROOTFS)/,$(wildcard bin/*))

.PHONY: all binary

all: $(USER).iso $(USER).img

$(USER).iso: kernel
	cp kernel $(ROOTBOOT)/kernel/kernel
	mkisofs -r -no-emul-boot -input-charset utf-8 -b boot/cdboot -o $@ $(ROOTFS)/

$(USER).img:
	qemu-img create -f raw $@ 16M

kernel: $(patsubst %.s,obj/%.asm.o,$(KERN_SRCS:%.c=obj/%.o)) obj/tarfs.o
	$(LD) $(LDLAGS) -o $@ -T linker.script $^

$(ROOTFS)/mnt:
	mkdir $(ROOTFS)/mnt
	touch $(ROOTFS)/mnt/.empty

obj/tarfs.o: $(BINS) $(ROOTFS)/mnt
	tar --format=ustar -cvf tarfs --no-recursion -C $(ROOTFS) $(shell find $(ROOTFS)/ -name boot -prune -o ! -name .empty -printf "%P\n")
	objcopy --input binary --binary-architecture i386 --output elf64-x86-64 tarfs $@
	@rm tarfs

$(ROOTLIB)/libc.a: $(LIBC_SRCS:%.c=obj/%.o)
	$(AR) rcs $@ $^

$(ROOTLIB)/libc.so: $(LIBC_SRCS:%.c=obj/%.o) $(ROOTLIB)/ld.so
	$(LD) $(LDLAGS) -shared -soname=$@ --dynamic-linker=/lib/ld.so --rpath-link=/lib -o $@ $^

$(ROOTLIB)/ld.so: $(LD_SRCS:%.c=obj/%.o)
	$(LD) $(LDLAGS) -shared -o $@ $^

$(ROOTLIB)/crt1.o: obj/crt/crt1.o
	cp $^ $@

$(BINS): $(ROOTLIB)/crt1.o $(ROOTLIB)/libc.a $(ROOTLIB)/libc.so $(shell find bin/ -type f -name *.c)
	@$(MAKE) --no-print-directory BIN=$@ binary

binary: $(patsubst %.c,obj/%.o,$(wildcard $(BIN:rootfs/%=%)/*.c))
	$(LD) $(LDLAGS) -o $(BIN) $(ROOTLIB)/crt1.o $^ $(ROOTLIB)/libc.a

obj/%.o: %.c $(wildcard include/*.h include/*/*.h)
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $<

obj/%.asm.o: %.s
	@mkdir -p $(dir $@)
	$(AS) -o $@ $<

.PHONY: submitxxx clean

SUBMITTO:=~mferdman/cse506-submit/

submit: clean
	tar -czvf $(USER).tgz --exclude=.empty --exclude=.*.sw? --exclude=*~ LICENSE README README_project Makefile linker.script sys bin crt libc ld include $(ROOTFS) $(USER).img
	@gpg --quiet --import cse506-pubkey.txt
	gpg --yes --encrypt --recipient 'CSE506' $(USER).tgz
	rm -fv $(SUBMITTO)$(USER)=*.tgz.gpg
	cp -v $(USER).tgz.gpg $(SUBMITTO)$(USER)=`date +%F=%T`.tgz.gpg

clean:
	find $(ROOTLIB) $(ROOTBIN) -type f ! -name .empty -print -delete
	rm -rfv obj kernel $(ROOTBOOT)/kernel/kernel
	rm -rf *.iso 
#	rm -rf *.iso *.img

all: $(USER).iso
new: clean all

r:
	qemu-system-x86_64 -curses -cdrom $(USER).iso -hda $(USER).img -gdb tcp::2424

rg:
	qemu-system-x86_64 -curses -cdrom $(USER).iso -hda $(USER).img -gdb tcp::2424 -S

# run QEMU with AHCI and network

ra:
	qemu-system-x86_64 -curses -cdrom $(USER).iso -drive id=disk,file=$(USER).img,if=none -device ahci,id=ahci -device ide-drive,drive=disk,bus=ahci.0 -net nic -net user,hostfwd=tcp::10080-:80 -net user,hostfwd=tcp::10023-:23 -gdb tcp::2424

rag: 
	qemu-system-x86_64 -curses -cdrom $(USER).iso -drive id=disk,file=$(USER).img,if=none -device ahci,id=ahci -device ide-drive,drive=disk,bus=ahci.0 -net nic -net user,hostfwd=tcp::10080-:80 -net user,hostfwd=tcp::10023-:23 -gdb tcp::2424 -S

g:
	gdb kernel 

n: new
nr: n r
nrg: n rg
nra: n ra
nrag: n rag
#	/usr/bin/gdb kernel 
