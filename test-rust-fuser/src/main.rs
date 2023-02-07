// sudo apt-get install -y pkg-config fuse fuse3 libfuse-dev
use clap::{crate_version, Arg, Command};
use fuser::{
    FileAttr, FileType, Filesystem, MountOption, ReplyAttr, ReplyData, ReplyDirectory, ReplyEntry,
    Request, KernelConfig, ReplyCreate, ReplyEmpty, ReplyOpen, ReplyStatfs, ReplyWrite, ReplyXattr, 
    TimeOrNow, FUSE_ROOT_ID,
};
use libc::ENOENT;
use std::ffi::OsStr;
use std::time::{Duration, UNIX_EPOCH};

use log::{debug, warn};
use log::{error, LevelFilter};

use std::os::unix::ffi::OsStrExt;
use std::fs::{File, OpenOptions};

use std::collections::BTreeMap;

const TTL: Duration = Duration::from_secs(1); // 1 second

const HELLO_DIR_ATTR: FileAttr = FileAttr {
    ino: 1,
    size: 0,
    blocks: 0,
    atime: UNIX_EPOCH, // 1970-01-01 00:00:00
    mtime: UNIX_EPOCH,
    ctime: UNIX_EPOCH,
    crtime: UNIX_EPOCH,
    kind: FileType::Directory,
    perm: 0o755,
    nlink: 2,
    uid: 501,
    gid: 20,
    rdev: 0,
    flags: 0,
    blksize: 512,
};

const HELLO_TXT_CONTENT: &str = "Hello World!\n";

const HELLO_TXT_ATTR: FileAttr = FileAttr {
    ino: 2,
    size: 13,
    blocks: 1,
    atime: UNIX_EPOCH, // 1970-01-01 00:00:00
    mtime: UNIX_EPOCH,
    ctime: UNIX_EPOCH,
    crtime: UNIX_EPOCH,
    kind: FileType::RegularFile,
    perm: 0o644,
    nlink: 1,
    uid: 501,
    gid: 20,
    rdev: 0,
    flags: 0,
    blksize: 512,
};

struct HelloFS;

impl Filesystem for HelloFS {

    fn create(
        &mut self,
        req: &Request,
        parent: u64,
        name: &OsStr,
        mut mode: u32,
        _umask: u32,
        flags: i32,
        reply: ReplyCreate,
    ) {
        debug!("create() called with {:?} {:?}", parent, name);
        // if hlp.lookup_name(parent, name).is_ok() {
        //     reply.error(libc::EEXIST);
        //     return;
        // }

        let (read, write) = match flags & libc::O_ACCMODE {
            libc::O_RDONLY => (true, false),
            libc::O_WRONLY => (false, true),
            libc::O_RDWR => (true, true),
            // Exactly one access mode flag must be specified
            _ => {
                reply.error(libc::EINVAL);
                return;
            }
        };

        // let mut parent_attrs = match self.get_inode(parent) {
        //     Ok(attrs) => attrs,
        //     Err(error_code) => {
        //         reply.error(error_code);
        //         return;
        //     }
        // };

        // if !check_access(
        //     parent_attrs.uid,
        //     parent_attrs.gid,
        //     parent_attrs.mode,
        //     req.uid(),
        //     req.gid(),
        //     libc::W_OK,
        // ) {
        //     reply.error(libc::EACCES);
        //     return;
        // }
        // parent_attrs.last_modified = time_now();
        // parent_attrs.last_metadata_changed = time_now();
        // self.write_inode(&parent_attrs);

        if req.uid() != 0 {
            mode &= !(libc::S_ISUID | libc::S_ISGID) as u32;
        }

        // let inode = self.allocate_next_inode();
        // let attrs = InodeAttributes {
        //     inode,
        //     open_file_handles: 1,
        //     size: 0,
        //     last_accessed: time_now(),
        //     last_modified: time_now(),
        //     last_metadata_changed: time_now(),
        //     kind: as_file_kind(mode),
        //     mode: self.creation_mode(mode),
        //     hardlinks: 1,
        //     uid: req.uid(),
        //     gid: creation_gid(&parent_attrs, req.gid()),
        //     xattrs: Default::default(),
        // };
        // self.write_inode(&attrs);
        // File::create(self.content_path(inode)).unwrap();

        // if as_file_kind(mode) == FileKind::Directory {
        //     let mut entries = BTreeMap::new();
        //     entries.insert(b".".to_vec(), (inode, FileKind::Directory));
        //     entries.insert(b"..".to_vec(), (parent, FileKind::Directory));
        //     self.write_directory_content(inode, entries);
        // }

        // let mut entries = self.get_directory_content(parent).unwrap();
        // entries.insert(name.as_bytes().to_vec(), (inode, attrs.kind));
        // self.write_directory_content(parent, entries);

        // TODO: implement flags
        // reply.created(
        //     &Duration::new(0, 0),
        //     &attrs.into(),
        //     0,
        //     self.allocate_next_file_handle(read, write),
        //     0,
        // );
    }

    fn lookup(&mut self, _req: &Request, parent: u64, name: &OsStr, reply: ReplyEntry) {
        if parent == 1 && name.to_str() == Some("hello.txt") {
            reply.entry(&TTL, &HELLO_TXT_ATTR, 0);
        } else {
            reply.error(ENOENT);
        }
    }

    fn getattr(&mut self, _req: &Request, ino: u64, reply: ReplyAttr) {
        match ino {
            1 => reply.attr(&TTL, &HELLO_DIR_ATTR),
            2 => reply.attr(&TTL, &HELLO_TXT_ATTR),
            _ => reply.error(ENOENT),
        }
    }

    fn read(
        &mut self,
        _req: &Request,
        ino: u64,
        _fh: u64,
        offset: i64,
        _size: u32,
        _flags: i32,
        _lock: Option<u64>,
        reply: ReplyData,
    ) {
        if ino == 2 {
            reply.data(&HELLO_TXT_CONTENT.as_bytes()[offset as usize..]);
        } else {
            reply.error(ENOENT);
        }
    }

    fn readdir(
        &mut self,
        _req: &Request,
        ino: u64,
        _fh: u64,
        offset: i64,
        mut reply: ReplyDirectory,
    ) {
        if ino != 1 {
            reply.error(ENOENT);
            return;
        }

        let entries = vec![
            (1, FileType::Directory, "."),
            (1, FileType::Directory, ".."),
            //(2, FileType::RegularFile, "hello.txt"),
        ];

        for (i, entry) in entries.into_iter().enumerate().skip(offset as usize) {
            // i + 1 means the index of the next entry
            if reply.add(entry.0, (i + 1) as i64, entry.1, entry.2) {
                break;
            }
        }
        reply.ok();
    }
}

fn main() {
    let matches = Command::new("hello")
        .version(crate_version!())
        .author("Christopher Berner")
        .arg(
            Arg::new("MOUNT_POINT")
                .required(true)
                .index(1)
                .help("Act as a client, and mount FUSE at given path"),
        )
        .arg(
            Arg::new("auto_unmount")
                .long("auto_unmount")
                .help("Automatically unmount on process exit"),
        )
        .arg(
            Arg::new("allow-root")
                .long("allow-root")
                .help("Allow root user to access filesystem"),
        )
        .get_matches();
    env_logger::init();
    let mountpoint = matches.value_of("MOUNT_POINT").unwrap();
    let mut options = vec![MountOption::RO, MountOption::FSName("hello".to_string())];
    if matches.is_present("auto_unmount") {
        options.push(MountOption::AutoUnmount);
    }
    if matches.is_present("allow-root") {
        options.push(MountOption::AllowRoot);
    }
    fuser::mount2(HelloFS, mountpoint, &options).unwrap();
}
