# psp

## Dependencies

Currently I only managed to install the psp toolchain on
linux x86-64. I installed all dependencies using 'apt',
set up

```
export PSPDEV=/usr/local/pspdev
export PATH=$PATH:$PSPDEV/bin
```

and then ran './toolchain.sh'.

* https://github.com/pspdev/psptoolchain

## Citations

Current source code from:

* https://en.wikibooks.org/wiki/PSP_Programming/General/Common_Callback
* https://en.wikibooks.org/wiki/PSP_Programming/General/Hello_World

# Kuriosa

The PSP's main microprocessor is a multifunction device
named "Allegrex" that includes a 32-bit MIPS32 R4k-based
CPU (Little Endian), a Floating Point Unit,
and a Vector Floating Point Unit.
