# Numitron Clock
This repository serves as a source for the code running on the PIC18F47K40 at the
heart of the board, as well as assembly instructions for those putting a kit together.

![Clock](/images/clock.jpg)

The source can be found under the [/code](/code) subfolder. It was built using
MPLAB X and XC8. There is a standard ICSP header on the board marked "ICSP" near
the PIC18. To reprogram the device, connect a programmer to this header, and flash
as normal.

The clock is sold via Tindie in kit form, or as a fully assembled unit. Both forms
will come with a preprogrammed microcontroller, so no programmer is needed to get
it running.

The board uses low voltage (5V or under) for all parts, including the number tubes.
All parts on the board are safe to touch, but try to avoid touching the chips, as
they can be sensitive to static.

## Setting the time
To set the time, simply use the buttons marked "Hour Button" or "Minute Button" to
adjust the time.

## 24 hour or 12 hour selection
The board will power up in 24 hour mode by default, unless otherwise specified when
you order the board. To change to 12 or 24 hour mode, hold down both the hour and
minute buttons together for 5 seconds. You should see the clock blink out the new
mode, and then change the time if necessary. If the clock is powered down, it will
return to the default mode again.
