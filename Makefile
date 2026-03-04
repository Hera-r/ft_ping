CC		= clang
CFLAGS	= -Wall -Wextra -Werror
NAME	= ft_ping

SRC_DIR	= src
INC_DIR	= include
OBJ_DIR	= obj

SRC		= $(SRC_DIR)/main.c \
		  $(SRC_DIR)/ping.c \
		  $(SRC_DIR)/stats.c \
		  $(SRC_DIR)/utils.c \
		  $(SRC_DIR)/signal.c

OBJ		= $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/ft_ping.h
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
