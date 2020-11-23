APP=cloning-apps
APP_BASE=.

.PHONY: module
module: $(APP)

CFLAGS = -std=gnu99 -fpie -rdynamic -I$(APP_BASE)
LDFLAGS += -lpthread
#LIBS += -lpthread

$(APP): common/boot.o common/net.o main.o server-tcp.o
	$(CC) -pie -o $@ $(CFLAGS) $(LDFLAGS) $^

%.o: %.c
	$(CC) -c -pie -o $@ $(CFLAGS) $<

clean:
	rm -f $(APP)


LIBCLONING_APPS_CINCLUDES-y += -I$(APP_BASE)

LIBCLONING_APPS_SRCS-y += $(APP_BASE)/common/boot.c
LIBCLONING_APPS_SRCS-y += $(APP_BASE)/common/net.c
LIBCLONING_APPS_SRCS-y += $(APP_BASE)/main.c
LIBCLONING_APPS_SRCS-y += $(APP_BASE)/server-tcp.c
