# Toy with ReSpeaker + Rpi ZERO WH

## Features

* RGB LED Control (3 REG LEDs on the ReSpeaker board)
* Button Control (Pressed, Released, Clicked, DoubleClicked and LongPress event)

## Build

Build from HOST PC(x86) using crosscompile docker image

    docker run -it --rm -v $PWD:$PWD -w $PWD nugulinux/devenv:rpi_buster ./build.sh

## Install

### ReSpeaker

https://github.com/respeaker/seeed-voicecard

    git clone https://github.com/respeaker/seeed-voicecard
    cd seeed-voicecard
    sudo ./install.sh
    sudo reboot

### Toy daemon

1. Copy the files in the OUTPUT directory to your Raspberry pi

2. Enable ntoy systmed service

```sh
systemctl enable ntoy
systemctl start ntoy
```


## Usage

### LED control

On/Off the LED

    mdbus2 -s n.toy /LED n.toy.LED.On

Set RGB

    mdbus2 -s n.toy /LED n.toy.LED.RGB 255 255 255

Set Brightness (0 ~ 31)

    mdbus2 -s n.toy /LED n.toy.LED.Brightness 31

You can also control each led using the object path(0 ~ 2):

    mdbus -s n.toy /LED/0 n.toy.LED.Off

### Button event monitoring

    mdbus -sl n.toy

