### Components:
|Qty|Reference(s)                |Value       | Remarks |
|---|----------------------------|------------|---------|
|1  |C1                          |4.7uF       | Polarised Electrolytic         |
|7  |C2, C3, C4, C6, C7, C8, C9  |0.1uF       |         |
|1  |C5                          |22pF        |         |
|4  |HourA1, HourB1, MinA1, MinB1|IV-9        |         |
|1  |J1                          |USB_B_Micro |         |
|1  |J2                          |ICSP Header |         |
|1  |J3                          |Lamp Test   | Used to test tubes (Short to turn all segments on)        |
|1  |R1                          |10K         |         |
|5  |R2, R3, R4, R5, R6          |1K          |         |
|1  |SW1                         |MinuteButton|         |
|1  |SW2                         |HourButton  |         |
|1  |U1                          |PIC18F47K40 |         |
|1  |U2                          |PCF8583     |         |
|4  |U3, U4, U5, U6              |CD4511      |         |
|1  |Y1                          |32.768kHz   |         |

### Assembly
Assemble all components according to the values above. On the circuit board there is a marker
to show which way round each chip should go – be sure to install the chip with the mark in the same direction.
When installing the tubes, take care to note the orientation. There is a small gap between two of the metal wires
coming out of it, and the gap must be aligned to the same spot on the circuit board. 

For easier assembly of the tubes, bend all wires outwards, and insert them one by one
into the board. Once the tubes are in the board bend them away from the center, and 
check that the tube is straight. Solder one or two wires at a time, and recheck the tube
position each time to make sure it is still correct. Then cut the excess length of each wire to fit.

Once assembled, the device can be powered via it’s micro USB port. It should turn on in 24-hour mode
with a time of 00:00. To set the time, use the minute and hour buttons in the center of the board. The
board does not have a battery backup, so the time will need to be reset each time it is powered on.
