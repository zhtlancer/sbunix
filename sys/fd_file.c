#include <sys/fs.h>
#include <sys/sched.h>
#include <sys/string.h>
#include <sys/dev.h>
#include <fcntl.h>

#define fs_error(fmt, ...)	\
	k_printf(1, "<FS> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)

#if DEBUG_FS
#define fs_db(fmt, ...)	\
	k_printf(1, "<FS DEBUG> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)
#else
#define fs_db(fmt, ...)
#endif

struct file_operations regular_fops;

/* return <0 on invalid fd */
static inline int check_fd(int fd)
{
	if (fd < 0 || fd >= NFILE_PER_PROC || current->files[fd] == NULL) {
		fs_error("Invalid fd(%d)\n", fd);
		return -1;
	}
	return 0;
}

static struct file *alloc_file(void)
{
	int i;

	for (i = 0; i < NFILE; i++)
		if (files[i].ref == 0) {
			return &files[i];
		}

	panic("No available file buffer!\n");
	return NULL;
}

static void free_file(struct file *file)
{
	memset(file, 0, sizeof(struct file));
}

/* Add ref to file, or allocate a new file copy if file==NULL */
struct file *get_file(struct file *file)
{
	if (file == NULL)
		file = alloc_file();

	file->ref += 1;
	return file;
}

void put_file(struct file *file)
{
	if (file->ref <= 0) {
		fs_error("File has no reference\n");
		panic("Unrecoverable FS error\n");
	}
	file->ref -= 1;
	if (file->ref == 0) {
		if (file->f_ops && file->f_ops->close != NULL)
			file->f_ops->close(file); /* don't operate on inode here */

		if (file->inode) {
			put_inode(file->inode);
			file->inode = NULL;
		}

		free_file(file);
	}
}

void file_close(struct file *file)
{
	if (file->ref <= 0) {
		k_printf(0, "Putting an unreferenced file %p\n", file);
		panic("file_put error\n");
	}
	file->ref -= 1;
	if (file->ref == 0) {
		if (file->f_ops->close != NULL)
			file->f_ops->close(file); /* don't operate on inode here */

		if (file->inode) {
			put_inode(file->inode);
			file->inode = NULL;
		}

		file->readable = 0;
		file->writeable = 0;
		file->offset = 0;
		file->f_ops = NULL;
	}
}

int fd_open(const char *pathname, int flags, mode_t mode)
{
	struct inode *inode;
	int fd = -1;
	struct file *file;

	/* find a usable fd */
	for (fd = 0; fd < NFILE_PER_PROC; fd++)
		if (current->files[fd] == NULL)
			break;

	if (fd >= NFILE_PER_PROC) {
		fs_error("Process %d open file limitation reached\n", current->pid);
		fd = -1;
		return fd;
	}

	file = get_file(NULL);
	if (file == NULL) {
		fs_error("Failed to allocate file struct\n");
		fd = -1;
		return fd;
	}

	inode = path_lookup(current->cwd, pathname);
	if (inode == NULL) {
		fs_db("File '%s' not found\n", pathname);
		fd = -1;
		goto L_free_file;
	}

	file->inode = inode;

	if ((flags & O_DIRECTORY) && (inode->p_inode.type != IT_DIR)) {
		fs_error("Target is not directory\n");
		fd = -1;
		goto L_free_file;
	}

	if ((flags & O_RDWR) && (inode->dev_num == DEV_DISK))
		file->writeable = 1;
	else
		file->writeable = 0;
	file->readable = 1;
	file->offset = 0;
	file->f_ops = &regular_fops;

	current->files[fd] = file;

	return fd;

L_free_file:
	put_file(file);
	return fd;
}

int fd_close(int fd)
{
	struct file *file;
	int rval = 0;

	if (fd >= NFILE_PER_PROC || fd < 0)
		return -1;
	if (current->files[fd] == NULL)
		return -1;

	file = current->files[fd];
	put_file(file);
	current->files[fd] = NULL;

	return rval;
}

int fd_getdents(int fd, struct dirent *dirp, int count)
{
	struct file *file;
	struct inode *inode;
	int rval = 0;
	if (check_fd(fd))
		return -1;

	file = current->files[fd];

	if ((file->offset % sizeof(struct dirent)) != 0) {
		fs_error("File offset (%d) is not aligned by dirent\n", file->offset);
		return -1;
	}

	inode = current->files[fd]->inode;
	if (inode == NULL || inode->fs_ops == NULL || inode->fs_ops->getdirents == NULL)
		return -1;

	rval = inode->fs_ops->getdirents(inode, dirp, file->offset/sizeof(struct dirent), count);
	if (rval > 0)
		file->offset += rval * sizeof(struct dirent);
	return rval;
}

static off_t file_seek(struct file *file, off_t offset, int pos)
{
	struct inode *inode = file->inode;

	if (inode == NULL)
		return -1;

	switch (pos) {
	case SEEK_SET:
		if (offset > inode->p_inode.size) {
			offset = inode->p_inode.size;
		}
		file->offset = offset;
		return offset;
	case SEEK_CUR:
		file->offset += offset;
		if (file->offset > inode->p_inode.size) {
			file->offset = inode->p_inode.size;
		}
		return file->offset;
	case SEEK_END:
		if (offset > inode->p_inode.size) {
			offset = inode->p_inode.size;
		}
		file->offset = inode->p_inode.size - offset;
		return file->offset;
	default:
		fs_error("Invalid whence value (%d)\n", pos);
		return -1;
	}
	return -1;
}

static size_t file_read(struct file *file, void *buf, size_t nbytes)
{
	struct inode *inode = file->inode;
	size_t nread;
	nread = inode->fs_ops->read(inode, buf, file->offset, nbytes);
	file->offset += nread;
	return nread;
}

static size_t file_write(struct file *file, void *buf, size_t nbytes)
{
	struct inode *inode = file->inode;
	size_t nwrite;
	nwrite = inode->fs_ops->write(inode, buf, file->offset, nbytes);
	file->offset += nwrite;
	return nwrite;
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	struct inode *inode;
	vma_t *vma_tmp;
	uint64_t va;
	if (check_fd(fd) < 0 || current->files[fd]->inode == NULL)
		return NULL;

	inode = current->files[fd]->inode;
	if (offset + length > inode->p_inode.size)
		return NULL;

	addr = (void *)PGROUNDDOWN((uint64_t)addr);

	if ((vma_tmp = vma_alloc(current->mm->mmap, (uint64_t)addr, length)) == NULL)
		return NULL;

	for (va = vma_tmp->vm_start; va < vma_tmp->vm_end; va += __PAGE_SIZE) {
		map_page_self(va, 1, 0, PG_USR, 0, 0, 0, PGT_USR | PGT_RW);
	} vma_tmp->file = (addr_t)inode;
	vma_tmp->ofs = offset;

	inode->fs_ops->read(inode, (void *)vma_tmp->vm_start, offset, length);

	vma_insert(current->mm->mmap, vma_tmp);

	return (void *)vma_tmp->vm_start;
}

int munmap(void *addr, size_t length)
{
	vma_t *vma_tmp;
	pgt_t *pgt_tmp;
	uint64_t va;
	struct inode *inode;

	vma_tmp = vma_find(current->mm->mmap, addr);
	if (vma_tmp == NULL)
		return -1;

	inode = (struct inode *)vma_tmp->file;
	for (va = vma_tmp->vm_start; va < vma_tmp->vm_end; va += __PAGE_SIZE) {
		pgt_tmp = get_pgt_entry_lv4_self(va);
		if (pgt_tmp == NULL)
			return -1;
		if (pgt_tmp->flag & PGT_D) {
			/* dirty page, write-back */
			size_t size = vma_tmp->vm_end - va;
			size = size > __PAGE_SIZE ? __PAGE_SIZE : size;
			inode->fs_ops->write(inode, (void *)va, vma_tmp->ofs+(va - vma_tmp->vm_start), size);
		}
	}

	vma_delete(vma_tmp);
	return 0;
}

struct file_operations regular_fops = {
	.seek = file_seek,
	.read = file_read,
	.write = file_write,
};

/* vim: set ts=4 sw=0 tw=0 noet : */
