# timerdisplay

## Connectivity

Buttons/Buzzers can either be connected to the following pins:
| Pin | Name/Function |
|-----|---------------|
| 35  | Start         |
| 34  | Stop          |
| 39  | Reset         |


## Modes

### Start/Stop
When the button connected to start is pressed, a countdown of 3 seconds will start and the timer will start measuring after the countdown is finished. Pressing the stop button will halt the timer and display the final time. The timer will reset as soon as the start button is pressed, at which point a new countdown will start.

Pressing the start button a second time after the timer has started will reset the timer and start a new countdown. Pressing the start button a second time before the timer has started (i.e. during the countdown) will have no effect. Pressing the stop button a second time will have no effect, only the time measured at the first press will be displayed.

`IM_START_STOP = 0`

### Start/Stop/Reset

Similar to _Start/Stop_, except pressing the start button will never reset the timer. An additional button has to be connected as the reset button, that will reset the timers display to zero when pressed and the timer either is currently either measuring or displaying the final time. The start button will only start the timer after it has been resetted using the reset button.

`IM_START_STOP_RESET = 1`

### Toggle
Pressing the start button will initiate a countdown of 3 seconds. After the countdown a timer will start until the start button is pressed again. 

`IM_TOGGLE = 2`

### Toggle with Reset

Similar to _Toggle_ except pressing the start button will only start and stop the timer/countdown, but never reset. An additional button has to be connected as the reset button, that will reset the timers display to zero everytime it is pressed. The start button will only initiate the countdown/timer after it has been resetted using the reset button.

`IM_TOGGLE_RESET = 3`