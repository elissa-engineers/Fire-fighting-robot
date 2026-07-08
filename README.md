# Fire-fighting-robot

## Simplified control logic of the prototype.

Start system

Initialize sensors, motors, pump, and servo

Loop:

Read left, center, and right flame sensors

If no flame is detected:

Stop motors or keep scanning

Else if center sensor detects flame:

Move forward

Else if left sensor detects flame:

Turn left

Else if right sensor detects flame:

Turn right

If suitable distance is reached:

Stop motors
, Activate pump
, Adjust servo/nozzle if needed
, Spray water toward flame

End loop
