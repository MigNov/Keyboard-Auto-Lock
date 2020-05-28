# Automatic Keyboard Lock
---

## Motivation
The automatic keyboard lock (or Keyboard-Auto-Lock) is a tool to automatically lock
screen in both Linux and Windows systems once a new keyboard device is connected.
This prevents the BadUSB devices to control your computer.

## Components
The project consists of following source directories:
- arduino-badusb-test - a simple testing BadUSB implementation for Arduino Leonardo/Lily GO
- linux - source code for Linux implementation of keyboard automatic locker
- windows - source code for Windows implementation of keyboard automatic locker

The compiled version for 32-bit and 64-bit Windows as well as 32-bit and 64-bit Linux
compiled on Fedora 31 is available in the _binary_ directory using cross-compile. The
Makefile for both Windows and Linux is provided.

## Note
The locking happens for all keyboard/mouse devices since there is no way to detect the
device is legitimate keyboard device or not.

If the BadUSB device is having some sleep implemented for 5 minutes and you leave the
device still connected to your computer, login and start working on your computer the
payload may be triggered after the specified time.

---

