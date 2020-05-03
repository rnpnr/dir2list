#include <dirent.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"

struct node {
	char path[PATH_MAX];	/* Path containing media files */
	int a;			/* Applied */
	struct node *next;	/* next node */
};

struct list {
	char *elem;
	struct list *next;
};

static struct node *node_head;
static struct list *list_head;

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

static struct list *
addfile(struct node *node, struct list *list, const char *file)
{
	char *s;
	size_t len;

	len = strlen(node->path) + strlen(file) + 2;
	s = xmalloc(len);
	s[0] = 0;

	strcat(s, node->path);
	strcat(s, "/");
	strcat(s, file);

	list->elem = s;
	list->next = xmalloc(sizeof(struct list));
	list->next->elem = NULL;
	list->next->next = NULL;

	return list->next;
}

/* Fill out the next field for all nodes */
static struct node *
subdir(struct node *node)
{
	DIR *dir, *tmpdir;
	struct dirent *ent;
	char *s, *t;

	if (!(dir = opendir(node->path)))
		die("opendir(): failed to open: %s\n", node->path);

	s = xmalloc(PATH_MAX);
	s[0] = 0;
	xstrcat(s, node->path, PATH_MAX);
	xstrcat(s, "/", PATH_MAX);

	while ((ent = readdir(dir))) {

		t = xmalloc(PATH_MAX);
		t[0] = 0;
		xstrcat(t, s, PATH_MAX);
		xstrcat(t, ent->d_name, PATH_MAX);

		if ((tmpdir = opendir(t))) {
			closedir(tmpdir);

			if (!valid_dir(ent->d_name)) {
				free(t);
				continue;
			}

			node->next = xmalloc(sizeof(struct node));
			node->next->path[0] = 0;
			xstrcat(node->next->path, t, PATH_MAX);
			node->next->a = 0;
			node->next->next = NULL;

			/* recurse into subdir */
			node = subdir(node->next);
		}
		free(t);
	}
	closedir(dir);
	free(s);

	/* Deepest node with no next pointer allocated */
	return node;
}

static void
mklist(struct node *node, struct list *list)
{
	DIR *dir;
	struct dirent *ent;

	if (!node)
		return;

	/* allocated or "dumb" randomization */
	if (node->a || (rand() % 100) < 77) {
		mklist(node->next, list);
		return;
	}

	if (!(dir = opendir(node->path)))
		die("opendir(): failed to open: %s\n", node->path);

	while ((ent = readdir(dir)))
		if (valid_file(ent->d_name))
			list = addfile(node, list, ent->d_name);

	node->a = 1;
	mklist(node->next, list);
}

static void
printlist(struct list *list)
{
	/* FIXME: we are allocating an extra list ptr */
	if (!list->next)
		return;

	fprintf(stdout, "'%s'\n", list->elem);
	printlist(list->next);
}

int
main(void)
{
	node_head = xmalloc(sizeof(struct node));
	list_head = xmalloc(sizeof(struct list));

	xstrcat(node_head->path, topdir, PATH_MAX);
	node_head->a = 0;
	node_head->next = NULL;

	list_head->elem = NULL;
	list_head->next = NULL;

	srand(time(NULL));

	subdir(node_head);
	mklist(node_head, list_head);

	printlist(list_head);

	return 0;
}
