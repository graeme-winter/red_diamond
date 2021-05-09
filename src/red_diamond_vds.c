#include "red_diamond.h"

/* gather VDS information from top level file:
    - read the VDS
    - dereference into real source data sets
    - if these are external links, re-dereference those to get the true file
    - also extract the extents of data sets while there...
    - write everything back to data files
    - return #files
 */

int vds_info(char *root, hid_t master, hid_t dataset, h5_data_file *vds) {
  hid_t plist, vds_source;
  size_t vds_count;
  herr_t status;

  plist = H5Dget_create_plist(dataset);

  status = H5Pget_virtual_count(plist, &vds_count);

  for (int j = 0; j < vds_count; j++) {
    hsize_t start[MAXDIM], stride[MAXDIM], count[MAXDIM], block[MAXDIM];
    size_t dims;

    vds_source = H5Pget_virtual_vspace(plist, j);
    dims = H5Sget_simple_extent_ndims(vds_source);

    if (dims != 3) {
      H5Sclose(vds_source);
      fprintf(stderr, "incorrect data dimensionality: %d\n", (int)dims);
      return -1;
    }

    H5Sget_regular_hyperslab(vds_source, start, stride, count, block);
    H5Sclose(vds_source);

    H5Pget_virtual_filename(plist, j, vds[j].filename, MAXFILENAME);
    H5Pget_virtual_dsetname(plist, j, vds[j].dsetname, MAXFILENAME);

    for (int k = 1; k < dims; k++) {
      if (start[k] != 0) {
        fprintf(stderr, "incorrect chunk start: %d\n", (int)start[k]);
        return -1;
      }
    }

    if (block[0] > MAXFILESIZE) {
      fprintf(stderr, "bllock size %d > %d\n", (int)block[0], MAXFILESIZE);
      return -1;
    }

    vds[j].frames = block[0];
    vds[j].offset = start[0];

    if ((strlen(vds[j].filename) == 1) && (vds[j].filename[0] == '.')) {
      H5L_info_t info;
      status = H5Lget_info(master, vds[j].dsetname, &info, H5P_DEFAULT);

      if (status) {
        fprintf(stderr, "error from H5Lget_info on %s\n", vds[j].dsetname);
        return -1;
      }

      /* if the data file points to an external source, dereference */

      if (info.type == H5L_TYPE_EXTERNAL) {
        char buffer[MAXFILENAME], scr[MAXFILENAME];
        unsigned flags;
        const char *nameptr, *dsetptr;

        H5Lget_val(master, vds[j].dsetname, buffer, MAXFILENAME, H5P_DEFAULT);
        H5Lunpack_elink_val(buffer, info.u.val_size, &flags, &nameptr,
                            &dsetptr);

        /* assumptions herein:
            - external link references are local paths
            - only need to worry about UNIX paths e.g. pathsep is /
            - ASCII so chars are ... chars
           so manually assemble...
         */

        strcpy(scr, root);
        scr[strlen(root)] = '/';
        strcpy(scr + strlen(root) + 1, nameptr);

        strcpy(vds[j].filename, scr);
        strcpy(vds[j].dsetname, dsetptr);
      }
    } else {
      char scr[MAXFILENAME];
      sprintf(scr, "%s/%s", root, vds[j].filename);
      strcpy(vds[j].filename, scr);
    }

    // do I want to open these here? Or when they are needed...
    vds[j].file = 0;
    vds[j].dataset = 0;
  }

  status = H5Pclose(plist);

  return vds_count;
}

int unpack_vds(char *filename, h5_data_file *data_files) {
  hid_t dataset, file;
  char *root, cwd[MAXFILENAME];
  int retval;

  // TODO if we want this to become SWMR aware in the future will need to
  // allow for that here
  file = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);

  if (file < 0) {
    fprintf(stderr, "error reading %s\n", filename);
    return -1;
  }

  dataset = H5Dopen(file, "/entry/data/data", H5P_DEFAULT);

  if (dataset < 0) {
    H5Fclose(file);
    fprintf(stderr, "error reading %s\n", "/entry/data/data");
    return -1;
  }

  /* always set the absolute path to file information */
  root = dirname(filename);
  if ((strlen(root) == 1) && (root[0] == '.')) {
    root = getcwd(cwd, MAXFILENAME);
  }

  retval = vds_info(root, file, dataset, data_files);

  H5Dclose(dataset);
  H5Fclose(file);

  return retval;
}
