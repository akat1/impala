#ifndef __MACHINE_DESCRIPTOR_H
#define __MACHINE_DESCRIPTOR_H

typedef struct segmnt_descr segmnt_descr;
struct segment_descr {
    uint16_t    limit_low;
    uint16_t    base_low;
    uint8_t     base_mid;
    uint8_t     access;
    uint8_t     attr;
    uint8_t     base_high;
} __packed;

typedef struct gate_descr gate_descr;
struct gate_descr {
    uint16_t    offset_low;
    uint16_t    selector;
    uint8_t     notused;
    uint8_t     access;
    uint16_t    offset_high;
} __packed;

typedef union descriptor descriptor;
union descriptor {
    segment_descr   sdescr;
    gate_descr      gdescr;
};



#endif
