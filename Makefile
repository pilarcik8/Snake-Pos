CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -g
LDFLAGS = -pthread

SRC_DIR = src
SERVER_DIR = $(SRC_DIR)/server
CLIENT_DIR = $(SRC_DIR)/client

SERVER_SRCS = \
	$(SERVER_DIR)/server_main.c \
	$(SERVER_DIR)/server.c \
	$(SERVER_DIR)/game.c \
	$(SERVER_DIR)/world.c \
	$(SERVER_DIR)/snake.c \
	$(SERVER_DIR)/ipc_server.c \
	$(SERVER_DIR)/collisions.c \
	$(SERVER_DIR)/fruit.c \

CLIENT_SRCS = \
	$(CLIENT_DIR)/client_main.c \
	$(CLIENT_DIR)/client.c \
	$(CLIENT_DIR)/input.c \
	$(CLIENT_DIR)/render.c \
	$(CLIENT_DIR)/ipc_client.c

SERVER_OBJS = $(SERVER_SRCS:.c=.o) $(COMMON_SRCS:.c=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o) $(COMMON_SRCS:.c=.o)

TARGET_SERVER = server
TARGET_CLIENT = client

.PHONY: all clean server client

all: server client

server: $(TARGET_SERVER)

client: $(TARGET_CLIENT)

$(TARGET_SERVER): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET_CLIENT): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS) $(TARGET_SERVER) $(TARGET_CLIENT)

