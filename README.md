This is a utility to aid debugging on XMOS 3 chips.    It writes data to the flash chip in JSON format.  You can then dump the flash to a computer, and extract/analyse the JSON data using Python.

## Configuration

To make a data partition on the flash chip, flash a binary and specify the data partition (by default, there isn't one).

```
xflash  --target=XCORE-AI-EXPLORER --boot-partition-size 2097152 --factory a.xe
```

The flash on the explorer board is 4Mb (32 Mbit)


## Capabilities

This will write a json object to flash, containing integers, floats, and 1D and 2D lists of float.  Numbers are encoded to string, with each consecutive hexadecimal pairs representing a byte from the source data.  These are decoded in Python.



## Dump the flash to a file

```xflash -o test.bin --read-all --target=XCORE-AI-EXPLORER```


## Decoding and Analysis

Follow steps from the Python 3 Notebook in the ```analysis``` folder.

