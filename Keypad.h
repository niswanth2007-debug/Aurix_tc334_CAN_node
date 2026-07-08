#ifndef KEYPAD_H_
#define KEYPAD_H_

#include "Ifx_Types.h"
#include "IfxPort.h"

/* Pin definitions */
#define ROW0    &MODULE_P02, 0
#define ROW1    &MODULE_P02, 1
#define COL0    &MODULE_P02, 2
#define COL1    &MODULE_P02, 3

/* Key values */
#define KEY_NONE   0xFF
#define KEY_1      1
#define KEY_2      2
#define KEY_4      4
#define KEY_5      5

void Keypad_init(void);
uint8 Keypad_scan(void);

#endif /* KEYPAD_H_ */
