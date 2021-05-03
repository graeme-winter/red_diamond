# Red Diamond

Code to evaluate how fast one may read Eiger 2XE 16M data from HDF5 files by reading and interpreting the VDS, then using a thread to pull the chunks out of the HDF5 and then decompressing in separate worker threads.

# Dependencies

Building on a Diamond system, following modules should be loaded

1) cmake/3.20.0        3) intel/compilers/2021
2) hdf5/1.10.5         4) gcc/9.2.0
# Building

```
mkdir build
cd build

## GCC build - use the following
cmake -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release ..

## Intel build
cmake -DCMAKE_C_COMPILER=icc -DCMAKE_CXX_COMPILER=icpc -DCMAKE_BUILD_TYPE=Release ..

## Having configured with either GCC or Intel, now you are ready to build
make -j 4

(make -j 4 builds in parallel with 4 processors, make will also work)
```

# Usage

```
red_diamond /path to an appropriate nexus file N
```

where `N` is the number of threads you wish to use. This will simply read out the data and decompress. In the above example a 521 gigapixel (so ~ 10^10 bytes) data set will be read. If this is run within `time` then an overall maximum theoretical speed of processing can be assessed.

This prints out the image number and compressed chunk size to show that work is being done. 

# Example output

On 14 core Intel box get (Intel(R) Xeon(R) W-2275 CPU @ 3.30GHz) for GCC:

```
[qms64511@ws448 build]$ time (./red_diamond /scratch/gw56/ins/insulin1_1.nxs 14)

real	0m4.315s
user	0m55.030s
sys	    0m2.951s
```
On 14 core Intel box get (Intel(R) Xeon(R) W-2275 CPU @ 3.30GHz) for Intel:
```
[qms64511@ws448 build]$ time (./red_diamond /scratch/gw56/ins/insulin1_1.nxs 14)

real	0m5.348s
user	1m10.853s
sys	    0m2.157s
```