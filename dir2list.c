#include <dirent.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "config.h"

/* sqrt(SIZE_MAX + 1): for (a < this; b < this) a * b <= SIZE_MAX */
#define MUL_NO_OVERFLOW (1UL << (sizeof(size_t) * 4))

struct node {
	char *path;	/* Path containing media files */
	int a;		/* Applied */
};

/* this makes calls to qsort nicer */
struct entry {
	char *name;
};

static struct node *nodes;
static struct entry *ents;

static size_t n_nodes;
static size_t n_ents;

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

static void *
reallocarray(void *p, size_t n, size_t s)
{
	if ((n >= MUL_NO_OVERFLOW || s >= MUL_NO_OVERFLOW)
	    && n > 0 && SIZE_MAX / n < s)
		die("reallocarray(): Out of Memory\n");

	return realloc(p, n * s);
}

static int
entcmp(const void *va, const void *vb)
{
	const struct entry *a = va, *b = vb;
	return strcmp(a->name, b->name);
}

static int
valid_dir(const char *dir)
{
	if (!strcmp(dir, ".") || !strcmp(dir, ".."))
		return 0;

	return 1;
}

static int
valid_file(const char *file)
{
	const char *s, **types;
	types = filetypes;
	for (; *types; types++) {
		s = *types;
		if (strstr(file, s))
			return 1;
	}
	return 0;
}

static void
addfiles(const char *path)
{
	DIR *dir;
	struct dirent *dent;
	char *s;
	size_t len, n;

	if (!(dir = opendir(path)))
		die("opendir(): failed to open: %s\n", path);

	for (n = 0; (dent = readdir(dir));) {
		if (!valid_file(dent->d_name))
			continue;

		ents = reallocarray(ents, n_ents + 1, sizeof(struct entry));

		len = strlen(path) + strlen(dent->d_name) + 2;
		s = xmalloc(len);
		snprintf(s, len, "%s/%s", path, dent->d_name);
		ents[n_ents++].name = s;
		n++;
	}
	closedir(dir);

	qsort(&ents[n_ents - n], n, sizeof(struct entry), entcmp);
}

/* Fill out the next field for all nodes */
static void
subdir(const char *path)
{
	DIR *dir;
	struct dirent *ent;
	struct stat sb;
	char *s;
	size_t len;

	if (!(dir = opendir(path)))
		die("opendir(): failed to open: %s\n", path);

	while ((ent = readdir(dir))) {
		len = strlen(path) + strlen(ent->d_name) + 2;
		s = xmalloc(len);
		len = snprintf(s, len, "%s/%s", path, ent->d_name);

		stat(s, &sb);
		if (!S_ISDIR(sb.st_mode) || !valid_dir(ent->d_name)) {
			free(s);
			continue;
		}

		nodes = reallocarray(nodes, n_nodes + 1, sizeof(struct node));
		nodes[n_nodes].path = s;
		nodes[n_nodes].a = 0;

		/* recurse into subdir */
		subdir(nodes[n_nodes++].path);
	}
	closedir(dir);
}

static void
mklist(void)
{
	int i, j;
	for (i = 0; i < n_nodes;) {
		j = rand() % n_nodes;

		if (nodes[j].a)
			continue;

		addfiles(nodes[j].path);
		nodes[j].a = 1;
		i++;
	}
}

static void
printlist(void)
{
	int i;
	for (i = 0; i < n_ents; i++)
		fprintf(stdout, "%s\n", ents[i].name);
}

int
main(void)
{
	nodes = reallocarray(nodes, ++n_nodes, sizeof(struct node));

	nodes[0].path = xmalloc(strlen(topdir) + 1);
	nodes[0].path = strcpy(nodes[0].path, topdir);
	nodes[0].a = 0;

	srand(time(NULL));

	subdir(nodes[0].path);
	mklist();

	printlist();

	return 0;
}
