

#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>

#define EOL 1
#define ARG 2
#define AMPERSAND 3
#define SEMICOLON 4
#define COMMENT 5

#define MAXARG 512
#define MAXBUF 512
#define MAXPATHLENGTH 512

#define FOREGROUND 0
#define BACKGROUND 1

//extern char **environ;

char *prompt = "smsh";

static char inpbuf[MAXBUF],
	tokbuf[MAXBUF],
	currentpath[MAXPATHLENGTH],
	*ptr = inpbuf,
	*tok = tokbuf;

/* ----------------------------------------------------------------------------- */
/*     printprompt()                           									 */
/* ----------------------------------------------------------------------------- */

void printprompt(char *prompt)
{
	printf("[%s:%s]# ", prompt, currentpath);
}

/* ----------------------------------------------------------------------------- */
/*     userin()																	 */
/* ----------------------------------------------------------------------------- */

int userin(char *p)
{
	int c, count;
	ptr = inpbuf;
	tok = tokbuf;
	printprompt(p);
	count = 0;
	while (1)
	{
		if ((c = getchar()) == EOF)
			return (EOF);
		if (count < MAXBUF)
			inpbuf[count++] = c;
		if (c == '\n' && count < MAXBUF)
		{
			inpbuf[count] = '\0';
			return count;
		}
		if (c == '\n')
		{
			printf("smallsh: input line too long\n");
			printprompt(p);
			count = 0;
		}
	}
}

/* ----------------------------------------------------------------------------- */
/*     is_not_end() 															 */
/* ----------------------------------------------------------------------------- */

int is_not_end(char arg)
{
	enum
	{
		size = 6
	};

	int count, ret = -1;
	char end_table[size] = {'\t', '\n', ' ', '&', ';', '#'};

	for (count = 0; count < size; count++)
		if (arg == end_table[count])
		{
			ret = count;
			break;
		}
	return ret;
}

/* ----------------------------------------------------------------------------- */
/*     gettok()													                 */
/* ----------------------------------------------------------------------------- */

int gettok(char **array)
{

	int choice;
	char *point;

RECHECK:
	point = ptr;

	while ((((choice = is_not_end(*ptr)) < 0)) || (*(ptr - 1) == '\\'))
		ptr++;

	if (ptr == point)
	{
		ptr++;
		switch (choice)
		{
		case EOL:
		case COMMENT:
			return EOL; /* il commento == EOL perch� si ignora fino a EOL */
		case AMPERSAND:
			return AMPERSAND;
		case SEMICOLON:
			return SEMICOLON;
		default:
			goto RECHECK;
		}
	}
	do
	{
		int count = 0, pos = 0;
		while (count < (ptr - point))
		{
			if (*(point + count) != '\\')
				tokbuf[pos] = *(point + count);
			else
			{
				tokbuf[pos] = *(point + count + 1);
				count++;
			}
			count++;
			pos++;
		}
		tokbuf[pos] = '\0';
	} while (0);
	*array = (char *)calloc(ptr - point + 1, sizeof(char));

	sprintf(*array, "%s", tok);
	return ARG;
}

/* ----------------------------------------------------------------------------- */
/*     changedir() :                             		    returns: 0=ok, 1=ko  */
/* ----------------------------------------------------------------------------- */

int changedir(char *dest)
{

	if (dest != NULL)
	{
		if (chdir(dest) == 0)
		{
			setenv("OLDPWD", getenv("PWD"), 1);
			setenv("PWD", dest, 1);
		}
		else
		{
			return 1;
		}
	}
	else
	{
		return 1;
	}

	return 0;
}

/*--------------------------------------------------------------------------------------*/
/* runcommand()							                                                */
/*--------------------------------------------------------------------------------------*/

void runcommand(char **arg, int type)
{

#define EXIT "exit"
#define CD "cd"
#define PWD "pwd"

	int waitme, count, pid;

	/* exit */
	if (!strcmp(arg[0], EXIT))
	{
		exit(0);
	}

	/* pwd (override) */
	if (!strcmp(arg[0], PWD))
	{
		printf("%s\n", currentpath);
		return;
	}

	/* cd */
	if (!strcmp(arg[0], CD))
	{

		if (arg[1] == '\0')
		{
			if (changedir((char *)getenv("HOME")) != 0)
			{
				fprintf(stderr, "cd: cannot change current directory to %s\n", getenv("HOME"));
			}
		}
		else
		{
			if ((strcmp(arg[1], "-") == 0))
			{
				if (changedir((char *)getenv("OLDPWD")) != 0)
				{
					fprintf(stderr, "cd: cannot change current directory to %s\n", getenv("OLDPWD"));
				}
				else
				{
					getcwd(currentpath, MAXPATHLENGTH);
					printf("%s\n", currentpath);
				}
			}
			else
			{
				if (changedir(arg[1]) != 0)
				{
					fprintf(stderr, "cd: cannot change current directory to %s\n", arg[1]);
				}
			}
		}
		getcwd(currentpath, MAXPATHLENGTH);

		free(arg[0]);
		free(arg[1]);
		return;
	}

	pid = fork();
	switch (pid)
	{

	case -1:
		fprintf(stderr, "Cannot execute!\n");
		break;

	case 0:
		execvp(arg[0], arg);
		fprintf(stderr, "smallsh: Error executing %s\n", arg[0]);
		exit(-1);
		break;

	default:
		if (type == FOREGROUND)
			waitpid(pid, &waitme, 0);
		else
		{
			signal(SIGCHLD, SIG_IGN);
			printf("[%d]\n", pid);
		}
	}

	for (count = 0; arg[count]; count++)
	{
		free(arg[count]);
		arg[count] = NULL;
	}
}

/*--------------------------------------------------------------------------------------*/
/* procline()                                                                           */
/*--------------------------------------------------------------------------------------*/

int procline(void)
{
	char *arg[MAXARG + 1];
	int toktype;
	int narg;
	int type;

	narg = 0;

	while (1)
	{
		switch (toktype = gettok(&arg[narg]))
		{
		case ARG:
			if (narg < MAXARG)
				narg++;
			break;
		case EOL:
		case SEMICOLON:
		case AMPERSAND:
			if (toktype == AMPERSAND)
				type = BACKGROUND;
			else
				type = FOREGROUND;

			if (narg != 0)
			{
				arg[narg] = NULL;
				runcommand(arg, type);
			}
			if (toktype == EOL)
				return;

			narg = 0;
			break;
		}
	}
}

/*--------------------------------------------------------------------------------------*/
/* main()                                                                               */
/*--------------------------------------------------------------------------------------*/

int main()
{

	getcwd(currentpath, MAXPATHLENGTH);

	while (userin(prompt) != EOF)
		procline();
	return 0;
}
