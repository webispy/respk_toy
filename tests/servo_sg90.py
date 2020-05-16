import RPi.GPIO as GPIO
from time import sleep

# ReSpeaker - Grove 4 PIN
# +---------+---------+---------+---------+
# |   GND   | VDD 3v3 | GPIO 13 | GPIO 12 |
# +---------+---------+---------+---------+
# |  Black  |   Red   |  White  | Yellow  |
# +---------+---------+---------+---------+
#
# PWM0: 12 - not work
# PWM1: 13 - ok
#
# ----
#
# SG90 Servo motor
#  - 3.5v ~ 7.2v
#  - PWM Period: 20ms (50 Hz)
#  - 180 degrees
#  - Position -90: ~1ms
#  - Position 0: 1.5ms
#  - Position 90: ~2ms
#

GPIO.setmode(GPIO.BCM)
GPIO.setup(13, GPIO.OUT)

servo = GPIO.PWM(13, 50)
servo.start(0)

servo.ChangeDutyCycle(3)
sleep(1)

servo.ChangeDutyCycle(12)
sleep(1)

servo.ChangeDutyCycle(7)
sleep(1)

servo.stop()

GPIO.cleanup()
