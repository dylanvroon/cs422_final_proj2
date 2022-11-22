#include <kern/lib/types.h>
#include <kern/lib/debug.h>
#include <kern/lib/string.h>
#include "inode.h"
#include "dir.h"

// Directories

int dir_namecmp(const char *s, const char *t)
{
    return strncmp(s, t, DIRSIZ);
}

/**
 * Look for a directory entry in a directory.
 * If found, set *poff to byte offset of entry.
 */
struct inode *dir_lookup(struct inode *dp, char *name, uint32_t * poff)
{
    uint32_t off;
    struct dirent *de;
    char data[sizeof(struct dirent)];

    if (dp->type != T_DIR)
        KERN_PANIC("dir_lookup not DIR");

    // KERN_DEBUG("og name: %s\n", name);
    

    //TODO
    // if (inode_read(dp, data, 0, ip->size) != ip->size)
    //     KERN_PANIC("inode_read fucked up")
    for (off = 0; off < dp->size; off += sizeof(struct dirent)) {
        inode_read(dp, data, off, sizeof(struct dirent));
        de = (struct dirent *) data;
        if (de->inum != 0) {
            // KERN_DEBUG("dp name: %s\n", de->name);
        }
        if (de->inum != 0 && dir_namecmp((const char*) name, (const char *) de->name) == 0) {
            *poff = off;
            return inode_get(dp->dev, de->inum);
        }
    }

    return 0;
}

// Write a new directory entry (name, inum) into the directory dp.
int dir_link(struct inode *dp, char *name, uint32_t inum)
{
    uint32_t off;
    struct dirent * de;
    char data[sizeof(struct dirent)];
    struct inode * ip;

    // TODO: Check that name is not present.
    if ((ip = dir_lookup(dp, name, &off)) != 0) {
        inode_put(ip); //accounting for extra reference addecd
        return -1;
    }

    // TODO: Look for an empty dirent.
    for (off = 0; off < dp->size; off += sizeof(struct dirent)) {
        inode_read(dp, data, off, sizeof(struct dirent));
        de = (struct dirent *) data;
        if (de->inum == 0) {
            de->inum = inum;
            strncpy(de->name, (const char *) name, DIRSIZ);
            inode_write(dp, data, off, sizeof(struct dirent));
            return 0;
        }
    }
    //not in dp->size -> we expand
    inode_read(dp, data, off, sizeof(struct dirent));
    de = (struct dirent *) data;
    de->inum = inum;
    strncpy(de->name, (const char *) name, DIRSIZ);
    inode_write(dp, data, off, sizeof(struct dirent));
    return 0;

    // KERN_DEBUG("No open dirent for dir_link");



    // return -1;
}
