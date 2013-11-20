#include <sys/k_stdio.h>
#include <sys/tarfs.h>
#include <sys/string.h>
#include <sys/mm.h>
#include <sys/error.h>

#define TARFS_BLOCK_SIZE	512

#if DEBUG_TARFS
#define tarfs_db(fmt, ...)	\
	k_printf(1, "<TARFS DEBUG> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)
#else
#define tarfs_db(fmt, ...)
#endif

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

int tarfs_init()
{
	tarfs_test();

	return 0;
}

TAR_FILE *tarfs_fopen(const char *name)
{
	TAR_FILE *fp;
	struct posix_header_ustar *p = (struct posix_header_ustar *)&_binary_tarfs_start;

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

size_t tarfs_fread(void *ptr, size_t size, size_t nmemb, TAR_FILE *fp)
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

int tarfs_fseek(TAR_FILE *fp, long offset, int whence)
{
	uint8_t *ptr;
	switch (whence) {
	case TARFS_SEEK_SET:
		if (offset > fp->size)
			fp->_ptr = (void *)fp->_header + TARFS_BLOCK_SIZE + fp->size;
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

void tarfs_close(TAR_FILE *fp)
{
	kfree(fp);
}

/* vim: set ts=4 sw=0 tw=0 noet : */
