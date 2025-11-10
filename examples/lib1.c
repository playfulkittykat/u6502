#include <stdio.h>
#include <stdlib.h>

#include "u6502.h"

/* Emulated OS functions. */

#define WRCH 0xFFEE /* Write accumulator to stdout. */

/* Write the accumulator to stdout.  This function will be invoked
 * when the emulated program calls 0xFFEE.
 */
static uint16_t
wrch(U6502* mpu)
{
    int pc;

    /* Write the character.
     */
    putchar(mpu->registers->a);

    uint8_t* memory = (uint8_t*)mpu->userdata;

    /* We arrived here from a JSR instruction.  The stack contains the
     * saved PC.  Pop it off the stack.
     */
    pc = memory[++mpu->registers->s + 0x100];
    pc |= memory[++mpu->registers->s + 0x100] << 8;

    /* The JSR instruction pushes the value of PC before it has been
     * incremented to point to the instruction after the JSR.  Return PC
     * + 1 as the address for the next insn.  Returning non-zero
     * indicates that we handled the 'subroutine' ourselves, and the
     * emulator should pretend the original 'JSR' neveer happened at
     * all.
     */
    return pc + 1; /* JSR pushes next insn addr - 1 */
}

/* Exit gracefully.  We arrange for this function to be called when
 * the emulator tries to transfer control to address 0.
 */
static uint16_t
done(U6502* mpu)
{
    char buffer[64];

    /* Dump the internal state of the processor.
     */
    U6502_dump(mpu, buffer);

    /* Print a cute message and quit.
     */
    printf("\nBRK instruction\n%s\n", buffer);
    exit(0);
}

static uint16_t
call(U6502* mpu, uint16_t address, uint16_t source)
{
    (void)source;

    switch (address) {
        case 0:
            return done(mpu);
        case WRCH:
            return wrch(mpu);
        default:
            return 0;
    }
}

static uint8_t
read(U6502* mpu, uint16_t address)
{
    return ((uint8_t*)mpu->userdata)[address];
}

static void
write(U6502* mpu, uint16_t address, uint8_t value)
{
    ((uint8_t*)mpu->userdata)[address] = value;
}

int
main(void)
{
    uint8_t* memory = calloc(0x10000, 1);
    if (!memory) {
        abort();
    }

    /* Install the callback functions defined above.
     */
    U6502_Callbacks callbacks = {
        .call = call,
        .read = read,
        .write = write,
    };

    U6502* mpu = U6502_new(0, callbacks, &memory[0]); /* Make a 6502 */
    unsigned pc = 0x1000;                             /* PC for 'assembly' */

    /* A few macros that dump bytes into the 6502's memory.
     */
#define gen1(X) (memory[pc++] = (uint8_t)(X))
#define gen2(X, Y)                                                             \
    gen1(X);                                                                   \
    gen1(Y)
#define gen3(X, Y, Z)                                                          \
    gen1(X);                                                                   \
    gen2(Y, Z)

    /* Hand-assemble the program.
     */
    gen2(0xA2, 'A');        // LDX #'A'
    gen1(0x8A);             // TXA
    gen3(0x20, 0xEE, 0xFF); // JSR FFEE
    gen1(0xE8);             // INX
    gen2(0xE0, 'Z' + 1);    // CPX #'Z'+1
    gen2(0xD0, -9);         // BNE 0x1002
    gen2(0xA9, '\n');       // LDA #'\n'
    gen3(0x20, 0xEE, 0xFF); // JSR FFEE
    gen2(0x00, 0x00);       // BRK

    /* Just for fun: disssemble the program.
     */
    {
        char insn[64];
        uint16_t ip = 0x1000;
        while (ip < pc) {
            ip += U6502_disassemble(mpu, ip, insn);
            printf("%04X %s\n", ip, insn);
        }
    }

    /* Point the RESET vector at the first instruction in the assembled
     * program.
     */
    U6502_setVector(mpu, RST, 0x1000);

    /* Reset the 6502 and run the program.
     */
    U6502_reset(mpu);
    U6502_run(mpu);
    U6502_delete(mpu); /* We never reach here, but what the hey. */
    free(memory);

    return 0;
}
