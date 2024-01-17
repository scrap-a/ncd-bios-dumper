#ifndef __NCD_BIOS_RAM_H__
#define __NCD_BIOS_RAM_H__

/* NCD BIOS Ram */
/* Information is from The NeoGeo Development Wiki.*/
/* https://wiki.neogeodev.org/index.php?title=BIOS_RAM_locations#CD_only */
#define BIOS_UPDEST ((volatile u32*)0x10FEF4)
#define BIOS_UPSRC  ((volatile u32*)0x10FEF8)
#define BIOS_UPSIZE ((volatile u32*)0x10FEFC)
#define BIOS_UPZONE ((volatile  u8*)0x10FEDA)
#define BIOS_UPBANK ((volatile  u8*)0x10FEDB)

#endif /* __NCD_BIOS_RAM_H__ */
