#include "../minishell.h"

bool	syntax_error = false;

static	char	*input_char(const char *insert, char *inserted)
{
	while ((*insert) != '\0')
	{
		*inserted = *insert;
		insert = insert + 1;
		inserted = inserted + 1;
	}
	return (inserted);
}

char	*ft_strjoin(char const *s1, char const *s2)
{
	size_t		len;
	char		*dust;
	char		*dust_first;

	if ((s1 == NULL) && (s2 == NULL))
		return (NULL);
	if (s1 == NULL)
		return (strdup(s2));
	if (s2 == NULL)
		return (strdup(s1));
	len = strlen(s1) + strlen(s2);
	dust = (char *)malloc(sizeof(char) * (len + 1));
	if (!(dust))
		return (NULL);
	dust_first = dust;
	dust = input_char(s1, dust);
	dust = input_char(s2, dust);
	*dust = '\0';
	return (dust_first);
}

void	tokenize_error(const char *location, char **rest, char *line)
{
	syntax_error = true;
	dprintf(STDERR_FILENO, "Fatal Error: %s", location);
	while(*line)
		line++;
	*rest = line;
}

bool	is_metacharactert(char c)
{
	return (c && strchr("|&;()<>\t\n", c));
}

bool	is_blank(char c)
{
	return (c == ' ' || c == '\t' || c == '\n');
}

bool	consume_blank(char **rest, char *line)
{
	if (is_blank(*line))
	{
		while (*line && is_blank(*line))
			line++;
		*rest = line;
		return (true);
	}
	*rest = line;
	return (false);
}

bool is_word(const char *s)
{
	return (*s && !is_metacharactert(*s) && !is_blank(*s));
}

bool	startswith(const char *s, const char *keyword)
{
	return (memcmp(s, keyword, strlen(keyword)) == 0);
}

bool	is_redirect(const char *s)
{
	static char *const	redirect[] = {"<", ">"};
	size_t				i;

	i = 0;
	while (i < sizeof(redirect) / sizeof(*redirect))
	{
		if (startswith(s, redirect[i]))
			return (true);
		i++;
	}
	return (false);
}

bool	is_operator(const char *s)
{
	static char *const operators[] = {"||", "&", "&&", ";", ";;", "(", ")", "|", "\n"};
	size_t	i = 0;

	while (i < sizeof(operators) / sizeof(*operators))
	{
		if (startswith(s, operators[i]))
			return (true);
		i++;
	}
	return (false);
}

t_token *new_token(char *word, t_token_kind kind)
{
	t_token *tok;

	tok = calloc(1, sizeof(*tok));
	if (tok == NULL)
		fatal_error("calloc");
	tok->word = word;
	tok->kind = kind;
	return (tok);
}

void append_string_test(char **s, char c)
{
	size_t	size;
	char	*new;

	size = 2;
	if (*s)
		size += strlen(*s);
	new = malloc(size);
	if (new == NULL)
		fatal_error("malloc");
	if (*s)
		ft_strlcpy(new, *s, size);
	new[size - 2] = c;
	new[size - 1] = '\0';
	// if (*s)
	// 	free(*s);
	*s = new;
}

char	*token_append(int flag)
{
	char	*return_str;

	return_str = NULL;
	if (flag == SINGLE)
		return_str = readline("single > ");
	else if (flag == DOUBLE)
		return_str = readline("double > ");
	return_str = ft_strjoin("\n", return_str);
	return (return_str);
}

t_token *word(char **rest, char *line)
{
    char *start;
    char *returnword;
	char *append_line;
	bool flag;

	start = line;
	flag = false;
    while (*line != '\0' && !is_metacharactert(*line) && !is_blank(*line))
	{
		if (*line == '\'')
		{
			line++;
			while (*line != '\'')
			{
				if (*line == '\0')
				{
					append_line = NULL;
					append_line = ft_strjoin(append_line, token_append(SINGLE));
					while (strchr(append_line, '\'') == NULL)
						append_line = ft_strjoin(append_line, token_append(SINGLE));
					start = ft_strjoin(start, append_line);
					flag = false;
					break;
				}
				else
					line++;
			}
			line++;
			if (flag == false)
			{
				flag = true;
				line = start;
			}
		}
		else if (*line == '\"')
		{
			line++;
			while (*line != '\"')
			{
				if (*line == '\0')
				{
					append_line = NULL;
					append_line = ft_strjoin(append_line, token_append(DOUBLE));
					while (strchr(append_line, '\"') == NULL)
						append_line = ft_strjoin(append_line, token_append(DOUBLE));
					start = ft_strjoin(start, append_line);
					flag = false;
					break;
				}
				else
					line++;
			}
			line++;
			if (flag == false)
			{
				flag = true;
				line = start;
			}
		}
		else
			line++;
	}
    returnword = strndup(start, line - start);
    if (returnword == NULL)
        fatal_error("strndup");
    *rest = line;
    return (new_token(returnword, TK_WORD));
}

t_token *operator(char **rest, char *line)
{
	static char *const operators[] = {"||", "&", "&&", ";", ";;", "(", ")", "|", "\n"};
	size_t i = 0;
	char *op;

	while (i < sizeof(operators) / sizeof(*operators))
	{
		if (startswith(line, operators[i]))
		{
			op = strdup(operators[i]);
			if (op == NULL)
				fatal_error("strdup");
			*rest = line + strlen(op);
			return (new_token(op, TK_OP));
		}
		i++;
	}
	fatal_error("Unexpected operator");
}

t_token	*redirect(char **rest, char *line)
{
	static char *const	redirect[] = {"<", ">"};
	size_t				i;
	char				*re;

	i = 0;
	while (i < sizeof(redirect) / sizeof(*redirect))
	{
		if (startswith(line, redirect[i]))
		{
			re = strdup(redirect[i]);
			if (re == NULL)
				fatal_error("strdup");
			*rest = line + strlen(re);
			return (new_token(re, TK_REDIRECT));
		}
		i++;
	}
	fatal_error("Unexpected redirect");
}

t_token *tokenizer(char *line)
{
	t_token	head;
	t_token *tok;

	head.next = NULL;
	tok = &head;
	while (*line)
	{
		if (consume_blank(&line, line))
			continue ;
		else if (is_operator(line))
			tok = tok->next = operator(&line, line);
		else if (is_word(line))
			tok = tok->next = word(&line, line);
		else if (is_redirect(line))
			tok = tok->next = redirect(&line, line);
		else
			tokenize_error("Unexpected Token", &line, line);
	}
	tok->next = new_token(NULL, TK_EOF);
	return (head.next);
}

void	free_token(t_token *head)
{
	t_token	*itr;
	t_token	*next;

	itr = head;
	next = itr->next;
	while (next != NULL)
	{
		free(itr->word);
		free(itr);
		itr = next;
		next = itr->next;
	}
	free(itr->word);
	free(itr);
}


/*********************************


		TEST TOKENIZER


**********************************/

/*void	test_tokenizer(void)
{
	char	*line = "echo hoge | echo huga";
	t_token *t = tokenizer(line);
	t_token *itr = t;
	while(itr->next != NULL)
	{
		printf("%s\n", itr->word);
		itr = itr->next;
	}
	free_token(t);	
	line = "echo\thoge";
	t = tokenizer(line);
	itr = t;
	while(itr->next != NULL)
	{
		printf("%s\n", itr->word);
		itr = itr->next;
	}
	free_token(t);
	line = "echo < hoge";
	t = tokenizer(line);
	itr = t;
	while(itr->next != NULL)
	{
		printf("%s\n", itr->word);
		itr = itr->next;
	}
	free_token(t);

	line = "< in cat";
	t = tokenizer(line);
	itr = t;
	while(itr->next != NULL)
	{
		printf("%s\n", itr->word);
		itr = itr->next;
	}
	free_token(t);
}

__attribute__((destructor))
static void destructor()
{
	system("leaks -q a.out");
}

int	main(void)
{
	test_tokenizer();
}
*/
