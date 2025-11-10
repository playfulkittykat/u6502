#ifndef __u6502_h
#define __u6502_h

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct _U6502 U6502;
    typedef struct _U6502_Registers U6502_Registers;
    typedef struct _U6502_Callbacks U6502_Callbacks;

    typedef uint8_t (*U6502_ReadCallback)(U6502* mpu, uint16_t address);
    typedef void (*U6502_WriteCallback)(U6502* mpu,
                                        uint16_t address,
                                        uint8_t data);
    typedef uint16_t (*U6502_CallCallback)(U6502* mpu,
                                           uint16_t address,
                                           uint16_t source);

    enum
    {
        U6502_NMIVector = 0xfffa,
        U6502_NMIVectorLSB = 0xfffa,
        U6502_NMIVectorMSB = 0xfffb,
        U6502_RSTVector = 0xfffc,
        U6502_RSTVectorLSB = 0xfffc,
        U6502_RSTVectorMSB = 0xfffd,
        U6502_IRQVector = 0xfffe,
        U6502_IRQVectorLSB = 0xfffe,
        U6502_IRQVectorMSB = 0xffff
    };

    enum _U6502_Status
    {
        U6502_StatusOk = 0,
        U6502_StatusStack,
        U6502_StatusIll,
    };

    typedef enum _U6502_Status U6502_Status;

    struct _U6502_Registers
    {
        uint8_t a;   /* accumulator */
        uint8_t x;   /* X index register */
        uint8_t y;   /* Y index register */
        uint8_t p;   /* processor status register */
        uint8_t s;   /* stack pointer */
        uint16_t pc; /* program counter */
    };

    struct _U6502_Callbacks
    {
        U6502_ReadCallback read;
        U6502_WriteCallback write;
        U6502_CallCallback call;
    };

    struct _U6502
    {
        U6502_Registers* registers;
        U6502_Callbacks callbacks;
        unsigned int flags;
        unsigned char ticks;
        void* userdata;
    };

    enum
    {
        U6502_RegistersAllocated = 1 << 0,
    };

    extern U6502* U6502_new(U6502_Registers* registers,
                            U6502_Callbacks callbacks,
                            void* userdata);
    extern void U6502_reset(U6502* mpu);
    extern void U6502_nmi(U6502* mpu);
    extern void U6502_irq(U6502* mpu);
    extern U6502_Status U6502_run(U6502* mpu);
    extern U6502_Status U6502_tick(U6502* mpu);
    extern U6502_Status U6502_step(U6502* mpu);
    extern int U6502_disassemble(U6502* mpu, uint16_t addr, char buffer[64]);
    extern void U6502_dump(U6502* mpu, char buffer[64]);
    extern void U6502_delete(U6502* mpu);

#define U6502_getVector(MPU, VEC)                                              \
    ((((MPU)->callbacks.read((MPU), U6502_##VEC##VectorLSB))) |                \
     ((MPU)->callbacks.read((MPU), U6502_##VEC##VectorMSB) << 8))

#define U6502_setVector(MPU, VEC, ADDR)                                        \
    ((((MPU)->callbacks.write(                                                 \
       (MPU), U6502_##VEC##VectorLSB, ((uint8_t)(ADDR)) & 0xff))),             \
     ((MPU)->callbacks.write(                                                  \
       (MPU), U6502_##VEC##VectorMSB, (uint8_t)((ADDR) >> 8))))

#define U6502_getCallback(MPU, TYPE) ((MPU)->callbacks.TYPE)
#define U6502_setCallback(MPU, TYPE, FN) ((MPU)->callbacks.TYPE = (FN))

#ifdef __cplusplus
}
#endif

#endif //__u6502_h
