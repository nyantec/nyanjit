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

static char buf[PATH_MAX + 1];

static char *const argv_ext[] = {
	"-use-mcjit",
	"--"
};

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

static char const *cache_dir_path() {
	char const *ret = (char *) 0;

	char const *env = getenv("NYANJIT_CACHE_DIR");
	if (env) {
		size_t len = strlen(env);
		if (unlikely(len >= sizeof buf))
			goto exit;

		memcpy(buf, env, len + 1);

		ret = buf;
		goto exit;
	}

	char const *xdg = getenv("XDG_CACHE_HOME");
	if (xdg) {
		if (unlikely(concat(buf, xdg, XDG_SUFFIX, sizeof buf)))
			goto exit;

		ret = buf;
		goto exit;
	}

	char const *home = getenv("HOME");
	if (home) {
		if (unlikely(concat(buf, home, HOME_SUFFIX, sizeof buf)))
			goto exit;

		ret = buf;
		goto exit;
	}

exit:
	return ret;
}

static char const *cache_dir() {
	char const *ret = (char *) 0;

	char const *path = cache_dir_path();
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
		path = cache_dir();

	size_t argc_ext = sizeof argv_ext / sizeof *argv_ext;
	size_t argc_lli = argc + argc_ext + 1;

	/* Construct argument vector */
	char *argv_lli[argc_lli];
	argv_lli[0] = "NULL";

	/* TODO: Cache directory option */

	/* Extra arguments */
	for (size_t itr = 0; itr < argc_ext; ++itr)
		argv_lli[itr + 1] = argv_ext[itr];

	/* Copy command‐line arguments */
	for (size_t itr = 0; itr < argc; ++itr)
		argv_lli[itr + argc_ext + 1] = argv[itr + 1];

	/* Terminate argument vector */
	argv_lli[argc_lli - 1] = (char *) 0;

	if (unlikely(execvp("lli", argv_lli)))
		fprintf(stderr,
			SELF ": Unable execute LLVM interpreter: %s\n",
			strerror(errno));

	return EXIT_FAILURE;
}
