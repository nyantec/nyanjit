PREFIX   ?= /usr/local/
LIBDIR   ?= lib
CFLAGS   ?= -O2 -pipe -g

CPPFLAGS += -D_XOPEN_SOURCE=700 -DLLI_PATH='"$(bindir_llvm)/lli"' -std=c99 -Wall
CFLAGS   += -fstack-protector-strong -flto
LDFLAGS  += -Wl,-O1 -Wl,-z,noexecstack

bindir_llvm := $(abspath $(shell llvm-config --bindir))
bindir_host := $(abspath $(PREFIX)/bin)
bindir      := $(abspath $(DESTDIR)/$(bindir_host))
binfmtdir   := $(abspath $(DESTDIR)/$(PREFIX)/$(LIBDIR)/binfmt.d)

nyanjit: nyanjit.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^

check: nyanjit helloworld.bc
	NYANJIT_DISABLE_CACHE=1 ./nyanjit helloworld.bc

helloworld.bc: helloworld.c
	clang -c -emit-llvm -o $@ $^

install: $(bindir)/nyanjit $(binfmtdir)/nyanjit.conf

$(bindir)/nyanjit: nyanjit $(bindir)
	install -m 755 $< $@

$(binfmtdir)/nyanjit.conf: $(binfmtdir)
	printf ':llvm:M::BC\\xc0\\xcde::%s:\n' $(bindir_host)/nyanjit >$@

$(bindir) $(binfmtdir):
	install -d $@

.PHONY: check install
