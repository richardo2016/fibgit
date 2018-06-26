#ifndef INCLUDE_ADD_H
#define INCLUDE_ADD_H

#include "git2.h"

enum print_options {
	SKIP = 1,
	VERBOSE = 2,
	UPDATE = 4,
};

struct print_payload {
	enum print_options options;
	git_repository *repo;
};

extern int test_git_add(int argc, char** argv);

static void parse_opts(int *options, int *count, int argc, char *argv[]);
void init_array(git_strarray *array, int argc, char **argv);
int print_matched_cb(const char *path, const char *matched_pathspec, void *payload);

#endif /* INCLUDE_ADD_H */