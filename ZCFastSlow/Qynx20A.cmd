@Echo Off
call _make.cmd Qynx20A pwm_rc_200
pause
avrdude.exe -C "H:\Program Files\arduino-0022\hardware\tools\avr\etc\avrdude.conf" -v -p m8 -P com3  -c avrisp -b 19200 -U flash:w:bin\Mystery20A.hex 



