#include "../minishell.h"
#include <stdio.h>

/*__attribute__((destructor))
static void destructor()
{
	system("leaks -q minishell");
}*/

int main()
{
	char	  *line;
	t_token	*tok;
  	t_node  *node;
	//t_node	*fnode;
	//t_redirect *redirect;

	rl_outstream = stderr;
	while (1)
	{
		line = readline("minishell$ ");
		if (line == NULL)
			break;
		if (*line)
			add_history(line);
		if (line[0] == '/' || line[0] == '.')
			abusolute_path(line);
		else
		{
			tok = tokenizer(line);
			// while (tok->word != NULL)
			// {

			// }
			node = parse(tok);
			expand(node);
			// while (node != NULL)
			// {
			// 	printf("%s\n", node->command->args->word);
			// 	if (node->command->redirect != NULL)
			// 	{
			// 		redirect = *(node->command->redirect);
			// 		while (redirect != NULL)
			// 		{
			// 			printf("%s\n", redirect->file_path);
			// 			redirect = redirect->next;
			// 		}
			// 	}
			// 	node = node->next;
			// }
			exec(node);
			if (tok != NULL)
				free_token(tok);
		}
		free(line);
	}
	exit(0);
}
