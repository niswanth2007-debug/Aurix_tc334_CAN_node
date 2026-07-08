#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"
#include "Ifx_Cfg_Ssw.h"
#include "Can/Can/IfxCan_Can.h"
#include "Can/Std/IfxCan.h"
#include "IfxPort.h"
#include "Keypad.h"

IFX_ALIGN(4) IfxCpu_syncEvent cpuSyncEvent = 0;

IfxCan_Can        can;
IfxCan_Can_Node   canNode;
IfxCan_Message    txMsg;

void LED_init(void)
{
    IfxPort_setPinModeOutput(&MODULE_P00, 5, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(&MODULE_P00, 6, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinHigh(&MODULE_P00, 5);
    IfxPort_setPinHigh(&MODULE_P00, 6);
}

void CAN_init(void)
{
    IfxCan_Can_Config     canConfig;
    IfxCan_Can_NodeConfig nodeConfig;
    IfxCan_Can_Pins       pins;

    /* Wake up CAN transceiver - drive CAN_STB (P20.9) LOW */
    IfxPort_setPinModeOutput(&MODULE_P20, 9, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(&MODULE_P20, 9);

    /* CAN module init */
    IfxCan_Can_initModuleConfig(&canConfig, &MODULE_CAN0);
    IfxCan_Can_initModule(&can, &canConfig);

    /* CAN node init with defaults first */
    IfxCan_Can_initNodeConfig(&nodeConfig, &can);

    nodeConfig.nodeId                            = IfxCan_NodeId_0;
    nodeConfig.clockSource                       = IfxCan_ClockSource_both;
    nodeConfig.frame.type                        = IfxCan_FrameType_transmit;
    nodeConfig.frame.mode                        = IfxCan_FrameMode_standard;

    /* Baudrate */
    nodeConfig.baudRate.baudrate                 = 500000;
    nodeConfig.baudRate.samplePoint              = 8000;
    nodeConfig.baudRate.syncJumpWidth            = 3;
    nodeConfig.baudRate.prescaler                = 1;
    nodeConfig.baudRate.timeSegment1             = 31;
    nodeConfig.baudRate.timeSegment2             = 8;

    /* TX buffer - 1 dedicated buffer */
    nodeConfig.txConfig.dedicatedTxBuffersNumber = 1;
    nodeConfig.txConfig.txBufferDataFieldSize    = IfxCan_DataFieldSize_8;

    /* MessageRAM addresses */
    nodeConfig.messageRAM.baseAddress            = (uint32)&MODULE_CAN0;
    nodeConfig.messageRAM.txBuffersStartAddress  = 0x400;

    /* Pins */
    pins.txPin     = &IfxCan_TXD00_P20_8_OUT;
    pins.txPinMode = IfxPort_OutputMode_pushPull;
    pins.rxPin     = &IfxCan_RXD00B_P20_7_IN;
    pins.rxPinMode = IfxPort_InputMode_pullUp;
    pins.padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed2;
    nodeConfig.pins = &pins;

    IfxCan_Can_initNode(&canNode, &nodeConfig);

    /* TX message init */
    IfxCan_Can_initMessage(&txMsg);
    txMsg.bufferNumber   = 0;
    txMsg.messageId      = 0x123;
    txMsg.dataLengthCode = IfxCan_DataLengthCode_8;
    txMsg.frameMode      = IfxCan_FrameMode_standard;
}

void core0_main(void)
{
    uint32 i, j;
    uint32 txData[2];
    uint8 keyPressed;

    IfxCpu_enableInterrupts();
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());

    IfxCpu_emitEvent(&cpuSyncEvent);
    IfxCpu_waitEvent(&cpuSyncEvent, 1);

    LED_init();

    /* Both LEDs ON briefly to confirm code reached here */
    IfxPort_setPinLow(&MODULE_P00, 5);
    IfxPort_setPinLow(&MODULE_P00, 6);
    for(i = 0; i < 200; i++)
        for(j = 0; j < 100000; j++);

    /* Both LEDs OFF */
    IfxPort_setPinHigh(&MODULE_P00, 5);
    IfxPort_setPinHigh(&MODULE_P00, 6);

    CAN_init();
    Keypad_init();

    while(1)
    {
        keyPressed = Keypad_scan();

        if(keyPressed != KEY_NONE)
        {
            /* Send ASCII value of key over CAN */
            txData[0] = '0' + keyPressed;
            txData[1] = 0x00000000;

            /* Toggle LED1 - heartbeat */
            IfxPort_togglePin(&MODULE_P00, 5);

            /* Send CAN - timeout after 1000 tries */
            i = 0;
            while((IfxCan_Can_sendMessage(&canNode, &txMsg, txData)
                   == IfxCan_Status_notSentBusy) && (i < 1000))
            {
                i++;
            }

            /* Toggle LED2 - TX attempted */
            IfxPort_togglePin(&MODULE_P00, 6);

            /* Debounce ~500ms */
            for(i = 0; i < 500; i++)
                for(j = 0; j < 100000; j++);
        }
    }
}
