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

### Using docker for macOS

I could not compile psptoolchain on my latest&greatest macOS computer,
and had to resort to docker:

```
docker run --name hackburk2 -it --mount type=bind,source=/Users/kristiansallberg/Documents/psp,target=/home/psp ubuntu
```

Go to /home/psp, source env.sh and make
(provided psptoolchain is already compiled)

## Citations

Current source code from:

* https://en.wikibooks.org/wiki/PSP_Programming/General/Common_Callback
* https://en.wikibooks.org/wiki/PSP_Programming/General/Hello_World

# Kuriosa

The PSP's main microprocessor is a multifunction device
named "Allegrex" that includes a 32-bit MIPS32 R4k-based
CPU (Little Endian), a Floating Point Unit,
and a Vector Floating Point Unit.
