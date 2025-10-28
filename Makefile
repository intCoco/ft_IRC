# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: chuchard <chuchard@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/10/07 15:29:20 by nihamdan          #+#    #+#              #
#    Updated: 2025/10/28 02:12:35 by chuchard         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

RED		=	\033[31m
GRE		=	\033[32m
BLU		=	\033[36m
YEL		=	\033[33m
EOC		=	\033[0m
BEI		=	\033[38;5;223m

DEF		=	\033[0m
BOLD	=	\033[1m
CUR		=	\033[3m
UL		=	\033[4m
UP		=	\033[A

NAME 			= ft_irc

SOURCES			=	./sources
INCLUDES		=	./includes
OBJECTS			=	./bin

SRCS 			=	main.cpp client.cpp server.cpp channel.cpp commandHandler.cpp

OBJS			=	$(addprefix ${OBJECTS}/, $(SRCS:.cpp=.o))

CFLAGS			=	-Wall -Wextra -Werror -std=c++98
CC				=	c++
CINCLUDES		=	-I ${INCLUDES}
CDEPENDENCIES	=
# ---------------------------------------------------------------------------- #

SRCS_COUNT = 0
SRCS_TOT = ${shell find ./sources/ -type f -name '*.cpp' | wc -l}
SRCS_PRCT = ${shell expr 100 \* ${SRCS_COUNT} / ${SRCS_TOT}}
BAR =  ${shell expr 23 \* ${SRCS_COUNT} / ${SRCS_TOT}}

${OBJECTS}/%.o: ${SOURCES}/%.cpp
	@${eval SRCS_COUNT = ${shell expr ${SRCS_COUNT} + 1}}
	@mkdir -p $(dir $@)
	@${CC} ${CFLAGS} -o $@ -c $< ${CINCLUDES}
	@echo ""
	@echo "\r\033[K -> Compilation de ""$(YEL)${notdir $<}$(EOC). ⏳"
	@printf "   ${BEI}[%-23.${BAR}s] [%d/%d (%d%%)]${DEF}" "***********************" ${SRCS_COUNT} ${SRCS_TOT} ${SRCS_PRCT}
	@echo "${UP}${UP}${UP}"

all: start ${NAME}

start:
	@if [ ! -e "bin" ]; then \
		echo "🚀 Début de la compilation de $(BLU)${NAME} 🚀$(EOC)"; \
	else \
		echo "make: Nothing to be done for \`all'."; \
	fi

${NAME}: ${OBJS}
	@clear
	@${CC} ${CFLAGS} -o ${NAME} ${OBJS}
	@echo "$(GRE)✅ Compilation terminée.$(EOC)"

clean:
	@echo "🗑  $(RED)Supression des fichiers binaires (.o).$(EOC) 🗑"
	@rm -rf ${OBJECTS}

fclean: clean
	@echo "🗑  $(RED)Supression des executables et librairies.$(EOC) 🗑"
	@rm -f ${NAME}
	@clear

re: fclean all

.PHONY:	all clean fclean re