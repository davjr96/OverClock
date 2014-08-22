OverClock
=========

Raspberry PI based clock and weather station

OverClock uses DS18B20 for internal temperature readings and openweathermap.org for outside weather.
Weather is displayed on a 20x4 LCD and time is displayed on a Sparkfun serial 7-segment LED display. Backlight and brightness automatically adjust based on time.
A Powerswitch Tail II is used to control a desk fan depending on time and room temperature.

The entire project is coded in C and uses the wiringPi library (http://wiringpi.com/).
It also uses code from Brad Berkland to read ds18b20. http://bradsrpi.blogspot.com/2013/12/c-program-to-read-temperature-from-1.html
