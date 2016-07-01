/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2008. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  NALADKA_P                       1
#define  NALADKA_P_MANUAL                2       /* callback function: MANUAL_F */
#define  NALADKA_P_STOP                  3       /* callback function: STOP_F */
#define  NALADKA_P_EXIT                  4       /* callback function: ExiT_TEST_F */
#define  NALADKA_P_RUNHARD               5       /* callback function: INIT_F */
#define  NALADKA_P_BURN                  6       /* callback function: BURN_F */
#define  NALADKA_P_RUN2                  7       /* callback function: SN_F */
#define  NALADKA_P_RUN1                  8       /* callback function: RUN_CALIBR_F */
#define  NALADKA_P_TEST8                 9       /* callback function: TEST_RUN */
#define  NALADKA_P_TEST6                 10      /* callback function: TEST_RUN */
#define  NALADKA_P_GROUND                11      /* callback function: TEST_RUN */
#define  NALADKA_P_TEST5                 12      /* callback function: TEST_RUN */
#define  NALADKA_P_TEST4                 13      /* callback function: TEST_RUN */
#define  NALADKA_P_TEST31                14      /* callback function: TEST_RUN */
#define  NALADKA_P_TEST3                 15      /* callback function: TEST_RUN */
#define  NALADKA_P_TEST2                 16      /* callback function: TEST_RUN */
#define  NALADKA_P_TESTFST               17      /* callback function: TESTBURN_F */
#define  NALADKA_P_TESTSPEED             18      /* callback function: FAST_F */
#define  NALADKA_P_TESTALL               19      /* callback function: TESTALL_F */
#define  NALADKA_P_TEST0                 20      /* callback function: TEST_RUN */
#define  NALADKA_P_TEST1                 21      /* callback function: TEST_RUN */
#define  NALADKA_P_LED1_2                22
#define  NALADKA_P_LED1                  23
#define  NALADKA_P_ERROR_STOP            24      /* callback function: LOOP_F */
#define  NALADKA_P_LOOP                  25      /* callback function: LOOP_F */
#define  NALADKA_P_TEST_CHECK_9          26
#define  NALADKA_P_LED_TEST_15           27
#define  NALADKA_P_LED_CALIBR            28
#define  NALADKA_P_LED_TEST_9            29
#define  NALADKA_P_TEST_CHECK_8          30
#define  NALADKA_P_TEST_CHECK_10         31
#define  NALADKA_P_LED_TEST_16           32
#define  NALADKA_P_LED_TEST_8            33
#define  NALADKA_P_TEST_CHECK_7          34
#define  NALADKA_P_LED_TEST_7            35
#define  NALADKA_P_TEST_CHECK_6          36
#define  NALADKA_P_LED_TEST_6            37
#define  NALADKA_P_TEST_CHECK_5          38
#define  NALADKA_P_LED_TEST_5            39
#define  NALADKA_P_TEST_CHECK_4          40
#define  NALADKA_P_LED_TEST_4            41
#define  NALADKA_P_TEST_CHECK_3          42
#define  NALADKA_P_LED_TEST_3            43
#define  NALADKA_P_TEST_CHECK_2          44
#define  NALADKA_P_LED_TEST_2            45
#define  NALADKA_P_TEST_CHECK_1          46
#define  NALADKA_P_LED_TEST_11           47
#define  NALADKA_P_LED_TEST_10           48
#define  NALADKA_P_LED_TEST_1            49
#define  NALADKA_P_TEXTBOX               50
#define  NALADKA_P_GRAPH_2               51
#define  NALADKA_P_STRING                52
#define  NALADKA_P_ERRORS                53
#define  NALADKA_P_CYCLE                 54
#define  NALADKA_P_SN                    55
#define  NALADKA_P_TIMER                 56      /* callback function: TIMER_TEST_F */
#define  NALADKA_P_DECORATION            57
#define  NALADKA_P_DECORATION_2          58

#define  PANEL                           2
#define  PANEL_GETADC                    2       /* callback function: ADCHAN_F */
#define  PANEL_CHANNEL                   3       /* callback function: CHAN_F */
#define  PANEL_GAIN                      4       /* callback function: CHAN_F */
#define  PANEL_LOOP                      5       /* callback function: CHAN_F */
#define  PANEL_ADC                       6
#define  PANEL_GETADC_2                  7       /* callback function: GET_TTL_F */
#define  PANEL_TTL_16                    8       /* callback function: PUT_TTL_F */
#define  PANEL_TTL_15                    9       /* callback function: PUT_TTL_F */
#define  PANEL_TTL_14                    10      /* callback function: PUT_TTL_F */
#define  PANEL_TTL_13                    11      /* callback function: PUT_TTL_F */
#define  PANEL_TTL_12                    12      /* callback function: PUT_TTL_F */
#define  PANEL_STATUS                    13
#define  PANEL_TTL_11                    14      /* callback function: PUT_TTL_F */
#define  PANEL_DECORATION_2              15
#define  PANEL_TTL_10                    16      /* callback function: PUT_TTL_F */
#define  PANEL_TTL_9                     17      /* callback function: PUT_TTL_F */
#define  PANEL_SYNCHRO                   18      /* callback function: SYNCHRO_F */
#define  PANEL_GETADC_4                  19      /* callback function: SOFTDEBUG_F */
#define  PANEL_GETADC_3                  20      /* callback function: SOFT_F */
#define  PANEL_GRAPH_TYPE                21      /* callback function: GRAPH_REDRAW_F */
#define  PANEL_DELAY                     22      /* callback function: CHAN_F */
#define  PANEL_RATEHZ                    23      /* callback function: RATE_F */
#define  PANEL_POINTS                    24      /* callback function: CHAN_F */
#define  PANEL_LOOP_2                    25      /* callback function: CHAN_F */
#define  PANEL_DAC                       26      /* callback function: DAC_F */
#define  PANEL_LOAD_FILE                 27      /* callback function: LOAD_FILE_F */
#define  PANEL_SAVE_FILE                 28      /* callback function: SAVE_FILE_F */
#define  PANEL_SAVE                      29      /* callback function: SAVE_F */
#define  PANEL_DONE                      30      /* callback function: Done */
#define  PANEL_CHAN_ON_1                 31      /* callback function: CHAN_F */
#define  PANEL_CHAN_ON_2                 32      /* callback function: CHAN_F */
#define  PANEL_CHAN_ON_3                 33      /* callback function: CHAN_F */
#define  PANEL_CHAN_ON_4                 34      /* callback function: CHAN_F */
#define  PANEL_CHAN_ON_5                 35      /* callback function: CHAN_F */
#define  PANEL_CHAN_ON_6                 36      /* callback function: CHAN_F */
#define  PANEL_CHAN_ON_7                 37      /* callback function: CHAN_F */
#define  PANEL_CHAN_ON_8                 38      /* callback function: CHAN_F */
#define  PANEL_ADC_GAIN_1                39      /* callback function: CHAN_F */
#define  PANEL_ADC_GAIN_2                40      /* callback function: CHAN_F */
#define  PANEL_ADC_GAIN_3                41      /* callback function: CHAN_F */
#define  PANEL_ADC_GAIN_4                42      /* callback function: CHAN_F */
#define  PANEL_ADC_GAIN_5                43      /* callback function: CHAN_F */
#define  PANEL_DECORATION                44
#define  PANEL_ADC_GAIN_6                45      /* callback function: CHAN_F */
#define  PANEL_TEXTMSG_5                 46
#define  PANEL_ADC_GAIN_7                47      /* callback function: CHAN_F */
#define  PANEL_TEXTMSG                   48
#define  PANEL_ADC_GAIN_8                49      /* callback function: CHAN_F */
#define  PANEL_TTL_8                     50
#define  PANEL_DRAW_MODE                 51      /* callback function: GRAPH1_REDRAW_F */
#define  PANEL_TTL_7                     52
#define  PANEL_SCALE_Y                   53      /* callback function: GRAPH_F */
#define  PANEL_Y_min                     54      /* callback function: GRAPH_F */
#define  PANEL_Y_max                     55      /* callback function: GRAPH_F */
#define  PANEL_TTL_6                     56
#define  PANEL_ALL_CHAN                  57      /* callback function: ALL_CHAN_F */
#define  PANEL_SCALE_X                   58      /* callback function: GRAPH_F */
#define  PANEL_X_min                     59      /* callback function: GRAPH_F */
#define  PANEL_X_max                     60      /* callback function: GRAPH_F */
#define  PANEL_FON                       61      /* callback function: GRAPH_F */
#define  PANEL_GRAPH_COLOR               62      /* callback function: GRAPH1_REDRAW_F */
#define  PANEL_GRID_COLOR                63      /* callback function: GRAPH_F */
#define  PANEL_TTL_5                     64
#define  PANEL_TTL_4                     65
#define  PANEL_TTL_3                     66
#define  PANEL_TTL_2                     67
#define  PANEL_TTL_1                     68
#define  PANEL_GRAPH                     69
#define  PANEL_DECORATION_3              70
#define  PANEL_DECORATION_4              71
#define  PANEL_TEXTMSG_2                 72
#define  PANEL_TEXTMSG_3                 73
#define  PANEL_DECORATION_6              74
#define  PANEL_TEXTMSG_11                75
#define  PANEL_TEXTMSG_12                76
#define  PANEL_TEXTMSG_4                 77
#define  PANEL_TEXTMSG_6                 78
#define  PANEL_TEXTMSG_7                 79
#define  PANEL_TEXTMSG_8                 80
#define  PANEL_TEXTMSG_9                 81
#define  PANEL_DECORATION_9              82
#define  PANEL_DECORATION_7              83
#define  PANEL_TIMER                     84      /* callback function: TIMER_F */
#define  PANEL_TEXTMSG_13                85
#define  PANEL_TEXTMSG_10                86
#define  PANEL_LED1_2                    87
#define  PANEL_LED1                      88
#define  PANEL_DECORATION_8              89
#define  PANEL_DECORATION_5              90
#define  PANEL_FBASE                     91
#define  PANEL_DX                        92
#define  PANEL_MX                        93
#define  PANEL_DC                        94
#define  PANEL_AC                        95
#define  PANEL_THDN                      96
#define  PANEL_THD                       97
#define  PANEL_CHANDRAW_8                98      /* callback function: CHANDRAW_F */
#define  PANEL_CHANDRAW_7                99      /* callback function: CHANDRAW_F */
#define  PANEL_CHANDRAW_6                100     /* callback function: CHANDRAW_F */
#define  PANEL_CHANDRAW_5                101     /* callback function: CHANDRAW_F */
#define  PANEL_CHANDRAW_4                102     /* callback function: CHANDRAW_F */
#define  PANEL_CHANDRAW_3                103     /* callback function: CHANDRAW_F */
#define  PANEL_CHANDRAW_2                104     /* callback function: CHANDRAW_F */
#define  PANEL_CHANDRAW_1                105     /* callback function: CHANDRAW_F */
#define  PANEL_GLUKS                     106
#define  PANEL_STRING                    107

#define  SYNCHRO_P                       3
#define  SYNCHRO_P_DONE                  2       /* callback function: DONE_S_F */
#define  SYNCHRO_P_CHANNEL               3
#define  SYNCHRO_P_RINGSLIDE             4       /* callback function: UPDATE_S_F */
#define  SYNCHRO_P_GAIN                  5
#define  SYNCHRO_P_TTL_MASK              6
#define  SYNCHRO_P_MODE2                 7
#define  SYNCHRO_P_MODE1                 8
#define  SYNCHRO_P_ANALOG_TYPE           9       /* callback function: UPDATE_S_F */
#define  SYNCHRO_P_BINARYSWITCH          10
#define  SYNCHRO_P_DECORATION            11
#define  SYNCHRO_P_TEXTMSG               12
#define  SYNCHRO_P_DECORATION_2          13
#define  SYNCHRO_P_POROG                 14
#define  SYNCHRO_P_TEXTMSG_2             15


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK ADCHAN_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ALL_CHAN_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK BURN_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CHAN_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CHANDRAW_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DAC_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Done(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DONE_S_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ExiT_TEST_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK FAST_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GET_TTL_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GRAPH1_REDRAW_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GRAPH_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GRAPH_REDRAW_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK INIT_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK LOAD_FILE_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK LOOP_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MANUAL_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PUT_TTL_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RATE_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RUN_CALIBR_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SAVE_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SAVE_FILE_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SN_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SOFT_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SOFTDEBUG_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK STOP_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SYNCHRO_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TEST_RUN(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TESTALL_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TESTBURN_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TIMER_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TIMER_TEST_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK UPDATE_S_F(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
