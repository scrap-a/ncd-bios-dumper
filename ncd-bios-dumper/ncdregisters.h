#ifndef __NCD_REGISTERS_H__
#define __NCD_REGISTERS_H__

/* NCD memory mapped registers */
/* Information is from The NeoGeo Development Wiki.*/
/* https://wiki.neogeodev.org/index.php?title=Memory_mapped_registers#NeoGeo_CD_registers */

/* CD drive*/
#define REG_CDDINPUT  ((volatile u8*)0xFF0161)
#define REG_CDDOUTPUT ((volatile u8*)0xFF0163)
#define REG_CDDCTRL   ((volatile u8*)0xFF0165)
#define REG_CDDSTAT   ((volatile u8*)0xFF0167)

/* Memory */
#define REG_SPRBANK    ((volatile u8*)0xFF01A1)
#define REG_PCMBANK    ((volatile u8*)0xFF01A3)
#define REG____BANK    ((volatile u8*)0xFF01A7)
#define REG_UPMAPSPR   ((volatile u8*)0xFF0121)
#define REG_UPMAPPCM   ((volatile u8*)0xFF0123)
#define REG_UPMAPZ80   ((volatile u8*)0xFF0127)
#define REG_UPMAPFIX   ((volatile u8*)0xFF0129)
#define REG_UPUNMAPSPR ((volatile u8*)0xFF0141)
#define REG_UPUNMAPPCM ((volatile u8*)0xFF0143)
#define REG_UPUNMAPZ80 ((volatile u8*)0xFF0147)
#define REG_UPUNMAPFIX ((volatile u8*)0xFF0149)

/* DMA */
#define REG_DMA_ADDR1 ((volatile u32*)0xFF0064)
#define REG_DMA_ADDR2 ((volatile u32*)0xFF0068)
#define REG_DMA_VALUE ((volatile u32*)0xFF006C)
#define REG_DMA_COUNT ((volatile u32*)0xFF0070)
#define REG_DMA_MODE  ((volatile  u8*)0xFF007E)

/* Misc */
#define REG_TRANSAREA ((volatile  u8*)0xFF0105)
#define REG_CDCONFIG  ((volatile u16*)0xFF011C)
#define REG_UPLOAD_EN ((volatile  u8*)0xFF016F)
#define REG_CDIRQ_EN  ((volatile  u8*)0xFF0181)
#define REG_Z80RST    ((volatile  u8*)0xFF0183)

/* CDDA */
#define REG_CDDALEFTL  ((volatile u16*)0xFF0188)
#define REG_CDDARIGHTL ((volatile u16*)0xFF018A)


#endif /* __NCD_REGISTERS_H__ */
