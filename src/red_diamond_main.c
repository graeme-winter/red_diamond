/* hello from 1990 - please compile with h5cc -o fast_hdf5_eiger_read.c  */

#include "bitshuffle.h"
#include <hdf5.h>
#include <hdf5_hl.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "red_diamond.h"

/* global variables */

// mutex to control HDF5 file access, total number of frames, current frame
pthread_mutex_t hdf_mutex;
int n_frames;
int frame;

// pointer to data structures containing information about the actual
// data sets
h5_data_file data_files[MAXDATAFILES];
int data_file_count;
int data_file_current;

// data about the number of bytes per uncompressed pixel, pixels per frame
hid_t datasize;
hsize_t dims[3];

/* data structures to manage the work */

typedef struct chunk_t {
  char *chunk;
  size_t size;
  int index;
} chunk_t;

/* next() method will lock and read the next chunk from the file as needed */

chunk_t next() {
  uint32_t filter = 0;
  hsize_t offset[3], size;
  chunk_t chunk;

  pthread_mutex_lock(&hdf_mutex);

  h5_data_file *current = &(data_files[data_file_current]);

  // figure out in here which block we are working on, scoot over to that
  // and get the next compressed frame from there - this is currently coded
  // around getting the next compressed chunk from a single data set

  offset[0] = frame - current->offset;
  offset[1] = 0;
  offset[2] = 0;
  frame += 1;

  if (frame == (current->offset + current->frames)) {
    data_file_current += 1;
  }

  if (frame > n_frames) {
    pthread_mutex_unlock(&hdf_mutex);
    chunk.size = 0;
    chunk.chunk = NULL;
    chunk.index = 0;
    return chunk;
  }

  H5Dget_chunk_storage_size(current->dataset, offset, &size);
  chunk.index = offset[0] + current->offset;
  chunk.size = (int)size;
  chunk.chunk = (char *)malloc(size);

  H5DOread_chunk(current->dataset, H5P_DEFAULT, offset, &filter, chunk.chunk);
  pthread_mutex_unlock(&hdf_mutex);
  return chunk;
}

// TODO add a FIFO to store the uncompressed data, methods to refill this with
// uncompressed data, interface to pop next frame from the queue

/* stupid interface to worker thread as it must be passed a void * and return
   the same */

void *worker(void *nonsense) {

  /* allocate frame storage - uses global information which is configured
     before threads spawned */

  int size = dims[1] * dims[2];
  char *buffer = (char *)malloc(datasize * size);

  /* while there is work to do, do work */

  while (1) {
    chunk_t chunk = next();
    if (chunk.size == 0) {
      free(buffer);
      return NULL;
    }

    /* decompress chunk - which starts 12 bytes in... */
    bshuf_decompress_lz4((chunk.chunk) + 12, (void *)buffer, size, datasize, 0);

    printf("Read %d %ld\n", chunk.index, chunk.size);

    free(chunk.chunk);
  }
  return NULL;
}

int main(int argc, char **argv) {
  hid_t plist, space, datatype, dataset;
  H5D_layout_t layout;
  herr_t status;

  /* I'll do my own debug printing: disable HDF5 library output */
  H5Eset_auto(H5E_DEFAULT, NULL, NULL);

  data_file_count = unpack_vds(argv[1], data_files);

  if (data_file_count < 0) {
    return 1;
  }

  /* set up global variables */

  frame = 0;
  data_file_current = 0;
  n_frames = 0;

  // print out this VDS info
  for (int j = 0; j < data_file_count; j++) {
    printf("%s/%s -> %ld + %ld\n", data_files[j].filename,
           data_files[j].dsetname, data_files[j].offset, data_files[j].frames);
    data_files[j].file =
        H5Fopen(data_files[j].filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    data_files[j].dataset =
        H5Dopen(data_files[j].file, data_files[j].dsetname, H5P_DEFAULT);
    n_frames += data_files[j].frames;
  }

  dataset = data_files[0].dataset;

  // these things need to be called on the first non-virtual data set
  plist = H5Dget_create_plist(dataset);
  datatype = H5Dget_type(dataset);
  datasize = H5Tget_size(datatype);
  layout = H5Pget_layout(plist);

  if (layout != H5D_CHUNKED) {
    printf("You will not go to space, sorry\n");
    H5Pclose(plist);
    for (int j = 0; j < data_file_count; j++) {
      H5Dclose(data_files[j].dataset);
      H5Fclose(data_files[j].file);
    }
    return 1;
  }

  printf("Element size %lld bytes\n", datasize);

  space = H5Dget_space(dataset);

  printf("N dimensions %d\n", H5Sget_simple_extent_ndims(space));

  H5Sget_simple_extent_dims(space, dims, NULL);

  dims[0] = n_frames;

  for (int j = 0; j < 3; j++) {
    printf("Dimension %d: %lld\n", j, dims[j]);
  }

  /* allocate and spin up threads */

  int n_threads = 1;

  if (argc > 2) {
    n_threads = atoi(argv[2]);
  }

  pthread_t *threads;

  pthread_mutex_init(&hdf_mutex, NULL);

  threads = (pthread_t *)malloc(sizeof(pthread_t) * n_threads);

  for (int j = 0; j < n_threads; j++) {
    pthread_create(&threads[j], NULL, worker, NULL);
  }

  for (int j = 0; j < n_threads; j++) {
    pthread_join(threads[j], NULL);
  }

  pthread_mutex_destroy(&hdf_mutex);

  H5Sclose(space);
  status = H5Pclose(plist);
  for (int j = 0; j < data_file_count; j++) {
    H5Dclose(data_files[j].dataset);
    H5Fclose(data_files[j].file);
  }

  return 0;
}
