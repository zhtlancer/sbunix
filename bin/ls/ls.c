#include <stdlib.h>
#include <stdio.h>

static void list_dir(const char *path)
{
	struct dirent *dirent;
	DIR *dirp = opendir(path);
	int i = 0;
	if (dirp == NULL)
		return;
	while ((dirent = readdir(dirp)) != NULL) {
		printf("%s\t", dirent->name);
		i += 1;
		if (1 % 2)
			printf("\n");
	}
	closedir(dirp);
}

int main(int argc, char *argv[])
{
	if (argc <= 1)
		list_dir(".");
	else {
		int i;
		for (i = 1; i < argc; i++)
			list_dir(argv[i]);
	}

	return 0;
}
/* vim: set ts=4 sw=0 tw=0 noet : */
