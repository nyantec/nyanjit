PREFIX   ?= /usr/local/
CFLAGS   ?= -O2 -pipe -g

CPPFLAGS += -D_XOPEN_SOURCE=700 -DLLI_PATH='"$(llidir)/lli"' -std=c99 -Wall
CFLAGS   += -fstack-protector-strong -flto
LDFLAGS  += -Wl,-O1 -Wl,-z,noexecstack

llidir   != llvm-config --bindir
bindir   := $(DESTDIR)/$(PREFIX)/bin

nyanjit: nyanjit.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^

install: $(bindir)/nyanjit

$(bindir)/nyanjit: nyanjit
	install -m 755 $< $@

$(bindir):
	install -d $@

.PHONY: install
