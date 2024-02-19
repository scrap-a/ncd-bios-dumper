/*
 * Copyright (c) 2020 Damien Ciabrini
 * This file is part of ngdevkit-examples
 *
 * ngdevkit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * ngdevkit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ngdevkit.  If not, see <http://www.gnu.org/licenses/>.
 */

// Additional resources for sound
// https://wiki.neogeodev.org/index.php?title=YM2610_registers

#include <ngdevkit/neogeo.h>
#include <ngdevkit/bios-ram.h>
#include <ngdevkit/ng-fix.h>
#include <ngdevkit/ng-video.h>
#include <stdio.h>
#include <stdlib.h>
#include "ncdbios-calls.h"
#include "ncdbios-ram.h"
// #include "ncdregisters.h"
#include "crc_table.h"

/// controller's current state, and change on button press (not release)
extern u8 bios_p1current;
extern u8 bios_p1change;
#define A_PRESSED 0x10
#define B_PRESSED 0x20
#define C_PRESSED 0x40

#define TOP 2
#define LEFT 1

/// top of BIOS address
#define BIOS ((u8*)(0xC00000))

/// top of SAVE address
#define SAVE ((u8*)(0x800000))

// top of SAVE FILE address
#define SAVE_FILE ((u8*)(0xFC000))

#define BIOS_SIZE 0x80000
#define SAVE_SIZE 0x4000

void memory_view(u8* base_add, int endian);

const u16 clut[][16]= {
    /// first 16 colors palette for the fix tiles
    {0x8000, 0x0fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x8000, 0x0fff, 0x0a40, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x8000, 0x0fff, 0x004a, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}
};

struct RUN_LENGTH_LIST
{
    u32  address;
    u32  length;
    u8   value;
};

void init_palette() {
    /// Initialize the two palettes in the first palette bank
    u16 *p=(u16*)clut;
    for (u16 i=0;i<sizeof(clut)/2; i++) {
        MMAP_PALBANK1[i]=p[i];
    }
    *((volatile u16*)0x401ffe)=0x0333;
}

/// Clear under lines of fix map
void ng_cls_under(int line) {
    u16 val = SROM_EMPTY_TILE;
    *REG_VRAMADDR = ADDR_FIXMAP;
    *REG_VRAMMOD = 1;
    for(u16 i=0; i<40;i++){
        for (u16 j=0; j<line; j++) *REG_VRAMRW = *REG_VRAMRW;
        for (u16 j=line; j<32; j++) *REG_VRAMRW = val;
    }
}


void i2a(unsigned char in, char *out){
    *(out+0) = ((in>>4 & 0x0F)>9) ? (in>>4 & 0x0F)+0x37 : (in>>4 & 0x0F)+0x30;
    *(out+1) = ((in    & 0x0F)>9) ? (in    & 0x0F)+0x37 : (in    & 0x0F)+0x30;
}

void (*BIOSF_UPLOAD_CALL)();

void upload_pcm(u32 src, u32 dest, u32 size, u8 bank){

    // *REG_UPMAPPCM = 1;
    // *REG_PCMBANK = 1;

    *BIOS_UPSRC = src;
    *BIOS_UPDEST = dest;
    *BIOS_UPSIZE = size;
    *BIOS_UPZONE = 0x04;
    *BIOS_UPBANK = bank;

    BIOSF_UPLOAD_CALL();

    // *REG_UPUNMAPPCM = 1;

    return;
}

void init_bioscall(){
    BIOSF_UPLOAD_CALL = (void*)BIOSF_UPLOAD;

}

int scan_run_length(struct RUN_LENGTH_LIST* rllist, u32 address, u32 size){
    int ret = 0;
    int i;
    u8* pdata;
    u8 data=0, data_pre=0;
    u32 cnt=0;
    int rllist_cnt=0;

    for(i=0; i<size; i++){
        pdata = (u8*)(address+i);
        data = *pdata;

        
        if(data == data_pre){
            cnt++;
        }else{
            if(cnt>=(u32)64){
                rllist[rllist_cnt].address = (u32)(i-cnt);
                rllist[rllist_cnt].length = cnt;
                rllist[rllist_cnt].value = data_pre;
                rllist_cnt++;
            }
            cnt=0;
        }
        data_pre = data;
    }
    if(cnt>=16){
        rllist[rllist_cnt].address = (u32)(size-cnt);
        rllist[rllist_cnt].length = cnt;
        rllist[rllist_cnt].value = data_pre;
        rllist_cnt++;
    }
    ret = rllist_cnt;

    return ret;
}

void am_encode(u8* address, u32 length, int quant_bit, int ch, int endian, int stripe){
    u8 input, code;
    u8 *buf;
    u32 i,j, rcnt=0, wcnt=0;
    int flag=1;
    char rcnt_str[15] = {'a','d','d','r','e','s','s',':',0,0,0,0,0,0,0};
    // int rlcnt=0, rlnum=0;
    int buf_flag=0;
    u32 crc=0xFFFFFFFF;
    char crc_str[] = {'C', 'R', 'C', ':', 0,0,0,0,0,0,0,0,0};
    int endian_flag=endian;
    int stripe_flag=1;
    
    // struct RUN_LENGTH_LIST rllist[128];
#define BUF_SIZE 16384
#define BANK_SIZE 0x80000

    if(ch!=1){
        return;
    }

    if(stripe<0 || stripe>2){
        return;
    }
    rcnt+=(stripe&0b1);
    if(stripe){
        stripe_flag++;
    }

    ng_cls_under(23);
    ng_center_text_tall(26, 0, "NOW PREPARING");

    buf = (u8*)malloc(sizeof(u8)*BUF_SIZE*2);
    for(i=0; i<BUF_SIZE*2; i++){
        buf[i] = 0x80;
    }

    // rlnum = scan_run_length(rllist, (u32)address, length);

    u8 pilot_signal[8] = {0x07, 0xF7, 0xF7, 0xF4, 0x4C, 0x4C, 0xC4, 0xC4};
    for(i=0; i<8; i++){
        buf[i] = pilot_signal[i];
        buf[i+BUF_SIZE] = pilot_signal[i];
    }
    wcnt+=8;

    for(i=0; i<length; i++){

        input = *(address+rcnt+endian_flag*stripe_flag);
        if(endian_flag==(!endian)){
            rcnt+=2*stripe_flag;
        }
        endian_flag = !endian_flag;

        crc = crc_table[(crc ^ input) & 0xFF] ^ (crc >> 8);

        if(quant_bit==2){
            code = ((input >> 6)&0x3);
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] = (code | 0x4)<<4;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] |= code | 0xC;
            wcnt++;

            code = ((input >> 4)&0x3);
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] = (code | 0xC)<<4;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] |= code | 0x4;
            wcnt++;

            code = ((input >> 2)&0x3);
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] = (code | 0x4)<<4;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] |= code | 0xC;
            wcnt++;

            code = ((input >> 0)&0x3);
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] = (code | 0xC)<<4;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] |= code | 0x4;
            wcnt++;
        }else if(quant_bit==1){
            code = ((input >> 7)&0x1);
            code = (code<<1) + code;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] = (code | 0x4)<<4;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] |= code | 0xC;
            wcnt++;

            code = ((input >> 6)&0x1);
            code = (code<<1) + code;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] = (code | 0xC)<<4;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] |= code | 0x4;
            wcnt++;

            code = ((input >> 5)&0x1);
            code = (code<<1) + code;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] = (code | 0x4)<<4;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] |= code | 0xC;
            wcnt++;

            code = ((input >> 4)&0x1);
            code = (code<<1) + code;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] = (code | 0xC)<<4;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] |= code | 0x4;
            wcnt++;

            code = ((input >> 3)&0x1);
            code = (code<<1) + code;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] = (code | 0x4)<<4;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] |= code | 0xC;
            wcnt++;

            code = ((input >> 2)&0x1);
            code = (code<<1) + code;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] = (code | 0xC)<<4;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] |= code | 0x4;
            wcnt++;

            code = ((input >> 1)&0x1);
            code = (code<<1) + code;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] = (code | 0x4)<<4;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] |= code | 0xC;
            wcnt++;

            code = ((input >> 0)&0x1);
            code = (code<<1) + code;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] = (code | 0xC)<<4;
            buf[wcnt%BUF_SIZE+buf_flag*BUF_SIZE] |= code | 0x4;
            wcnt++;
        }

        if((wcnt+8) % BUF_SIZE == 0){
            wcnt+=8;

            i2a( (rcnt>>16)&0xFF, &rcnt_str[8]);
            i2a( (rcnt>> 8)&0xFF, &rcnt_str[10]);
            i2a( (rcnt>> 0)&0xFF, &rcnt_str[12]);
            ng_center_text(23, 0, rcnt_str);

            // memory_view((u8*)buf, 0);

            while(flag==0){
                if(*REG_SOUND>>7 & 0x01){
                    break;
                }
            }
            upload_pcm((u32)&buf[buf_flag*BUF_SIZE], (u32)(((wcnt-BUF_SIZE) % BANK_SIZE)*0), (u32)(BUF_SIZE), buf_flag);
            
            for(j=0; j<6;j++){
                ng_wait_vblank();
            }

            if(flag==1){
                ng_cls_under(26);
                ng_center_text_tall(26, 0, "NOW PLAYING");
                *REG_SOUND=5+buf_flag*4;
                flag=0;
            }
            else {
                *REG_SOUND=5+buf_flag*4;
            }
            buf_flag = !buf_flag;
            wcnt+=8;
        }
        if(rcnt >= length){
            break;
        }

    }
    if(wcnt % BUF_SIZE !=0){
        for(i=wcnt%BUF_SIZE; i<BUF_SIZE; i++){
            buf[i+buf_flag*BUF_SIZE] = 0x80;
        }
        while(1){
            if(*REG_SOUND>>7 & 0x01){
                break;
            }
        }
        upload_pcm((u32)&buf[buf_flag*BUF_SIZE], (u32)(((wcnt - (wcnt % BUF_SIZE) ) % BANK_SIZE)*0), (u32)(BUF_SIZE), buf_flag);
    }

    free(buf);
    for(j=0; j<6;j++){
        ng_wait_vblank();
    }
    *REG_SOUND=5+buf_flag*4;

    if(rcnt > length){
        rcnt = length;
    }

    i2a( (rcnt>>16)&0xFF, &rcnt_str[8]);
    i2a( (rcnt>> 8)&0xFF, &rcnt_str[10]);
    i2a( (rcnt>> 0)&0xFF, &rcnt_str[12]);
    ng_center_text(23, 0, rcnt_str);

    crc = ~crc;
    i2a( (crc>>24)&0xFF, &crc_str[4]);
    i2a( (crc>>16)&0xFF, &crc_str[6]);
    i2a( (crc>> 8)&0xFF, &crc_str[8]);
    i2a( (crc>> 0)&0xFF, &crc_str[10]);
    ng_center_text(24, 0, crc_str);

    while(1){
        if(*REG_SOUND>>7 & 0x01){
            ng_cls_under(26);
            ng_center_text_tall(26, 0, "FINISHED");
            break;
        }
    }
    while(1){}
}

void bios_dump(int method){

    switch(method){
        case 0:     //am_mono_low(1bit per quantizaion)
            am_encode(BIOS, BIOS_SIZE, 1, 1, 1, 0);
            break;
        case 1:     //am_mono_high(2bit per quantizaion)
            am_encode(BIOS, BIOS_SIZE, 2, 1, 1, 0);
            break;
        case 2:     //am_stereo
            // am_encode(BIOS, 524288, 2);
            break;
    }
}

void save_dump(int method){

    switch(method){
        case 0:     //am_mono_low(1bit per quantizaion)
            am_encode(SAVE, SAVE_SIZE, 1, 1, 0, 1);
            break;
        case 1:     //am_mono_high(2bit per quantizaion)
            am_encode(SAVE, SAVE_SIZE, 2, 1, 0, 1);
            break;
        case 2:     //am_stereo
            // am_encode(BIOS, 524288, 2);
            break;
    }
}

void save_restore(){
    int i;
    u8 a, b, d, s;

    ng_center_text_tall(20, 2, "CAUTION");
    ng_center_text(22, 2, "Save will be overwritten");
    ng_center_text(23, 2, "If you continue, press Start+A+D");
    ng_center_text(24, 2, "If you cancel, press B");

    while(1){
        a = (bios_p1current & CNT_A);
        b = (bios_p1current & CNT_B);
        d = (bios_p1current & CNT_D);
        s = (bios_statcurnt & CNT_START1);
        if(s)
            if(d)
                if(a)
                    break;

        if(b){
            ng_cls_under(20);
            return;
        }
    }

    for(i=0; i<SAVE_SIZE/2; i++){
        *(SAVE+i*2+1) = *(SAVE_FILE+i);
    }
    ng_center_text_tall(26, 0, "FINISHED");
    while(1){}
}

// memory viewer (debug)
void memory_view(u8* base_add, int endian){
    char add_str[] = {0,0,0,0,0,':',0};
    u32 add=0;
    u8 u, d, s;
    if(endian != 0){
        endian = 1;
    }

    ng_cls_under(TOP+5);
    char bin[] = {0,0,0};
    for(int j=0; j<23; j++){
        i2a( (((add+16*j)>>16)&0xF)<<4, &add_str[0]);
        i2a( ((add+16*j)>>8)&0xFF, &add_str[1]);
        i2a( (add+16*j)&0xFF, &add_str[3]);
        ng_text(LEFT, TOP+4+j, 0, add_str);
    }

    for(int j=0; j<23; j++){
        for(int i=0; i<16; i+=2){
            i2a( *(base_add+i+endian+j*16+add), bin);
            ng_text(LEFT+6 +i*2 +0, TOP+4+j, 0, bin);
            
            i2a( *(base_add+i+!endian+j*16+add), bin);
            ng_text(LEFT+6 +i*2 +2, TOP+4+j, 0, bin);
        }
    }
    while(1){
        u = (bios_p1change & CNT_UP);
        d = (bios_p1change & CNT_DOWN);
        s = (bios_statcurnt & CNT_START1);

        if(u){
            add= ( add >= 23*16) ? add-23*16 : 0;

            ng_cls_under(TOP+5);
            char bin[] = {0,0,0};

            for(int j=0; j<23; j++){
                i2a( (((add+16*j)>>16)&0xF)<<4, &add_str[0]);
                i2a( ((add+16*j)>>8)&0xFF, &add_str[1]);
                i2a( (add+16*j)&0xFF, &add_str[3]);
                ng_text(LEFT, TOP+4+j, 0, add_str);
            }

            for(int j=0; j<23; j++){
                for(int i=0; i<16; i+=2){
                    i2a( *(base_add+i+endian+j*16+add), bin);
                    ng_text(LEFT+6 +i*2 +0, TOP+4+j, 0, bin);
                    
                    i2a( *(base_add+i+!endian+j*16+add), bin);
                    ng_text(LEFT+6 +i*2 +2, TOP+4+j, 0, bin);
                }
            }            
            
            ng_wait_vblank();
        }
        if(d){
            add= ( add+23*16 <=0x80000) ? add+23*16 : 0x80000;

            ng_cls_under(TOP+5);
            char bin[] = {0,0,0};

            for(int j=0; j<23; j++){
                i2a( (((add+16*j)>>16)&0xF)<<4, &add_str[0]);
                i2a( ((add+16*j)>>8)&0xFF, &add_str[1]);
                i2a( (add+16*j)&0xFF, &add_str[3]);
                ng_text(LEFT, TOP+4+j, 0, add_str);
            }

            for(int j=0; j<23; j++){
                for(int i=0; i<16; i+=2){
                    i2a( *(base_add+i+endian+j*16+add), bin);
                    ng_text(LEFT+6 +i*2 +0, TOP+4+j, 0, bin);
                    
                    i2a( *(base_add+i+!endian+j*16+add), bin);
                    ng_text(LEFT+6 +i*2 +2, TOP+4+j, 0, bin);
                }
            }            
            
            ng_wait_vblank();
        }
        if(s){
            break;
        }

    }
}

// calc CRC32 (debug)
void calc_crc32(u8* base_add, int endian, u32 length){
    char crc_str[] = {'C', 'R', 'C', ':', 0,0,0,0,0,0,0,0,0};
    u32 crc=0xFFFFFFFF;
    u8 data;
    u8 s;

    if(endian != 0){
        endian = 1;
    }

    ng_cls_under(26);
    ng_center_text_tall(26, 0, "NOW CALCULATING");

    for(u32 i=0; i<length; i+=2){
        data = *(base_add+i+endian);
        crc = crc_table[(crc ^ data) & 0xFF] ^ (crc >> 8);
        data = *(base_add+i+!endian);
        crc = crc_table[(crc ^ data) & 0xFF] ^ (crc >> 8);
    }
    crc = ~crc;

    i2a( (crc>>24)&0xFF, &crc_str[4]);
    i2a( (crc>>16)&0xFF, &crc_str[6]);
    i2a( (crc>> 8)&0xFF, &crc_str[8]);
    i2a( (crc>> 0)&0xFF, &crc_str[10]);
    ng_cls_under(26);
    ng_center_text_tall(26, 0, crc_str);

    while(1){
        s = (bios_statcurnt & CNT_START1);
        if(s){
            break;
        }
    }
}

int main(void) {
    // Command 3: reset z80 sound driver
    *REG_SOUND = 3;
    ng_cls();
    init_palette();

    u8 l, r, u, d, a, b, c;

    ng_center_text_tall(TOP+2, 0, "NCD BIOS DUMPER");

#define MODE_NUM 3      //(BIOS dump / SAVE dump / SAVE restore)
#define METHOD_NUM 2    //encoding method num
    int mode = 0;
    int mode_array[MODE_NUM];
    int method = 0;
    int method_array[METHOD_NUM];

    for(int i=0; i<MODE_NUM; i++) mode_array[i]=0;
    mode_array[mode] = 1;
    for(int i=0; i<METHOD_NUM; i++) method_array[i]=0;
    method_array[method] = 1;

    init_bioscall();

    ng_center_text(TOP+6, 0, "mode");
    ng_center_text(TOP+12, 0, "method");

    for(;;) {
        l = (bios_p1change & CNT_LEFT);
        r = (bios_p1change & CNT_RIGHT);
        u = (bios_p1change & CNT_UP);
        d = (bios_p1change & CNT_DOWN);
        a = (bios_p1current & CNT_A);
        b = (bios_p1current & CNT_B);
        c = (bios_p1current & CNT_C);
        
        if(l){
            mode_array[mode] = 0;
            mode = (mode-1 + MODE_NUM) % MODE_NUM;
            mode_array[mode] = 1;
        }
        if(r){
            mode_array[mode] = 0;
            mode = (mode+1) % MODE_NUM;
            mode_array[mode] = 1;
        }

        if( METHOD_NUM !=1){
            if(mode==2){
                method = 0;
                for(int i=0; i<METHOD_NUM; i++) method_array[i]=0;
                method_array[method] = 1;
            }else{
                if(u){
                    method_array[method] = 0;
                    method = (method-1 + METHOD_NUM) % METHOD_NUM;
                    method_array[method] = 1;
                }
                if(d){
                    method_array[method] = 0;
                    method = (method+1) % METHOD_NUM;
                    method_array[method] = 1;
                }
            }
        }

        ng_text(LEFT+2,  TOP+8, mode_array[0], "BIOS dump");
        ng_text(LEFT+13, TOP+8, mode_array[1], "SAVE dump");
        ng_text(LEFT+24, TOP+8, mode_array[2], "SAVE restore");

        if( l | r){
            ng_cls_under(TOP+13);
        }

        switch(mode){
            case 0:
                ng_center_text(TOP+14, method_array[0], "AM(Low Speed)/mono/about 8 min");
                ng_center_text(TOP+15, method_array[1], "AM(High Speed)/mono/about 4 min");
                // ng_center_text(TOP+16, method_array[2], "AM/stereo(unimplemented)");
                // ng_center_text(TOP+16, method_array[2], "KCS/2400baud/mono (about 30minutes)");
                break;
            case 1:
                ng_center_text(TOP+14, method_array[0], "AM(Low Speed)/mono/about 10 sec");
                ng_center_text(TOP+15, method_array[1], "AM(High Speed)/mono/about 5 sec");
                // ng_center_text(TOP+16, method_array[2], "AM/stereo(unimplemented)");
                // ng_center_text(TOP+15, method_array[1], "SCS/mono (about 22minutes)");
                // ng_center_text(TOP+16, method_array[2], "KCS/2400baud/mono (about 30minutes)");
                break;
            case 2:
                ng_center_text(TOP+14, method_array[0], "SAVE.PRG->restore");
                break;
        }


        if(a){
            if(b){ //B+A -> bios header view(for debug)
                memory_view(SAVE, 1);
                break;
            }
            if(c){ //C+A -> bios crc32 calculation (for debug)
                calc_crc32(BIOS, 1, BIOS_SIZE);
                break;
            }

            switch(mode){
                case 0:
                    bios_dump(method);
                    break;
                case 1:
                    save_dump(method);
                    break;
                case 2:
                    save_restore();
                    break;
            }
        }

        ng_wait_vblank();
    }
    
    return 0;
}
