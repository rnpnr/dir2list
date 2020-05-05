#include <dirent.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "config.h"

struct node {
	char *path;		/* Path containing media files */
	int a;			/* Applied */
	struct node *next;	/* next node */
};

struct list {
	char *elem;
	struct list *next;
};

struct entry {
	char *name;
};

static struct node *node_head;
static struct list *list_head;

static int node_elems = 1;

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

static int
namecmp(const void *va, const void *vb)
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

static struct list *
addfiles(const char *path, struct list *list)
{
	DIR *dir;
	struct dirent *dent;
	struct entry *fents;
	char *s;
	int i;
	size_t len, n = 0;

	if (!(dir = opendir(path)))
		die("opendir(): failed to open: %s\n", path);

	while ((dent = readdir(dir)))
		if (valid_file(dent->d_name))
			n++;

	if (!n) {
		closedir(dir);
		return list;
	}

	rewinddir(dir);

	fents = xmalloc(sizeof(struct entry) * n);

	for (i = 0; i < n;) {
		if (!(dent = readdir(dir)))
			die("readdir(): end of dir: %s\n", path);

		if (!valid_file(dent->d_name))
			continue;

		len = strlen(path) + strlen(dent->d_name) + 2;
		s = xmalloc(len);
		snprintf(s, len, "%s/%s", path, dent->d_name);
		fents[i].name = s;

		i++;
	}
	closedir(dir);

	qsort(fents, n, sizeof(struct entry), namecmp);

	for (i = 0; i < n; i++) {
		list->elem = fents[i].name;
		list->next = xmalloc(sizeof(struct list));
		list = list->next;
	}
	free(fents);

	list->next = NULL;

	return list;
}

/* Fill out the next field for all nodes */
static struct node *
subdir(struct node *node)
{
	DIR *dir;
	struct dirent *ent;
	struct stat sb;
	char *s, *t;
	size_t s_len, t_len;

	if (!(dir = opendir(node->path)))
		die("opendir(): failed to open: %s\n", node->path);

	/* node changes in while() but we need this first path */
	s_len = strlen(node->path) + 1;
	s = xmalloc(s_len);
	s_len = snprintf(s, s_len, "%s", node->path);

	while ((ent = readdir(dir))) {

		t_len = s_len + strlen(ent->d_name) + 2;
		t = xmalloc(t_len);
		t_len = snprintf(t, t_len, "%s/%s", s, ent->d_name);

		stat(t, &sb);
		if (S_ISDIR(sb.st_mode)) {
			if (!valid_dir(ent->d_name)) {
				free(t);
				continue;
			}

			node->next = xmalloc(sizeof(struct node));
			node->next->path = xmalloc(t_len + 1);
			node->next->path = strcpy(node->next->path, t);
			node->next->a = 0;
			node->next->next = NULL;

			node_elems++;

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
	struct node *o_node = node;
	int i, j;
	for (i = node_elems; i > 0;) {
		for (j = rand() % node_elems; node && j > 0; j--)
			node = node->next;

		if (!node || node->a) {
			node = o_node;
			continue;
		}

		list = addfiles(node->path, list);
		node->a = 1;
		node = o_node;
		i--;
	}
}

static void
printlist(struct list *list)
{
	/* FIXME: we are allocating an extra list ptr */
	if (!list->next)
		return;

	fprintf(stdout, "%s\n", list->elem);
	printlist(list->next);
}

int
main(void)
{
	node_head = xmalloc(sizeof(struct node));
	list_head = xmalloc(sizeof(struct list));

	node_head->path = xmalloc(strlen(topdir) + 1);
	node_head->path = strcpy(node_head->path, topdir);
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
