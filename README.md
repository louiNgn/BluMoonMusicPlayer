# BluMoon Music Player
## a scuffed MP3 player PCB with a scrolling rotary encoder!

<img width="1157" height="310" alt="image" src="https://github.com/user-attachments/assets/59eb1059-7eb5-415b-8bcc-26f3af58aba6" />
<img width="841" height="247" alt="image" src="https://github.com/user-attachments/assets/c843bf9a-be5d-4475-b49d-dd854caa89cb" />



## BOM
| Reference   | Qty | Value                     | DNP | Exclude from BOM | Exclude from Board | Footprint                                                      | Datasheet                                                           |
|-------------|-----|---------------------------|-----|------------------|--------------------|----------------------------------------------------------------|---------------------------------------------------------------------|
| BT1         | 1   | Battery                   |     |                  |                    | Connector_PinSocket_2.54mm:PinSocket_1x02_P2.54mm_Vertical     |                                                                     |
| J1          | 1   | Conn_01x04_Socket         |     |                  |                    | Display:ER_OLEDM0.91_1x-I2C                                    |                                                                     |
| J2          | 1   | Conn_01x04_Socket         |     |                  |                    | Connector_PinSocket_2.54mm:PinSocket_1x04_P2.54mm_Vertical     |                                                                     |
| J3          | 1   | AudioJack3                |     |                  |                    | Connector_Audio:Jack_3.5mm_CUI_SJ1-3533NG_Horizontal           |                                                                     |
| R1          | 1   | 1k                        |     |                  |                    | PCM_SparkFun-Resistor:R_0603_1608Metric                        | https://www.vishay.com/docs/20035/dcrcwe3.pdf                       |
| R2,R3       | 2   | 100k                      |     |                  |                    | PCM_SparkFun-Resistor:R_0603_1608Metric                        | https://www.vishay.com/docs/20035/dcrcwe3.pdf                       |
| SW1         | 1   | RotaryEncoder_Switch      |     |                  |                    | Keebio-Parts.pretty-master:RotaryEncoder_EC11B-Horizontal-Dual |                                                                     |
| SW2         | 1   | SPDT_Slide_PTH_11.6x4.0mm |     |                  |                    | PCM_SparkFun-Switch:Slide_SPDT_PTH_11.6x4.0mm                  | https://www.sparkfun.com/datasheets/Components/Buttons/P040040c.pdf |
| SW3,SW4,SW5 | 3   | SW_Push                   |     |                  |                    | Button_Switch_SMD:SW_Push_1P1T_NO_E-Switch_TL3301NxxxxxG       |                                                                     |
| U1          | 1   | ESP32-S3-SuperMini        |     |                  |                    | ESP-S3_SuperMini:ESP32-S3-SuperMini                            |                                                                     |
| U2          | 1   | DFR0299                   |     |                  |                    | DFPlayerMini:MODULE_DFR0299                                    |                                                                     |
