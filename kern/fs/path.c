// File system implementation.  Five layers:
//   + Blocks: allocator for raw disk blocks.
//   + Log: crash recovery for multi-step updates.
//   + Files: inode allocator, reading, writing, metadata.
//   + Directories: inode with special contents (list of other inodes!)
//   + Names: paths like /usr/fs.c for convenient naming.
//
// This file contains the low-level file system manipulation
// routines.  The (higher-level) system call implementations
// are in sysfile.c.

#include <kern/lib/types.h>
#include <kern/lib/debug.h>
#include <kern/lib/spinlock.h>
#include <kern/lib/string.h>
#include <thread/PTCBIntro/export.h>
#include <thread/PCurID/export.h>
#include "inode.h"
#include "dir.h"
#include "log.h"

// Paths

/**
 * Copy the next path element from path into name.
 * If the length of name is larger than or equal to DIRSIZ, then only
 * (DIRSIZ - 1) # characters should be copied into name.
 * This is because you need to save '\0' in the end.
 * You should still skip the entire string in this case.
 * Return a pointer to the element following the copied one.
 * The returned path has no leading slashes,
 * so the caller can check *path == '\0' to see if the name is the last one.
 * If no name to remove, return 0.
 *
 * Examples :
 *   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
 *   skipelem("///a//bb", name) = "bb", setting name = "a"
 *   skipelem("a", name) = "", setting name = "a"
 *   skipelem("", name) = skipelem("////", name) = 0
 */
static char *skipelem(char *path, char *name)
{
    // TODO
    uint32_t count, state, nameStart, nameSize;
    nameStart = 0;
    nameSize = 0;
    state = 0; //0 = pre name, 1 = reading name, 2 = post name, 3 returning the rest
    for (count = 0; ; count++) {
        switch (state) {
        case 0: //ignoring all starting /s
            if (path[count] == '\0') {
                return 0;
            } else if (path[count] != '/') {
                nameStart = count;
                state += 1;
            }
            break;
        case 1: //reading path
            if (path[count] == '/') {
                nameSize = count - nameStart;
                if (nameSize > DIRSIZ - 1)
                    nameSize = DIRSIZ - 1;
                state += 1;
            } else if (path[count] == '\0') {
                nameSize = count - nameStart;
                if (nameSize > DIRSIZ - 1)
                    nameSize = DIRSIZ - 1;
                strncpy(name, (const char *) &(path[nameStart]), nameSize);
                name[nameSize] = '\0';
                return &(path[count]);
            }
            break;
        case 2: //
            if (path[count] == '\0') {
                strncpy(name, (const char *) &(path[nameStart]), nameSize);
                name[nameSize] = '\0';
                return &(path[count]);
            } else if (path[count] != '/') {
                strncpy(name, (const char *) &(path[nameStart]), nameSize);
                name[nameSize] = '\0';
                return &(path[count]);
            }
        }
    }
    return 0;
}


/**
 * Look up and return the inode for a path name.
 * If nameiparent is true, return the inode for the parent and copy the final
 * path element into name, which must have room for DIRSIZ bytes.
 * Returns 0 in the case of error.
 */
static struct inode *namex(char *path, bool nameiparent, char *name)
{
    struct inode *ip;
    struct inode *next;

    // If path is a full path, get the pointer to the root inode. Otherwise get
    // the inode corresponding to the current working directory.
    if (*path == '/') {
        ip = inode_get(ROOTDEV, ROOTINO);
    } else {
        ip = inode_dup((struct inode *) tcb_get_cwd(get_curid()));
    }

    while ((path = skipelem(path, name)) != 0) {
        // KERN_DEBUG("curr path: %s\n", path);
        // KERN_DEBUG("curr name: %s\n", name);
        // KERN_DEBUG("name iparent: %s\n", nameiparent ? "True" : "False");
        // TODO
        inode_lock(ip);
        if (ip->type != T_DIR) {
            inode_unlockput(ip);
            return 0;
        }
        // KERN_DEBUG("path[0]: %c\n", path[0]);
        if (nameiparent && path[0] == '\0') {
            // KERN_DEBUG("namex got to parent and empty path\n");
            //copy final path element to name, but I think it's already in there (?¿)
            inode_unlock(ip);
            return ip;
        }
        next = dir_lookup(ip, name, 0);
        if (next == 0) {
            // KERN_DEBUG("Directory not found in namex\n");
            inode_unlockput(ip);
            return 0;
        }
        // KERN_DEBUG("got here ig\n");
        inode_unlockput(ip);
        ip = next;

    }
    if (nameiparent) {
        inode_put(ip);
        return 0;
    }
    // inode_put(ip); //Think this is necessary but not sure
    return ip;
}

/**
 * Return the inode corresponding to path.
 */
struct inode *namei(char *path)
{
    char name[DIRSIZ];
    return namex(path, FALSE, name);
}

/**
 * Return the inode corresponding to path's parent directory and copy the final
 * element into name.
 */
struct inode *nameiparent(char *path, char *name)
{
    return namex(path, TRUE, name);
}
