# -*- makefile -*-

# Output format. (can be srec, ihex, binary)
ifndef FORMAT
  FORMAT = ihex
endif

# Build for debug / release.
ifndef BUILD
  BUILD = debug
endif

ifdef VERBOSE
  ECHO =
  NEWLINE = @echo
else
  ECHO = @
  NEWLINE = @
endif

# List C source files here. (C dependencies are automatically generated.)
ARTX_SRC = src/task.c \
           src/serial.c \
           src/spi.c \
           src/util.c \
           src/monitor.c \
           src/decimal.c \
           src/sleep.c \
           src/isr.c \
           src/twi.c \
           src/date.c
ARTX_OBJ = $(ARTX_SRC:%.c=.build/artx/%.o)
ARTX_LST = $(ARTX_OBJ:.o=.lst)

# Optimization level, can be [0, 1, 2, 3, s].
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
ifndef OPT
  OPT = 2
endif

# Debugging format.
# Native formats for AVR-GCC's -g are stabs [default], or dwarf-2.
# AVR (extended) COFF requires stabs, plus an avr-objcopy run.
ifndef DEBUG
  DEBUG = stabs
endif
#DEBUG = dwarf-2
#DEBUG = 0

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
EXTRAINCDIRS =

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Compiler flags.
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
ifeq ($(BUILD),release)
  CFLAGS = -ffunction-sections -fdata-sections
else
  CFLAGS = -g$(DEBUG)
endif
CFLAGS += $(TESTCFLAGS)
CFLAGS += $(CDEFS) $(CINCS) -I$(ARTX_ROOT)/include
CFLAGS += -O$(OPT)
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -Wa,-adhlns=$(@:.o=.lst)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += $(CSTANDARD)
CFLAGS += -DF_OSC=$(F_OSC)

EXTRA_WARN = -Wunused -Wstrict-prototypes -Wmissing-prototypes -Werror
# EXTRA_WARN =

WARNFLAGS = -Wall -Wextra -Wundef -Wshadow -Wbad-function-cast -Wc++-compat \
            -Wcast-qual -Wmissing-noreturn -Winline -Wuninitialized \
            $(EXTRA_WARN)

# Assembler flags.
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -ahlms:    create listing
#  -gstabs:   have the assembler create line number information; note that
#             for use in COFF files, additional information about filenames
#             and function names needs to be present in the assembler source
#             files -- see avr-libc docs [FIXME: not yet described there]
ASFLAGS = -Wa,-adhlns=$(<:.S=.lst),-gstabs
ASFLAGS += -DF_OSC=$(F_OSC)


#Additional libraries.

# Minimalistic printf version
PRINTF_LIB_MIN = -Wl,-u,vfprintf -lprintf_min

# Floating point printf version (requires MATH_LIB = -lm below)
PRINTF_LIB_FLOAT = -Wl,-u,vfprintf -lprintf_flt

PRINTF_LIB =

# Minimalistic scanf version
SCANF_LIB_MIN = -Wl,-u,vfscanf -lscanf_min

# Floating point + %[ scanf version (requires MATH_LIB = -lm below)
SCANF_LIB_FLOAT = -Wl,-u,vfscanf -lscanf_flt

SCANF_LIB =

# MATH_LIB = -lm
MATH_LIB =

# External memory options

# 64 KB of external RAM, starting after internal RAM (ATmega128!),
# used for variables (.data/.bss) and heap (malloc()).
#EXTMEMOPTS = -Wl,-Tdata=0x801100,--defsym=__heap_end=0x80ffff

# 64 KB of external RAM, starting after internal RAM (ATmega128!),
# only used for heap (malloc()).
#EXTMEMOPTS = -Wl,--defsym=__heap_start=0x801100,--defsym=__heap_end=0x80ffff

EXTMEMOPTS =

# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -Wl,-Map=$(TARGET).map,--cref,--gc-sections,--relax
LDFLAGS += $(EXTMEMOPTS)
# LDFLAGS += $(PRINTF_LIB) $(SCANF_LIB) $(MATH_LIB)




# Programming support using avrdude. Settings and variables.

# Programming hardware: alf avr910 avrisp bascom bsd
# dt006 pavr picoweb pony-stk200 sp12 stk200 stk500
#
# Type: avrdude -c ?
# to get a full listing.
#
ifndef AVRDUDE_PROGRAMMER
  AVRDUDE_PROGRAMMER = stk500v2
endif

# com1 = serial port. Use lpt1 to connect to parallel port.
ifndef AVRDUDE_PORT
  AVRDUDE_PORT = /dev/ttyUSB0    # programmer connected to serial device
endif

AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex:i
#AVRDUDE_WRITE_EEPROM = -U eeprom:w:$(TARGET).eep


# Uncomment the following if you want avrdude's erase cycle counter.
# Note that this counter needs to be initialized first using -Yn,
# see avrdude manual.
#AVRDUDE_ERASE_COUNTER = -y

# Uncomment the following if you do /not/ wish a verification to be
# performed after programming the device.
#AVRDUDE_NO_VERIFY = -V

# Increase verbosity level.  Please use this when submitting bug
# reports about avrdude. See <http://savannah.nongnu.org/projects/avrdude>
# to submit bug reports.
#AVRDUDE_VERBOSE = -v -v

AVRDUDE_FLAGS = -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)
AVRDUDE_FLAGS += $(AVRDUDE_NO_VERIFY)
AVRDUDE_FLAGS += $(AVRDUDE_VERBOSE)
AVRDUDE_FLAGS += $(AVRDUDE_ERASE_COUNTER)
AVRDUDE_FLAGS += -D -b 57600



# ---------------------------------------------------------------------------

# Define directories, if needed.
DIRAVR = c:/winavr
DIRAVRBIN = $(DIRAVR)/bin
DIRAVRUTILS = $(DIRAVR)/utils/bin
DIRINC = .
DIRLIB = $(DIRAVR)/avr/lib


# Define programs and commands.
SHELL = sh
CC = avr-gcc
AR = avr-ar
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm
AVRDUDE = avrdude
REMOVE = rm -f
REMOVE_REC = rm -f -r
COPY = cp

# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = -------- begin --------
MSG_END = --------  end  --------
MSG_SIZE_BEFORE = Size before:
MSG_SIZE_AFTER = Size after:
MSG_COFF = Converting to AVR COFF:
MSG_EXTENDED_COFF = Converting to AVR Extended COFF:
MSG_FLASH = Creating load file for Flash:
MSG_EEPROM = Creating load file for EEPROM:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = Linking:
MSG_ARCHIVING = Archiving:
MSG_COMPILING = Compiling:
MSG_ASSEMBLING = Assembling:
MSG_CLEANING = Cleaning project:

MSG_ARTX = \\033[1;36m
MSG_USER = \\033[1;32m
MSG_RESET = \\033[0m




# Define all object files.
OBJ = $(SRC:%.c=.build/%.o)

# Define all listing files.
LST = $(OBJ:%.o=%.lst)


# Compiler flags to generate dependency files.
### GENDEPFLAGS = -Wp,-M,-MP,-MT,$(*F).o,-MF,.dep/$(@F).d
GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d

# Combine all necessary flags and optional flags.
# Add target processor to flags.
CFG_CFLAGS = -mmcu=$(MCU) $(CFLAGS) $(GENDEPFLAGS)
ALL_CFLAGS = $(CFG_CFLAGS) $(WARNFLAGS)
ALL_ASFLAGS = -mmcu=$(MCU) -x assembler-with-cpp $(ASFLAGS)





# Default target.
all: begin gccversion sizebefore build sizeafter finished end

build: elf hex eep lss sym

elf: $(TARGET).elf
hex: $(TARGET).hex
eep: $(TARGET).eep
lss: $(TARGET).lss
sym: $(TARGET).sym



# Eye candy.
# AVR Studio 3.x does not check make's exit code but relies on
# the following magic strings to be generated by the compile job.
begin:
	@echo
	@echo $(MSG_BEGIN)

finished:
	@echo $(MSG_ERRORS_NONE)

end:
	@echo $(MSG_END)
	@echo


# Display size of file.
HEXSIZE = $(SIZE) --target=$(FORMAT) $(TARGET).hex
ELFSIZE = $(SIZE) -A $(TARGET).elf

sizebefore:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); echo; fi

sizeafter:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); echo; fi



# Display compiler version information.
gccversion:
	@$(CC) --version



# Program the device.
program: $(TARGET).hex $(TARGET).eep
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH) $(AVRDUDE_WRITE_EEPROM)




# Convert ELF to COFF for use in debugging / simulating in AVR Studio or VMLAB.
COFFCONVERT=$(OBJCOPY) --debugging \
--change-section-address .data-0x800000 \
--change-section-address .bss-0x800000 \
--change-section-address .noinit-0x800000 \
--change-section-address .eeprom-0x810000


coff: $(TARGET).elf
	$(NEWLINE)
	@echo $(MSG_COFF) $(TARGET).cof
	$(COFFCONVERT) -O coff-avr $< $(TARGET).cof


extcoff: $(TARGET).elf
	$(NEWLINE)
	@echo $(MSG_EXTENDED_COFF) $(TARGET).cof
	$(COFFCONVERT) -O coff-ext-avr $< $(TARGET).cof



# Create final output files (.hex, .eep) from ELF output file.
%.hex: %.elf
	$(NEWLINE)
	@echo -e "$(MSG_USER)$(MSG_FLASH) $@$(MSG_RESET)"
	$(ECHO) $(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

%.eep: %.elf
	$(NEWLINE)
	@echo $(MSG_EEPROM) $@
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# Create extended listing file from ELF output file.
%.lss: %.elf
	@echo
	@echo $(MSG_EXTENDED_LISTING) $@
	$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo
	@echo $(MSG_SYMBOL_TABLE) $@
	$(NM) -n $< > $@



# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(OBJ) $(ARTX_OBJ)
%.elf: $(OBJ) $(ARTX_OBJ)
	$(NEWLINE)
	@echo -e "$(MSG_USER)$(MSG_LINKING) $@$(MSG_RESET)"
	$(ECHO) $(CC) $(ALL_CFLAGS) $(OBJ) $(ARTX_OBJ) --output $@ $(LDFLAGS)

# Compile: create object files from C source files.
# TODO: handle (sub)directories
.build/%.o : %.c
	$(NEWLINE)
	@echo -e "$(MSG_USER)$(MSG_COMPILING) $< [USER]$(MSG_RESET)"
	@mkdir -p .build >/dev/null
	$(ECHO) $(CC) -c $(ALL_CFLAGS) $< -o $@

.build/artx/src/%.o : $(ARTX_ROOT)/src/%.c
	$(NEWLINE)
	@echo -e "$(MSG_ARTX)$(MSG_COMPILING) $(subst $(ARTX_ROOT)/,,$<) [ARTX]$(MSG_RESET)"
	@mkdir -p .build/artx/src >/dev/null
	$(ECHO) $(CC) -c $(ALL_CFLAGS) $< -o $@


# Compile: create assembler files from C source files.
%.s : %.c
	$(CC) -S $(ALL_CFLAGS) $< -o $@


# Compile: create preprocessed files from C source files.
%.i : %.c
	$(CC) -E $(ALL_CFLAGS) $< -o $@


# Assemble: create object files from assembler source files.
%.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(ALL_ASFLAGS) $< -o $@



# Target: clean project.
clean: begin clean_list finished end

realclean: begin clean_list realclean_list finished end

distclean: realclean

clean_list :
	@echo
	@echo $(MSG_CLEANING)
	$(REMOVE) $(ARTX_OBJ)
	$(REMOVE) $(ARTX_LST)
	$(REMOVE) $(TARGET).hex
	$(REMOVE) $(TARGET).eep
	$(REMOVE) $(TARGET).obj
	$(REMOVE) $(TARGET).cof
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(TARGET).obj
	$(REMOVE) $(TARGET).a90
	$(REMOVE) $(TARGET).sym
	$(REMOVE) $(TARGET).lnk
	$(REMOVE) $(TARGET).lss
	$(REMOVE) $(OBJ)
	$(REMOVE) $(LST)
	$(REMOVE) $(SRC:.c=.s)
	$(REMOVE) $(SRC:.c=.d)
	$(REMOVE) .dep/*

realclean_list :
	$(REMOVE_REC) doc
	$(REMOVE_REC) .dep
	$(REMOVE_REC) .build

debug:
	avr-gdbtui -x gdbinit

gccconfig:
	@ccconfig -c avr-gcc -f require >tools/avr-gcc-config.pl -- $(CFG_CFLAGS)

# Include the dependency files.
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)


# Listing of phony targets.
.PHONY : all begin finish end sizebefore sizeafter gccversion \
build elf hex eep lss sym coff extcoff \
clean clean_list program
