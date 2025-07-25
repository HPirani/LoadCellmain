
# ➡️ Reverse Engineering 'AND' Lab weighting device with HT1621 LCD

✔️ I Used AVR Atmega16 For RePlacing Withy Its MicroController.

✔️ I Replaced Its Analog Amplification circuit With Hx711 digital ADC, but AD chip Has More Accurate and has Lesser ADC Noise.

✔️ It Has 0.001gr accuracy But increases noise in precise readings, i decreased it to 0.01gr for better and stable readings.

✔️ the HT1621 Used In Had Different Memory Mapping than public ones and i Forced To Create Custom Digit Buffer for it.

✔️ Its Fully functionable and working.

I Used These Libraries:

HT1621 Library : https://github.com/valerionew/ht1621-7-seg

HX711 Library : https://github.com/bogde/HX711

You need To Calibrate Loadcell and change value in ``` constexpr auto DefaultCalibrationFactor = 1679.25f; ``` at line 44.

upload  ```LoadCell_Calibrating.ino``` sketch and calibrate your loadcell. sketch owner: https://RandomNerdTutorials.com/arduino-load-cell-hx711/
