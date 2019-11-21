WSSA Lab 4 - Enabling Interrupts

This repo was created for the 4th lab. Enabling an interrupt service routine wasthe focus of the lab -- iterated over lab 3's SPI code.

This lab required the implementation of interrupt functions as wells as a calibration function for the magnetometer. A threshold value was calculated by using a running average of sample values along with the mean and standard deviation. If the magnetic signal exceeded this threshold, an interrupt would be fired and it should be visible on the oscilloscope. 
