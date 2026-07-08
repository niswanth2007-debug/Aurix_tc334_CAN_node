#include "Keypad.h"

void Keypad_init(void)
{
    IfxPort_setPinModeOutput(ROW0, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(ROW1, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinMode(COL0, IfxPort_Mode_inputPullUp);
    IfxPort_setPinMode(COL1, IfxPort_Mode_inputPullUp);
    IfxPort_setPinHigh(ROW0);
    IfxPort_setPinHigh(ROW1);
}

uint8 Keypad_scan(void)
{
    uint8 key = KEY_NONE;
    volatile uint32 d;

    /* Scan ROW0 */
    IfxPort_setPinLow(ROW0);
    IfxPort_setPinHigh(ROW1);
    for(d = 0; d < 50000; d++);

    if(IfxPort_getPinState(COL0) == FALSE)
        key = KEY_1;
    else if(IfxPort_getPinState(COL1) == FALSE)
        key = KEY_2;

    IfxPort_setPinHigh(ROW0);
    for(d = 0; d < 50000; d++);

    /* Scan ROW1 only if no key found */
    if(key == KEY_NONE)
    {
        IfxPort_setPinLow(ROW1);
        for(d = 0; d < 50000; d++);

        if(IfxPort_getPinState(COL0) == FALSE)
            key = KEY_4;
        else if(IfxPort_getPinState(COL1) == FALSE)
            key = KEY_5;

        IfxPort_setPinHigh(ROW1);
    }
    else
    {
        IfxPort_setPinHigh(ROW1);
    }

    return key;
}
