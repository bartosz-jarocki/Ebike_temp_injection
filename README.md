# Ebike_temp_injection
This litthe hardware+software injects a temperature to data stream between e-bike "KT" (sinus) controller and KT-LCD3 display.

I bought e-bike motor which apparently didnt have a temperature sensor which work with popular KT motor controller.
KT controller (in my case KT48SVPRL-LCD-C&D) expects thermistor NTC10k. My motor had KTY-83 which have different harecteristic.
Thats why i needed to mount this special "injector" which measure temperature by KTY-83 and inject it to datastream between controller and LCD.

Whole communication protocol decoding was done by M.Sharonov aka obelix662000 on Endless Sphere forum https://endless-sphere.com/forums/viewtopic.php?t=73475
My addition is changing sensor type and temperature calculation.