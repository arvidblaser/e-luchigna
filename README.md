# E-luchigna
code for E-luchigna, Hardware version 1.0.

## Building
Use toolchain and sdk 2.6.1. Choose eluchigna as a custom board. Can be found at https://github.com/arvidblaser/jiva-board/tree/main.

As alternative gwen can be chosen as board together with e-luchigna.overlay (no point doing so really).


## Unit tests for screen logic
There is unittests also that can be run on a raspberry pi.

To do so follow this guide for installing zephyr: https://forums.raspberrypi.com/viewtopic.php?t=391754

in the step making a manifest i used hal_nordic instead of the examples (might not matter).

With the venv activated in the zephyr_dev dir the test are built and run with
````
# first time
west build -p always -b native_sim/native/64 -d build/tests ../git/e-luchigna/tests/screen -t run
# when built once a shorter command can be used
west build -d build/tests -t run
````

