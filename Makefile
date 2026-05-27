CC = arm-none-linux-gnueabihf-gcc

CPPFLAGS = -I .
CFLAGS = -g -std=gnu99 -O1 -Wall
LDFLAGS = -static
LDLIBS = -lrt -lpthread

SOURCES = duck_hunters.c
SOURCES += display.c input.c menu.c game.c control.c pause.c
SOURCES += mzapo_phys.c mzapo_parlcd.c serialize_lock.c
SOURCES += font_prop14x16.c font_rom8x16.c

TARGET_EXE = duck_hunters
TARGET_DIR ?= /tmp/$(shell whoami)
TARGET_USER ?= root

OBJECTS = $(SOURCES:%.c=%.o)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

all: $(TARGET_EXE)

$(TARGET_EXE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(CFLAGS) -L. $^ -o $@ $(LDLIBS)

.PHONY: all clean run copy-executable

clean:
	rm -f *.o $(TARGET_EXE)

copy-executable: $(TARGET_EXE)
	ssh -t $(TARGET_USER)@$(TARGET_IP) killall gdbserver 1>/dev/null 2>/dev/null || true
	ssh $(TARGET_USER)@$(TARGET_IP) mkdir -p $(TARGET_DIR)
	scp $(TARGET_EXE) $(TARGET_USER)@$(TARGET_IP):$(TARGET_DIR)/$(TARGET_EXE)

run: copy-executable
	ssh -t $(TARGET_USER)@$(TARGET_IP) $(TARGET_DIR)/$(TARGET_EXE)
