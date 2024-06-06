CC=gcc
CFLAGS=-o

minishell: minishell.c 
	$(CC) $(CFLAGS) minishell minishell.c