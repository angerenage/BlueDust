#!/bin/bash

# Build script for kernel and bootloader with paging setup

ORIGIN_DIR=$(pwd)

BUILD_DIR=$(pwd)/.build
PAGING_TOOL=$(pwd)/tools/pageGenerator/pageGenerator
MAPPING_FILE=$BUILD_DIR/kernel_mapping.txt
OUTPUT_MAPPING_FILE=$BUILD_DIR/generated_pages.bin
KERNEL_OUTPUT_BIN=$BUILD_DIR/kernel.bin
DISASSEMBLY_FILE=$BUILD_DIR/kernel.dis
FINAL_OS_FILE=$BUILD_DIR/os.bin

# Memory layout addresses
KERNEL_ADDR_P=0x8000
KERNEL_ADDR_V=0xFFFFFFFF80000000
HEAP_ADDR_P=0x1000
HEAP_ADDR_V=0xFFFFFFFFF7F00000
STACK_ADDR=0xFFFFFFFF7FFFD000

cd $BUILD_DIR

# Collect source files
C_FILES=$(find ../kernel ../libc -type f -name "*.c")
ASM_FILES=$(find ../kernel -type f -name "*.asm")

# Compile C files
gcc -m64 -mcmodel=large -ffreestanding -nostartfiles -nostdlib -fno-pic -O2 \
	-I../libc/include -I../kernel/include -c $C_FILES

# Assemble ASM files
for file in $ASM_FILES; do
	OUTPUT_FILE=$BUILD_DIR/$(basename "${file%.asm}.o")
	nasm -f elf64 "$file" -o $OUTPUT_FILE
done

# Link kernel and generate binary
ld -m elf_x86_64 -T ../linker.ld --gc-sections -o kernel.elf *.o
objcopy --set-section-flags .bss=alloc,load,contents kernel.elf -O binary kernel.bin
objdump -M intel -d kernel.elf > $DISASSEMBLY_FILE

# Calculate kernel size and memory alignment
KERNEL_SIZE=$(stat -c%s "kernel.bin")
KERNEL_PAGES=$((($KERNEL_SIZE + 4095) / 4096))
PAGES_ADDR=$(($KERNEL_ADDR_P + $KERNEL_PAGES * 4096))

echo "Kernel size: $KERNEL_SIZE bytes"
echo "Kernel pages: $KERNEL_PAGES"

# Function to generate memory mapping configuration
generate_mapping_file() {
	cat <<EOF > $MAPPING_FILE
# Video memory
0xB8000 0xB8000 1 0x3
# Kernel mapping
$KERNEL_ADDR_P $KERNEL_ADDR_V $KERNEL_PAGES 0x3
# Kernel stack page
0x7000 0x7000 1 0x3 # Bootloader mapping
0x7000 $STACK_ADDR 1 0x3
# Kernel heap page
$HEAP_ADDR_P $HEAP_ADDR_V 1 0x3
# Paging structures (updated dynamically)
$(printf "0x%X 0x%X" $PAGES_ADDR $PAGES_ADDR) $1 0x3
EOF
}

# Generate initial memory mapping
generate_mapping_file 1

# Generate paging structures until stabilized
PREV_GENERATED_PAGES=0
STABILIZED=false

while [ "$STABILIZED" == false ]; do
	OUTPUT=$($PAGING_TOOL $MAPPING_FILE --base=$(printf "0x%X" $PAGES_ADDR) -o $OUTPUT_MAPPING_FILE)
	GENERATED_PAGES=$(echo "$OUTPUT" | grep -oP 'Total pages written: \K\d+')

	generate_mapping_file $GENERATED_PAGES

	if [ "$GENERATED_PAGES" -eq "$PREV_GENERATED_PAGES" ]; then
		STABILIZED=true
	else
		PREV_GENERATED_PAGES=$GENERATED_PAGES
	fi
done

echo "Final mapping: $GENERATED_PAGES pages at 0x$(printf '%X' $PAGES_ADDR)"

# Add padding to align kernel binary
PADDING_SIZE=$((KERNEL_PAGES * 4096 - KERNEL_SIZE))
if [ "$PADDING_SIZE" -gt 0 ]; then
	echo "Adding padding of $PADDING_SIZE bytes to kernel"
	dd status=none if=/dev/zero bs=1 count=$PADDING_SIZE >> kernel.bin
fi

# Concatenate kernel and paging structures
cat kernel.bin $OUTPUT_MAPPING_FILE > kernel_with_pages.bin

# Build bootloader with parameters
TOTAL_SIZE=$(stat -c%s "kernel_with_pages.bin")
nasm -f bin ../bootloader.asm -o bootloader.bin \
	-DKERNEL_SIZE=$KERNEL_SIZE -DTOTAL_SIZE=$TOTAL_SIZE -DPML4_TABLE_BASE=$PAGES_ADDR

# Combine bootloader and kernel into final OS binary
cat bootloader.bin kernel_with_pages.bin > $FINAL_OS_FILE

cd $ORIGIN_DIR
