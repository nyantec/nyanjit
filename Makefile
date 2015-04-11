CFLAGS   ?= -O2 -pipe -g

CPPFLAGS += -D_XOPEN_SOURCE=700 -std=c99 -Wall
CFLAGS   += -fstack-protector-strong -flto
LDFLAGS  += -Wl,-O1 -Wl,-z,noexecstack

nyanjit: nyanjit.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^
