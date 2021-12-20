/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: graja <graja@student.42wolfsburg.de>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/15 19:17:12 by graja             #+#    #+#             */
/*   Updated: 2021/12/20 17:22:24 by graja            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>

static
int	get_next_step(int argc, char **argv, int start)
{
	while (start < argc)
	{
		if (!strncmp(argv[start], "|\0", 2))
		{
			argv[start] = NULL;
			return (start);
		}
		else if (!strncmp(argv[start], ";\0", 2))
		{
			argv[start] = NULL;
			return (start * -1);
		}
		start++;
	}
	return (argc - 1);
}

static
size_t	getlen(char *str)
{
	size_t	c;

	c = 0;
	if (!str)
		return (0);
	while (*str)
	{
		c++;
		str++;
	}
	return (c);
}

static
void	print_error(char *str, int flag)
{
	write(2, str, getlen(str));
	if (flag > 0)
		write(2, "\n", 1);
	else if (flag < 0)
		exit(errno);
}

static
int	builtin_cd(char **argv, int start, int fin)
{
	if (fin < 0)
		fin *= -1;
	if (fin - start != 2)
	{
		print_error("error: cd: bad arguments", 1);
		return (fin);
	}
	if (chdir(argv[start + 1]) < 0)
	{
		print_error("error: cannot change directory ", 0);
		print_error(argv[start + 1], 1);
	}
	return (fin);
}

static
int	run_command(char **argv, int start, int stp, char **env)
{
	pid_t	pid;
	int	status;
	static	int	piped = 0;
	
	if (start == stp)
		return (stp);
	pid = fork();
	if (!pid)
	{
		if (execve(argv[start], &argv[start], env) < 0)
		{
			print_error("error: cannot execute ", 0);
			print_error(argv[start], -1);
		}
		exit (0);
	}
	waitpid(pid, &status, 0);
	return (stp);
}

static
void	debug(char **argv, int start, int fin)
{
	static int	i = 1;

	if (fin < 0)
		fin *= -1;
	printf("%3d) ", i);
	while (start < fin)
	{
		printf("%s ", argv[start]);
		start++;
	}
	printf("\n");
	i++;
}

int	main(int argc, char **argv)
{
	int				start;
	int				fin;
	extern	char	**environ;

	start = 1;
	fin = 0;
	if (argc == 1)
		return (0);
	while (start < argc && argv[start])
	{
		fin = get_next_step(argc, argv, start);
		debug(argv, start, fin);
		if (!strncmp(argv[start], "cd\0", 3))
			fin = builtin_cd(argv, start, fin);
		else
			fin = run_command(argv, start, fin, environ);
		start = fin + 1;
	}
	return (0);
}
