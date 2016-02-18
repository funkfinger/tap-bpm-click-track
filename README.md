# tap-bpm-click-track
An ATtiny85-based (or Arduino) click track generator with tap BPM - to be used for Teenage Engineering Pocket Operator (PO-12)

Requires [PlatformIO](http://platformio.org/)

My project is ATtiny85-based so I have a custom board file in `~/.platformio/boards` - see [Github repo file](https://github.com/funkfinger/.platformio-boards/blob/master/attiny85.json)

To build:
    
    platformio run
    
To burn:
    
    platformio run -t upload
    
To clean:
    
    platformio run -t clean
    
To set fuses (on my avr dragon):

    avrdude -c dragon_hvsp -p attiny85 -U lfuse:w:0x62:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m -P usb
    
