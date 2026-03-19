.SUFFIXES:

override OUTPUT := Espresso
ARCH ?= x86_64

ISO_IMAGE := image.iso
ISO_ROOT := iso_root

CC := clang
LD := ld.lld

CFLAGS := -g -O2 -pipe
# 추후에 사용될 때를 위하여
CPPFLAGS :=
NASMFLAGS := -g
LDFLAGS :=

WASM3_DIR := src/runtime/wasm3

# 크로스 컴파일 설정
override CC += -target $(ARCH)-unknown-none-elf

override CFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu11 \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -fno-PIC \
    -ffunction-sections \
    -fdata-sections \
    -m64 \
    -march=x86-64 \
    -mabi=sysv \
    -mno-mmx \
    -mno-red-zone \
    -mcmodel=kernel \
    -fno-builtin \
    -fno-strict-aliasing \

override CPPFLAGS := \
	-nostdinc \
    -I src/kernel \
    -I src/kernel/include \
	-I $(WASM3_DIR)/source \
    $(CPPFLAGS) \
    -MMD \
    -MP

override NASMFLAGS := \
    -f elf64 \
    $(patsubst -g,-g -F dwarf,$(NASMFLAGS)) \
    -Wall

override LDFLAGS += \
    -m elf_x86_64 \
    -nostdlib \
    -static \
    -z max-page-size=0x1000 \
    --gc-sections \
    -T linker.lds


CLANG_RESOURCE_DIR := $(shell $(CC) -print-resource-dir)
override CPPFLAGS += -I $(CLANG_RESOURCE_DIR)/include

override CPPFLAGS += -D__$(ARCH)__

# 소스 파일 찾기
override COMMON_SRC := $(shell find -L src/kernel -type f 2>/dev/null | LC_ALL=C sort)
override WASM3_SRC := $(shell find -L $(WASM3_DIR)/source -type f -name "*.c" 2>/dev/null)
override SRCFILES := $(COMMON_SRC) $(WASM3_SRC)

override CFILES := $(filter %.c,$(SRCFILES))
override ASFILES := $(filter %.S,$(SRCFILES))
override NASMFILES := $(filter %.asm,$(SRCFILES))

override OBJ := $(addprefix obj/,$(CFILES:.c=.c.o) $(ASFILES:.S=.S.o) $(NASMFILES:.asm=.asm.o))
override HEADER_DEPS := $(addprefix obj/,$(CFILES:.c=.c.d) $(ASFILES:.S=.S.d))

.PHONY: all
# 출력 경로 설정
all: bin/$(OUTPUT)

# 헤더 의존성 설정
-include $(HEADER_DEPS)

# 최종 Executable 규칙
bin/$(OUTPUT): makefile linker.lds $(OBJ)
	mkdir -p "$(dir $@)"
	$(LD) $(LDFLAGS) $(OBJ) -o $@

# *.c 파일 컴파일 규칙
obj/%.c.o: %.c makefile
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# *.S 파일 컴파일 규칙
obj/%.S.o: %.S makefile
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# *.asm 컴파일 규칙
obj/%.asm.o: %.asm makefile
	mkdir -p "$(dir $@)"
	nasm $(NASMFLAGS) $< -o $@

.PHONY: iso
iso: bin/$(OUTPUT)
    # Limine 다운로드 & 빌드
	[ -d limine ] || git clone https://codeberg.org/Limine/Limine.git limine --branch=v10.x-binary --depth=1
	$(MAKE) -C limine

	# 디렉터리 구성
	rm -rf $(ISO_ROOT)
	mkdir -p $(ISO_ROOT)/boot/limine $(ISO_ROOT)/EFI/BOOT
	
	# 파일 복사
	cp -v bin/$(OUTPUT) $(ISO_ROOT)/boot/
	cp -v limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin \
	     limine/limine-uefi-cd.bin $(ISO_ROOT)/boot/limine/
	cp -v limine/BOOTX64.EFI limine/BOOTIA32.EFI $(ISO_ROOT)/EFI/BOOT/

	# ISO 생성
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
	        -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
	        -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
	        -efi-boot-part --efi-boot-image --protective-msdos-label \
	        $(ISO_ROOT) -o $(ISO_IMAGE)
	
	./limine/limine bios-install $(ISO_IMAGE)

.PHONY: qemu
qemu: iso
	qemu-system-x86_64 \
		-enable-kvm \
		-m 8G \
		-machine q35 \
		-cpu host \
		-smp 4 \
		-bios /usr/share/ovmf/OVMF.fd \
		-cdrom $(ISO_IMAGE) \
		-serial stdio

.PHONY: clean
clean:
	rm -rf bin obj $(ISO_ROOT) $(ISO_IMAGE)