#include <sys/k_stdio.h>
#include <sys/tarfs.h>
#include <sys/string.h>
#include <sys/mm.h>
#include <sys/error.h>
#include <sys/fs.h>
#include <sys/dev.h>
#include <fcntl.h>

#define TARFS_BLOCK_SIZE	512

#define tarfs_error(fmt, ...)	\
	k_printf(1, "<TARFS> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)

#if DEBUG_TARFS
#define tarfs_db(fmt, ...)	\
	k_printf(1, "<TARFS DEBUG> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)
#else
#define tarfs_db(fmt, ...)
#endif

static struct fs_operations tarfs_ops;

/*
 * Convert octal number string to uint64_t data
 */
static uint64_t get_size(const char *str)
{
	uint64_t size = 0;
	uint64_t base = 1;
	int len = strlen(str);

	while (--len >= 0) {
		size += base * (str[len] - '0');
		base *= 8;
	}

	return size;
}

static inline struct posix_header_ustar *tarfs_next_header(void *ptr, size_t size)
{
	return ptr + ((size + TARFS_BLOCK_SIZE - 1)/TARFS_BLOCK_SIZE + 1) * TARFS_BLOCK_SIZE;
}

/* forward header to next one, mainly used for getdirents, return NULL when last one met */
static inline struct posix_header_ustar *tarfs_header_walk(struct posix_header_ustar *hdr)
{
	uint64_t size = get_size(hdr->size);
	struct posix_header_ustar *new = (void *)hdr + ((size + TARFS_BLOCK_SIZE - 1)/TARFS_BLOCK_SIZE + 1) * TARFS_BLOCK_SIZE;
	if (new->name[0] == '\0')
		return NULL;
	return new;
}

#if DEBUG_TARFS
/* FIXME: Remove the test code */
static void tarfs_test()
{
	volatile int d = 0;
	struct posix_header_ustar *p = (struct posix_header_ustar *)&_binary_tarfs_start;
	TAR_FILE *fp;
	char temp[50];
	size_t len;

	while (d);
	while (p < (struct posix_header_ustar *)&_binary_tarfs_end) {
		uint64_t size = get_size(p->size);
		k_printf(1, "TARFS: %s\n", p->name);
		k_printf(1, "TARFS: %s %d\n", p->size, size);
		p = tarfs_next_header(p, size);
	}

	fp = tarfs_fopen("bin/test");
	memset(temp, 0, sizeof(temp));
	len = tarfs_fread(temp, 1, 100, fp);
	tarfs_db("%d: %s", len, temp);

	memset(temp, 0, sizeof(temp));
	len = tarfs_fread(temp, 1, 100, fp);
	tarfs_db("%d: %s", len, temp);

	return;
}
#endif

static TAR_FILE *tarfs_fopen(const char *name)
{
	TAR_FILE *fp;
	struct posix_header_ustar *p = (struct posix_header_ustar *)&_binary_tarfs_start;

	if (strlen(name) == 0)
		return NULL;

	while (p < (struct posix_header_ustar *)&_binary_tarfs_end) {
		uint64_t size = get_size(p->size);

		if (!strncmp(p->name, name, sizeof(p->name))) {
			fp = kmalloc(sizeof(TAR_FILE), PG_SUP);
			fp->_header = p;
			fp->_ptr = (void *)p + TARFS_BLOCK_SIZE;
			fp->size = size;
			fp->_end = fp->_ptr + fp->size;

			tarfs_db("Found %s = %s, size = %d\n", name, p->name, fp->size);

			return fp;
		}

		p = tarfs_next_header(p, size);
	}
	return NULL;
}

#if 0
static size_t tarfs_fread(void *ptr, size_t size, size_t nmemb, TAR_FILE *fp)
{
	
	if (fp->_ptr >= fp->_end)
		return 0;

	size *= nmemb;

	if ((fp->_ptr + size) >= fp->_end)
		size = fp->_end - fp->_ptr;

	memcpy(ptr, fp->_ptr, size);
	fp->_ptr += size;

	return size;
}

static int tarfs_fseek(TAR_FILE *fp, long offset, int whence)
{
	uint8_t *ptr;
	switch (whence) {
	case TARFS_SEEK_SET:
		if (offset > fp->size)
			fp->_ptr = (void *)fp->_header + TARFS_BLOCK_SIZE + fp->size;
		else
			fp->_ptr = (void *)fp->_header + TARFS_BLOCK_SIZE + offset;
		break;
	case TARFS_SEEK_CUR:
		ptr = fp->_ptr + offset;
		if (ptr > fp->_end)
			ptr = fp->_end;
		fp->_ptr = ptr;
		break;
	case TARFS_SEEK_END:
		if (offset > fp->size)
			offset = fp->size;
		fp->_ptr = fp->_end - offset;
		break;
	default:
		tarfs_db("Invalid whence: %d\n", whence);
		return -EINVAL;
		break;
	}

	return 0;
}
#endif

static void tarfs_fclose(TAR_FILE *fp)
{
	kfree(fp);
}

/* We don't have on-disk hierarchy, just build the full path look for */
static struct inode *tarfs_path_lookup(struct inode *parent, const char *path)
{
	char full_path[TARFS_NAME_MAX];
	int i, j;
	TAR_FILE *tmp;
	TAR_FILE *fp = (TAR_FILE *)parent->priv_data;
	struct inode *inode;
	if (parent->p_inode.type != IT_DIR) {
		tarfs_error("Not a dir inode\n");
		return NULL;
	}

	i = strlcpy(full_path, fp->_header->name, TARFS_NAME_MAX);
	j = 0;
	while (1) {
		while (path[j] != '\0' && path[j] == '/')
			j++;
		if (path[j] == '\0')
			break;
		if (path[j] == '.' && (path[j+1] == '/' || path[j+1] == '\0'))
			j += 1;
		else if (path[j] == '.' && path[j+1] == '.'
				&& (path[j+2] == '/' || path[j+2] == '\0')) {
			j += 2;
			i -= 1;
			while ((--i >= 0) && full_path[i] != '/')
				;
			if (i < 0)
				return path_lookup(rootfs, path+j);
		} else {
			while (path[j] != '\0' && path[j] != '/')
				full_path[i++] = path[j++];
			if (path[j] == '/')
				full_path[i++] = path[j++];
		}
	}
	full_path[i] = '\0';

	if (i == 0)
		return get_inode(rootfs);

	tmp = tarfs_fopen(full_path);
	if (tmp == NULL)
		return NULL;

	inode = get_inode(NULL);
	inode->priv_data = tmp;
	if (full_path[strlen(full_path)-1] == '/')
		inode->p_inode.type = IT_DIR;
	else
		inode->p_inode.type = IT_FILE;
	inode->p_inode.size = tmp->size;
	inode->dev_num = DEV_TARFS;
	inode->fs_ops = &tarfs_ops;

	return inode;
}

static size_t tarfs_read(struct inode *inode, void *dst, off_t off, size_t n)
{
	size_t size;
	TAR_FILE *fp = (TAR_FILE *)inode->priv_data;

	size = fp->size - off;
	size = size > n ? n : size;
	memcpy(dst, fp->_ptr+off, size);

	return size;
}

static size_t tarfs_write(struct inode *inode, void *src, off_t off, size_t n)
{
	panic("Tarfs doesn't support write\n");
	return 0;
}

static int tarfs_is_parent(const char *pre, const char *s)
{
	int i = 0;
	while (pre[i] != '\0' && s[i] != '\0' && pre[i] == s[i])
		i += 1;
	if (pre[i] != '\0')
		return 0;
	while (s[i] != '\0' && s[i] != '/')
		i += 1;
	if (s[i] == '\0' || s[i+1] == '\0')
		return 1;
	return 0;
}

static inline int tarfs_name_copy(char *dst, const char *src)
{
	int i = 0;
	while (i < DIRSIZ && src[i] != '\0' && src[i] != '/') {
		dst[i] = src[i];
		i += 1;
	}
	return i;
}

static int tarfs_getdirents(struct inode *inode, void *buf, int offset, int count)
{
	struct dirent *dirents = (struct dirent *)buf;
	TAR_FILE *fp = (TAR_FILE *)inode->priv_data;
	struct posix_header_ustar *hdr, *hdr_tmp;
	int i;
	size_t len;
	if (inode->p_inode.type != IT_DIR) {
		tarfs_error("Getting dirents on non-dir\n");
		return 0;
	}
	if (count <= 0)
		return 0;

	hdr = fp->_header;
	len = strlen(hdr->name);
	hdr_tmp = (struct posix_header_ustar *)&_binary_tarfs_start;
	i = 0;

	for ( ; hdr_tmp != NULL; hdr_tmp = tarfs_header_walk(hdr_tmp)) {
		if (!tarfs_is_parent(hdr->name, hdr_tmp->name))
			continue;

		if (offset <= 0 && i < count) {
			tarfs_name_copy(dirents[i].name, hdr_tmp->name+len);
			dirents[i].inum = 0;
			i++;
		}
		if (i >= count)
			return i;
		if (offset > 0)
			offset -= 1;
	}

	return i;
}

static void tarfs_close(struct inode *inode)
{
	TAR_FILE *fp = (TAR_FILE *)inode->priv_data;
	tarfs_fclose(fp);
	inode->priv_data = NULL;
}

static struct fs_operations tarfs_ops = {
	.path_lookup	= tarfs_path_lookup,
	.read			= tarfs_read,
	.write			= tarfs_write,
	.getdirents		= tarfs_getdirents,
	.close			= tarfs_close,
};

int tarfs_init()
{
	TAR_FILE *fp;
	struct inode *inode;
#if DEBUG_TARFS
	tarfs_test();
#endif

	inode = get_inode(rootfs);

	fp = kmalloc(sizeof(TAR_FILE), PG_SUP);
	/* point to the last block */
	fp->_header = (struct posix_header_ustar *)(&_binary_tarfs_end - TARFS_BLOCK_SIZE);
	inode->p_inode.type = IT_DIR;
	inode->p_inode.size = 0;
	inode->priv_data = (TAR_FILE *)fp;
	inode->fs_ops = &tarfs_ops;

	put_inode(inode);

	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
