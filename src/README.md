# Red Diamond

Code to evaluate how fast one may read Eiger 2XE 16M data from HDF5 files by reading and interpreting the VDS, then using a thread to pull the chunks out of the HDF5 and then decompressing in separate worker threads.

# Dependencies

Requires HDF5 1.10 or later with `h5cc` and the static HDF5 libraries.

# Building

```
cd src
make
```

which gives:

```
h5cc -Wall -O2 -o red_diamond red_diamond_main.c \
	red_diamond_vds.c \
	bitshuffle.c \
	lz4.c \
	bitshuffle_core.c \
	iochain.c
```

... yeh this could use a smarter build system... 

# Usage

```
red_diamond /dls/mx-scratch/gw56/data/i03-protk-28k/protk_1_13_3.nxs N
```

where `N` is the number of threads you wish to use. This will simply read out the data and decompress. In the above example a 521 gigapixel (so ~ 10^10 bytes) data set will be read. If this is run within `time` then an overall maximum theoretical speed of processing can be assessed.

This prints out the image number and compressed chunk size to show that work is being done. 

# Example output

On 32 core AMD Rome box get:

```
cs04r-sc-com16-01 ~ :) $ time ./bin/red_diamond /dls/mx-scratch/gw56/data/i03-protk-28k/protk_1_13_3.nxs 32 > /tmp/junk

real	0m32.234s
user	10m58.344s
sys	0m17.528s
```

which indicates that the 28,800 frames were read from GPFS in around 32s - 893 frames / s which substantially exceeds the acquisition rate. 
