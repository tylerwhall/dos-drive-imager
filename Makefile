# https://wiki.archlinux.org/index.php/Open_Watcom
export WATCOM = /opt/watcom
export INCLUDE = $(WATCOM)/h
export PATH := $(WATCOM)/binl64:$(PATH)

#
# Specify target platform (16-bit real-mode DOS)
# instruction set (vanilla 8086)
# maximum warnings and warnings should be treated as errors.
# floating point instructions (soft emulation),
# Memory model small (near code, near data)
WCC_COMMON = \
	-bt=dos \
	-0 \
	-wx \
	-we \
	-fpc \
	-ms \

WCC_FLAGS = $(WCC_COMMON) -zpw
WCC_FLAGS += -zc -ze -r

# -s    Disable stack checks
# -oh   Enable expensive optimizations
# -ol   Enable loop optimizations
# -ot   Optimize for speed (exclusive w/ -os)
# -oi   Inline functions (outp)
WCC_FLAGS += -s -oh -ot -oi

# Add DWARF debugging table
#WCC_FLAGS += -d2 -hd

WASM_FLAGS = $(WCC_COMMON) -d1 -dROMDISK_WORKAROUND

TARGETS = \
	image.exe \

all: $(TARGETS)


%.hex: %.bin
	@echo " -CONV-   Converting '$<' -> '$@'"
	@objcopy -I binary -O ihex $< $@

# Watcom rules

%.obj %.lst : %.c
	@echo " -WCC-    Compiling '$<'"
	@wcc -q $(WCC_FLAGS) -fo=$@ $<
	@wdis -s=$< -a -e -p -l=$*.lst $@
	@chmod 644 $*.lst

%.obj: %.asm
	@echo " -WASM-   Assembling '$<'"
	@wasm -zq $(WASM_FLAGS) -fo=$@ $<

.PRECIOUS: %.obj %.bin

%.exe: %.obj
	@echo " -WLINK-  Linking '$@'"
	@WL_TARGET=$* wlink @exe.lnk file { $^ }

%.com: %.obj
	@echo " -WLINK-  Linking '$@'"
	@WL_TARGET=$* wlink @com.lnk file { $^ }

clean:
	rm -rf *~ *.o *.obj *.lst *.err *.exe *.com $(TARGETS)
