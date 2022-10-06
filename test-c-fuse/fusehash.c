// sudo apt-get update && sudo apt-get install -y libfuse-dev
// sudo apt-get install -y pkg-config
// https://engineering.facile.it/blog/eng/write-filesystem-fuse/
// https://github.com/fntlnz/fuse-example
// mkdir /tmp/fusehashes/
// ./fusehash -f /tmp/fusehashes/
// fusermount -u /tmp/fusehashes
// echo -n "" > /tmp/abc.bin

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "hashmap.h"

#define BIN_FILE "/tmp/abc.bin"

#define FILE_NAME_SIZE				64
#define FILE_SIZE		(4 * 1024)

//#define TRACE_OUTPUT	true

static char filepath[1+FILE_NAME_SIZE+1];
static char filename[FILE_NAME_SIZE+1];
static unsigned long positions_count;

static struct HashTree hashTree;

//static const char *filepath = "/file";
//static const char *filename = "file";
static const char *filecontent = "I'm the content of the only file available there\n";

static FILE* fbin_file;

static int getattr_callback(const char *path, struct stat *stbuf) {
#ifdef TRACE_OUTPUT
  printf("getattr_callback path=%s\n", path);
#endif
  obj_type* ppos = NULL;

  memset(stbuf, 0, sizeof(struct stat));

  if (strcmp(path, "/") == 0) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
    return 0;
  }
  
  if (strlen(path) == 1 + FILE_NAME_SIZE) {
    //stbuf->st_mode = S_IFREG | 0777;
    //stbuf->st_nlink = 1;
    //stbuf->st_size = FILE_SIZE;//strlen(filecontent);
    //return 0;
    //printf("HashTreeGet: ");
    ppos = HashTreeGet(&hashTree, (uint8_t*)&path[1]);//&foo;
    if (ppos != NULL) {
#ifdef TRACE_OUTPUT
      printf("%ld\n", *ppos);
#endif

      stbuf->st_mode = S_IFREG | 0777;
      stbuf->st_nlink = 1;
      stbuf->st_size = FILE_SIZE;//strlen(filecontent);
      return 0;
    }
    //printf("Not Found\n");
  }

  if (strcmp(path, filepath) == 0) {
    stbuf->st_mode = S_IFREG | 0777;
    stbuf->st_nlink = 1;
    stbuf->st_size = FILE_SIZE;//strlen(filecontent);
    return 0;
  }


  /*if (strcmp(path, "/1") == 0) {
    stbuf->st_mode = S_IFREG | 0777;
    stbuf->st_nlink = 1;
    stbuf->st_size = 1;
    return 0;
  }

  if (strcmp(path, "/2") == 0) {
    stbuf->st_mode = S_IFREG | 0777;
    stbuf->st_nlink = 1;
    stbuf->st_size = 1;
    return 0;
  }

  if (strcmp(path, "/22") == 0) {
    stbuf->st_mode = S_IFREG | 0777;
    stbuf->st_nlink = 1;
    stbuf->st_size = 0;
    return 0;
  }

  if (strcmp(path, "/3") == 0) {
    stbuf->st_mode = S_IFREG | 0777;
    stbuf->st_nlink = 1;
    stbuf->st_size = 1;
    return 0;
  }*/

  return -ENOENT;
}

static int readdir_callback(const char *path, void *buf, fuse_fill_dir_t filler,
    off_t offset, struct fuse_file_info *fi) {
  (void) offset;
  (void) fi;

#ifdef TRACE_OUTPUT
  printf("readdir_callback path=%s offset=%ld\n", path, offset);
#endif

  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);

  //filler(buf, filename, NULL, 0);
  //filler(buf, "1", NULL, 0);
  //filler(buf, "2", NULL, 0);
  //filler(buf, "3", NULL, 0);

  return 0;
}

/*static int mknod_callback(const char *path, mode_t mode, dev_t dev) {
  printf("mknod_callback path=%s mode=%d\n", path, mode);
  return 0;
}*/

static int create_callback(const char *path, mode_t mode, struct fuse_file_info *fi) {
#ifdef TRACE_OUTPUT
  printf("create_callback path=%s mode=%d\n", path, mode);
#endif

  strcpy(filepath, path);
  //printf("mChildNodes: %ld\n", (unsigned long)hashTree.mRootNode.mChildNodes);
  HashTreeAdd(&hashTree, (uint8_t*)&path[1], &positions_count);
  //printf("mChildNodes: %ld\n", (unsigned long)hashTree.mRootNode.mChildNodes);
  positions_count++;
  //struct fuse_context *cxt = fuse_get_context();
  //if (cxt) {
  //  uid = cxt->uid;
  //  gid = cxt->gid;
  //}
  return 0;
}

/*static int utimens_callback(const char *path, const struct timespec tv[2]) {
  printf("utimens_callback path=%s\n", path);
  return 0;
}

static int chmod_callback(const char *path, mode_t mode) {
  printf("chmod_callback path=%s mode=%d\n", path, mode);
  return 0;
}
 
static int chown_callback(const char *path, uid_t uid, gid_t gid) {
  printf("chown_callback path=%s uid=%d gid=%d\n", path, uid, gid);
  return 0;
}*/

static int open_callback(const char *path, struct fuse_file_info *fi) {
#ifdef TRACE_OUTPUT
  printf("open_callback path=%s\n", path);
#endif

  return 0;
}

static int read_callback(const char *path, char *buf, size_t size, off_t offset,
    struct fuse_file_info *fi) {
  obj_type* ppos = NULL;
  //obj_type foo = 0;

#ifdef TRACE_OUTPUT
  printf("read_callback path=%s size=%ld offset=%ld\n", path, size, offset);
#endif

  /*if (strcmp(path, filepath) == 0) {
    size_t len = strlen(filecontent);
    if (offset >= len) {
      return 0;
    }

    if (offset + size > len) {
      memcpy(buf, filecontent + offset, len - offset);
      return len - offset;
    }

    memcpy(buf, filecontent + offset, size);
    return size;
  }*/
  
  
  /*if (strcmp(path, "/1") == 0) {
    fseek(fbin_file, 0, SEEK_SET);
    fread(buf, 1, 1, fbin_file);
    return 1;
  }
  else if (strcmp(path, "/2") == 0) {
    fseek(fbin_file, 1, SEEK_SET);
    fread(buf, 1, 1, fbin_file);
    return 1;
  }
  else if (strcmp(path, "/3") == 0) {
    fseek(fbin_file, 2, SEEK_SET);
    fread(buf, 1, 1, fbin_file);
    return 1;
  }*/

  ppos = HashTreeGet(&hashTree, (uint8_t*)&path[1]);//&foo;
  if (ppos != NULL) {
#ifdef TRACE_OUTPUT
    printf("HashTreeGet %ld\n", *ppos);
#endif

    fseek(fbin_file, FILE_SIZE * *ppos, SEEK_SET);
    size = fread(buf, size, 1, fbin_file);
    return size;
  }

  return -ENOENT;
}

static int write_callback(const char * path, const char * buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
#ifdef TRACE_OUTPUT
  printf("write_callback path=%s size=%ld offset=%ld\n", path, size, offset);
#endif

  /*if (strcmp(path, "/1") == 0) {
    fseek(fbin_file, 0, SEEK_SET);
    fwrite(buf, 1, 1, fbin_file);
    fflush(fbin_file);
    return 1;
  }
  else if (strcmp(path, "/2") == 0) {
    fseek(fbin_file, 1, SEEK_SET);
    fwrite(buf, 1, 1, fbin_file);
    fflush(fbin_file);
    return 1;
  }
  else if (strcmp(path, "/3") == 0) {
    fseek(fbin_file, 2, SEEK_SET);
    fwrite(buf, 1, 1, fbin_file);
    fflush(fbin_file);
    return 1;
  }
  else*/ if (strcmp(path, filepath) == 0) {
    fseek(fbin_file, 0, SEEK_END);
    fwrite(buf, size, 1, fbin_file);
    return size;
  }

  return -ENOENT;
}

static int truncate_callback(const char *path, off_t offset) {
#ifdef TRACE_OUTPUT
  printf("truncate_callback path=%s offset=%ld\n", path, offset);
#endif

  return 0;
}

static int flush_callback(const char *path, struct fuse_file_info *fi) {
#ifdef TRACE_OUTPUT
  printf("flush_callback path=%s\n", path);
#endif

  fflush(fbin_file);
  return 0;
}

static int release_callback(const char *path, struct fuse_file_info *fi) {
#ifdef TRACE_OUTPUT
  printf("release_callback path=%s\n", path);
#endif

  return 0;
}

static struct fuse_operations fuse_example_operations = {
  .getattr = getattr_callback,
  //.mknod = mknod_callback,
  .create = create_callback,
  //.utimens = utimens_callback,
  //.chmod = chmod_callback,
  //.chown = chown_callback,
  .open = open_callback,
  .read = read_callback,
  .write = write_callback,
  //.truncate = truncate_callback,
  .readdir = readdir_callback,
  .flush = flush_callback,
  //.release = release_callback,
};

int main(int argc, char *argv[])
{
  int res;
  //unsigned long *ppos;
  //uint8_t b[32];
  //printf("HashTreeInit");
  HashTreeInit(&hashTree);
  //ppos = HashTreeGet(&hashTree, b);

  positions_count = 0;

  fbin_file = fopen(BIN_FILE, "a+");//"r+");
  res = fuse_main(argc, argv, &fuse_example_operations, NULL);
  fclose(fbin_file);

  HashNodeDestroy(&(hashTree.mRootNode));

  return res;
}
