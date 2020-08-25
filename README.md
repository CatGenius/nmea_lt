# NMEA to local time converter
This project converts UTC time in an GPRMC NMEA sentence into local time by modifying the contents of the GPRMC sentence. It runs on an inexpesive MicroChip PIC16F15325 and can be built using MicroChip's MPLab X IDE.

The MicroChip microcontroller is placed in the serial line between the NMEA source, suchs as a GPS receiver, and the NMEA sink/consumer, such as a nixie clock. This way, the NMEA receiver always receives local time, hence including corrections for time zone offset and daylight saving time, instead of UTC, preventing the user to change the time offset twice a year.

Please note that both the time zone offset and the daylight saving time switchover algorithms are hard-coded for the EU time zone, since these world wide rules seem a bit of a mess, and memory on this microcontroller is limited.