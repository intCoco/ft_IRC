NAME = ircserv
TEST_NAME = run_tests
CC = c++
CFLAGS = -Wall -Werror -Wextra -std=c++98 -fsanitize=address

SRCS = src/main.cpp src/parsing/check.cpp src/parsing/message.cpp src/connection/server.cpp src/connection/client.cpp src/connection/auth.cpp src/commands/commandHandler.cpp src/commands/channel.cpp

TEST_SRCS = tests/test.cpp tests/testing.cpp src/parsing/check.cpp

OBJS = $(SRCS:.cpp=.o)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

test: $(TEST_NAME)
	./$(TEST_NAME)

$(TEST_NAME): $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_OBJS) -o $(TEST_NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) -I includes -c $< -o $@

clean:
	rm -f $(OBJS) $(TEST_OBJS)

fclean: clean
	rm -f $(NAME) $(TEST_NAME)

re: fclean all

.PHONY: all clean fclean re test