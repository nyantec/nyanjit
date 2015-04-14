#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

#include "expect.h"

#define SELF "nyanjit"
#define XDG_SUFFIX "/" SELF
#define HOME_SUFFIX "/.cache" XDG_SUFFIX

#define CACHE_OPT "-object-cache-dir="

static char buf[PATH_MAX + sizeof CACHE_OPT] = CACHE_OPT;

static int concat(char *restrict buf,
	char const *restrict prefix, char const *restrict suffix, size_t size) {
	assert(buf);
	assert(prefix);
	assert(suffix);

	int ret = -1;

	size_t prelen = strlen(prefix);
	size_t suflen = strlen(suffix);

	if (unlikely(prelen + suflen >= size))
		goto exit;

	memcpy(buf, prefix, prelen);
	memcpy(buf + prelen, suffix, suflen + 1);

	ret = 0;

exit:
	return ret;
}

static char const *cache_dir_path(char *restrict buf, size_t size) {
	char const *ret = (char *) 0;

	char const *env = getenv("NYANJIT_CACHE_DIR");
	if (env) {
		size_t len = strlen(env);
		if (unlikely(len >= size))
			goto exit;

		memcpy(buf, env, len + 1);

		ret = buf;
		goto exit;
	}

	char const *xdg = getenv("XDG_CACHE_HOME");
	if (xdg) {
		if (unlikely(concat(buf, xdg, XDG_SUFFIX, size)))
			goto exit;

		ret = buf;
		goto exit;
	}

	char const *home = getenv("HOME");
	if (home) {
		if (unlikely(concat(buf, home, HOME_SUFFIX, size)))
			goto exit;

		ret = buf;
		goto exit;
	}

exit:
	return ret;
}

static char const *cache_dir(char *restrict buf, size_t size) {
	char const *ret = (char *) 0;

	char const *path = cache_dir_path(buf, size);
	if (unlikely(!path))
		goto exit;

	struct stat st;
	int rt;

	do {
		/* Try to stat cache directory */
		rt = stat(path, &st);
		if (unlikely(rt)) {
			if (unlikely(errno != ENOENT)) {
				fprintf(stderr,
					SELF ": Unable to stat cache directory ‘%s’: %s\n",
					path, strerror(errno));
				goto exit;
			}

			/* Create cache directory */
			if (unlikely(mkdir(path, 0700) && errno != EEXIST)) {
				fprintf(stderr,
					SELF ": Failed to create cache directory ‘%s’: %s\n",
					path, strerror(errno));
				goto exit;
			}
		}
	} while (unlikely(rt));

	/* Check permissions */
	if (unlikely(st.st_mode & 0022)) {
		fprintf(stderr,
			SELF ": Insecure permissions on cache directory ‘%s’\n", path);
		goto exit;
	}

	ret = path;

exit:
	return ret;
}

int main(int argc, char *argv[]) {
	char const *path = (char *) 0;

	if (!getenv("NYANJIT_CACHE_DISABLE"))
		path = cache_dir(buf + sizeof CACHE_OPT - 1,
			sizeof buf - sizeof CACHE_OPT);

	size_t argc_lli = argc + (path ? 4 : 3);
	size_t argc_off = 0;

	/* Construct argument vector */
	char *argv_lli[argc_lli];
	argv_lli[argc_off++] = LLI_PATH;
	argv_lli[argc_off++] = "-use-mcjit";

	/* Cache directory option */
	if (path) {
		argv_lli[argc_off++] = "-enable-cache-manager";
		argv_lli[argc_off++] = buf;
	}

	argv_lli[argc_off++] = "--";

	/* Copy command‐line arguments */
	for (size_t itr = 0; itr < argc; ++itr)
		argv_lli[argc_off++] = argv[itr + 1];

	/* Terminate argument vector */
	argv_lli[argc_off] = (char *) 0;

	if (unlikely(execv(LLI_PATH, argv_lli)))
		fprintf(stderr,
			SELF ": Unable execute LLVM interpreter: %s\n",
			strerror(errno));

	return EXIT_FAILURE;
}
