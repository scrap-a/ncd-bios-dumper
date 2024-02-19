# Copyright (c) 2015-2020 Damien Ciabrini
# This file is part of ngdevkit
#
# ngdevkit is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# ngdevkit is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with ngdevkit.  If not, see <http://www.gnu.org/licenses/>.

all: cd

include Makefile.config

# ROM names and common targets
#include ../Makefile.common

ASSETS=./assets

assets:
	$(MAKE) -C $(ASSETS) all

rom:
	mkdir $@



NCDINTV=-Wl,--defsym,rom_handler_VBlank=rom_handler_Timer_default -Wl,--defsym,rom_handler_Timer=rom_handler_VBlank_default

PRJ=DUMPER
MROM=rom/$(PRJ).Z80
SROM=rom/$(PRJ).FIX
PROM=rom/$(PRJ).PRG
SAVE=rom/SAVE.PRG
ABS=rom/ABS.TXT
BIB=rom/BIB.TXT
CPY=rom/CPY.TXT
IPL=rom/IPL.TXT

cd: $(PROM) $(SROM) $(MROM) $(SAVE) $(ABS) $(BIB) $(CPY) $(IPL) | rom

OBJS=main
ELF=rom.elf
Z80_NULLSOUND_OBJS=nullsound
Z80_USER_OBJS=user_commands
HEX=driver.ihx
NULLSOUNDSRCDIR=nullsound
FIX_ASSETS=$(ASSETS)/rom/s1-shadow.bin

$(FIX_ASSETS):
	$(MAKE) -C $(ASSETS)

# MROM: Build a custom nullsound driver for the example
# it consists is the nullsound template, plus specific
# functions to trigger the configured actions (play samples)
#
$(HEX):	nullsound.rel user_commands.rel
	$(Z80SDLD) -i $@ -u $^

user_commands.rel: user_commands.s
	$(Z80SDAS) -g -l -p -u -I$(NULLSOUNDSRCDIR) -o $@ $<

nullsound.rel: $(NULLSOUNDSRCDIR)/nullsound.s
	$(Z80SDAS) -g -l -p -u -I$(NULLSOUNDSRCDIR) -o $@ $<

$(MROM): $(HEX) | rom
	$(Z80SDOBJCOPY) -I ihex -O binary $(HEX) $@

# fixed tile ROM: fonts from common assets
$(SROM): $(FIX_ASSETS) | rom
	cp $(FIX_ASSETS) $@

# program ROM
$(ELF):	$(OBJS:%=%.o)
	$(M68KGCC) -o $@ $^ $(NCDINTV) $(NGLIBS)

%.o: %.c
	$(M68KGCC) $(NGCFLAGS) -std=gnu99 -fomit-frame-pointer -g -c $< -o $@

$(PROM): $(ELF) | rom
	#$(M68KOBJCOPY) -O binary -S -R .comment $< $@
	$(M68KOBJCOPY) -O binary -S -R .comment $< tmp.prg
	# Replace with vector for CD
	dd if=tmp.prg of=$@ bs=1 count=48
	cat cdReserved.bin >> $@
	dd if=tmp.prg of=$@ bs=1 skip=100 conv=notrunc oflag=append

# SAVE
$(SAVE):save/save.bin | rom
	cp save/save.bin $@

# A/B/C File
$(ABS):txt/ABS.txt | rom
	cp txt/ABS.txt $@
$(BIB):txt/BIB.txt | rom
	cp txt/BIB.txt $@
$(CPY):txt/CPY.txt | rom
	cp txt/CPY.txt $@

$(IPL): $(shell ls | grep $(IPL)) | rom
	echo -en $(PRJ).FIX,0,0'\r\n' >> $@
	echo -en $(PRJ).PRG,0,0'\r\n' >> $@
	echo -en SAVE.PRG,0,FC000'\r\n' >> $@
	echo -en $(PRJ).Z80,0,0'\r\n' >> $@
	echo -en '\x1a' >> $@

clean:
	rm -rf samples *.gif *.png *.pal *.adpcma *.o *~ $(ELF) *.lst *.sym *.rel *.ihx *.rst tmp.* rom

.PHONY: clean
