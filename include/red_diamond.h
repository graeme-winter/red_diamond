#pragma once

#include <hdf5.h>
#include <hdf5_hl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXFILENAME 256

#define MAXFILESIZE 1000
#define MAXDATAFILES 100

#define MAXDIM 3

typedef struct h5_data_file {
  char filename[MAXFILENAME];
  char dsetname[MAXFILENAME];
  hid_t file;
  hid_t dataset;
  size_t frames;
  size_t offset;
} h5_data_file;

int unpack_vds(char *filename, h5_data_file *data_files);
