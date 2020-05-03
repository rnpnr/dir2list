#include <dirent.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node {
	char path[PATH_MAX];	/* Path containing media files */
	int a;			/* Applied */
	struct node *next;	/* next node */
};

const char *topdir = "/";

static struct node top;

static void
die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	exit(1);
}

static void *
xmalloc(size_t s)
{
	void *p;

	if (!(p = malloc(s)))
		die("malloc()\n");

	return p;		
}

static void
xstrcat(char *dst, const char *src, size_t dst_sz)
{
	if (!(dst_sz - strlen(dst) - strlen(src) - 1 > 0))
		die("Path too long\n");
	strcat(dst, src);
}

static int
valid_dir(const char *dir)
{
	if (!strcmp(dir, ".") && !strcmp(dir, ".."))
		return 1;

	return 0;
}

/* Fill out the next field for all nodes */
static struct node *
subdir(struct node *node)
{
	DIR *dir, *tmpdir;
	struct dirent *ent;
	char *s = xmalloc(PATH_MAX);

	s[0] = 0;

	if (!(dir = opendir(node->path)))
		die("opendir(): failed to open: %s\n", node->path);

	xstrcat(s, node->path, PATH_MAX);
	xstrcat(s, "/", PATH_MAX);

	while ((ent = readdir(dir))) {
		if ((tmpdir = opendir(ent->d_name))) {
			closedir(tmpdir);

			if (!valid_dir(ent->d_name))
				break;

			xstrcat(s, ent->d_name, PATH_MAX);

			node->next = xmalloc(sizeof(struct node));
			xstrcat(node->next->path, s, PATH_MAX);

			/* recurse into subdir */
			node = subdir(node->next);
		}
	}
	closedir(dir);
	free(s);

	/* Deepest node with no next pointer allocated */
	return node;
}

static void
recurse(struct node *node)
{
	subdir(node);	
}

int
main(void)
{
	xstrcat(top.path, topdir, PATH_MAX);
	top.a = 0;
	top.next = NULL;
	recurse(&top);
	return 0;
}
