#include <wmcliphist.h>
#include <sys/stat.h>

#define	RC_BUF_SIZE	256

/* automat state */
typedef enum {
	STATE_BEGINING,
	STATE_COMMENT,
	STATE_DIRECTIVE,
	STATE_VALUE,
	STATE_EXPRESSION_START,
	STATE_EXPRESSION,
	STATE_EXPRESSION_END,
	STATE_ACTION,
	STATE_COMMAND
} STATE;


GList	*action_list = NULL;


/* add character to buffer */
#define	add_to_buff(buf, pos, chr)	do { \
	buf[pos++] = chr; \
	buf[pos] = '\0'; \
	if (pos + 1 == RC_BUF_SIZE) \
		error = 7; \
} while (0)


#define	ismywhite(c)	(c == '\t' || c == ' ')


/*
 * returns config/data file name in user's home
 */
char *
rcconfig_get_name(char *append)
{
	static gchar	fname[PATH_MAX];
	const gchar	*home;

	begin_func("rcconfig_get_name");

	if (!(home = g_getenv("HOME")))
		return_val(NULL);

	memset(fname, 0, PATH_MAX);
	snprintf(fname, PATH_MAX, "%s/.wmcliphist%s", home, append);

	return_val(fname);
}




/*
 * appends parsed action to action list
 */
int
action_append(char *expr_buf, char *action_buf, char *cmd_buf)
{
	ACTION		*action;

	begin_func("action_append");

	action = g_new0(ACTION, 1);
	
	if (regcomp(&action->expression, expr_buf,
				REG_EXTENDED|REG_ICASE|REG_NOSUB) != 0) {
		g_free(action);
		return_val(-101);
	}

	if (strcmp(action_buf, "exec") == 0)
		action->action = ACT_EXEC;
	else if (strcmp(action_buf, "submenu") == 0)
		action->action = ACT_SUBMENU;
	else if (strcmp(action_buf, "ignore") == 0)
		action->action = ACT_IGNORE;
	else {
		g_free(action);
		return_val(-102);
	}
	
	action->command = g_strdup(cmd_buf);

	action_list = g_list_append(action_list, action);
	
	return_val(0);
}


/*
 * read and parse rcconfig
 */
int
rcconfig_get(char *fname)
{
	int		f_rc;
	char		tmp[1024], c;
	int		byte_cnt;
	STATE		state = STATE_BEGINING;
	char		direc_buf[RC_BUF_SIZE],
			expr_buf[RC_BUF_SIZE],
			action_buf[RC_BUF_SIZE],
			cmd_buf[RC_BUF_SIZE];
	int		buf_index = 0;
	int		error = 0, eof = 0;
	int		i;
	int		line = 0;
	int		res;

	begin_func("rcconfig_get");

	close(open(fname,  O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));

	if ((f_rc = open(fname, O_RDONLY)) < 0) {
		fprintf(stderr, ".wmcliphistrc not found\n");
		return_val(1);
	}

	i = byte_cnt = 0;
	while (1) {
		if (i == byte_cnt) {
			byte_cnt = read(f_rc, tmp, 1024);
			if (byte_cnt == -1) {
				fprintf(stderr, "cannot read .wmcliphistrc\n");
				break;
			} else if (byte_cnt < 1024) {
				tmp[byte_cnt++] = 0;
				eof = 1;
			}
			i = 0;
		}

		c = tmp[i++];
		switch (state) {
			case STATE_BEGINING:
				line++;
				if (isalnum(c)) {
					state = STATE_DIRECTIVE;
					buf_index = 0;
					add_to_buff(direc_buf, buf_index, c);
				} else if (c == '#')
					state = STATE_COMMENT;
				else if (ismywhite(c))
					line--;
				else if (c == '\n')
					state = STATE_BEGINING;
				else
					error = 1;
				break;

			case STATE_COMMENT:
				if (c == '\n' || c == '\0')
					state = STATE_BEGINING;
				break;

			case STATE_DIRECTIVE:
				if (ismywhite(c)) {
					if (strcmp(direc_buf, "action") == 0) {
						state = STATE_EXPRESSION_START;
						buf_index = 0;
					} else if (strcmp(direc_buf, "menukey") == 0) {
						state = STATE_VALUE;
						buf_index = 0;
					} else if (strcmp(direc_buf, "prev_item_key") == 0) {
						state = STATE_VALUE;
						buf_index = 0;
					} else if (strcmp(direc_buf, "exec_item_key") == 0) {
						state = STATE_VALUE;
						buf_index = 0;
					} else if (strcmp(direc_buf, "keep") == 0) {
						state = STATE_VALUE;
						buf_index = 0;
					} else if (strcmp(direc_buf, "lcolor") == 0) {
						state = STATE_VALUE;
						buf_index = 0;
					} else if (strcmp(direc_buf, "autosave") == 0) {
						state = STATE_VALUE;
						buf_index = 0;
					} else if (strcmp(direc_buf,
								"confirm_exec") == 0) {
						state = STATE_VALUE;
						buf_index = 0;
					} else if (strcmp(direc_buf, "exec_immediately") == 0) {
						state = STATE_VALUE;
						buf_index = 0;
					} else if (strcmp(direc_buf, "exec_middleclick") == 0) {
						state = STATE_VALUE;
						buf_index = 0;
					} else if (strcmp(direc_buf, "exec_hotkey") == 0) {
						state = STATE_VALUE;
						buf_index = 0;
					} else if (strcmp(direc_buf, "auto_take_up") == 0) {
						state = STATE_VALUE;
						buf_index = 0;
					} else {
						error = 8;
					}
				} else if (isalpha(c) || c == '_') {
					add_to_buff(direc_buf, buf_index, c);
				} else {
					error = 1;
				}
				break;

			case STATE_VALUE:
				if (c == '\n' || ismywhite(c)) {
					if (strcmp(direc_buf, "menukey") == 0) {
						memset(menukey_str, 0, 32);
						strncpy(menukey_str,
								expr_buf, 31);
					} else if (strcmp(direc_buf, "prev_item_key") == 0) {
						memset(prev_item_key_str, 0, 32);
						strncpy(prev_item_key_str,
								expr_buf, 31);
					} else if (strcmp(direc_buf, "exec_item_key") == 0) {
						memset(exec_item_key_str, 0, 32);
						strncpy(exec_item_key_str,
								expr_buf, 31);
					} else if (strcmp(direc_buf, "keep") == 0) {
						num_items_to_keep =
							atoi(expr_buf);
					} else if (strcmp(direc_buf, "lcolor")
							== 0) {
						memset(locked_color_str, 0, 32);
						strncpy(locked_color_str,
								expr_buf, 31);
					} else if (strcmp(direc_buf, "autosave") == 0) {
						autosave_period =
							atoi(expr_buf);
					} else if (strcmp(direc_buf,
								"confirm_exec") == 0) {
						if (strcasecmp(expr_buf, "yes") == 0) {
							confirm_exec = 1;
						}
					} else if (strcmp(direc_buf, "exec_immediately") == 0) {
						if (strcasecmp(expr_buf, "no") == 0) {
							exec_immediately= 0;
						}
					} else if (strcmp(direc_buf, "exec_middleclick") == 0) {
						if (strcasecmp(expr_buf, "no") == 0) {
							exec_middleclick = 0;
						}
					} else if (strcmp(direc_buf, "exec_hotkey") == 0) {
						if (strcasecmp(expr_buf, "no") == 0) {
							exec_hotkey = 0;
						}
					} else if (strcmp(direc_buf, "auto_take_up") == 0) {
						if (strcasecmp(expr_buf, "no") == 0) {
							auto_take_up = 0;
						}
					} else
						error = 1;
				} else if (isgraph(c))
					add_to_buff(expr_buf, buf_index, c);
				else
					error = 2;

				if (error == 0) {
					if (c == '\n' || c == '\0') {
						buf_index = 0;
						state = STATE_BEGINING;
					} else if (ismywhite(c)) {
						buf_index = 0;
						state = STATE_COMMENT;
					}
				}

				break;

			case STATE_EXPRESSION_START:
				if (c == '"')
					state = STATE_EXPRESSION;
				else
					error = 1;
				break;

			case STATE_EXPRESSION:
				if (c == '"')
					state = STATE_EXPRESSION_END;
				else if (c == '\n')
					error = 1;
				else
					add_to_buff(expr_buf, buf_index, c);
				break;

			case STATE_EXPRESSION_END:
				if (c != ' ' && c != '\t')
					error = 1;
				if (strcmp(direc_buf, "action") == 0) {
					state = STATE_ACTION;
					buf_index = 0;
				} else
					error = 1;
				
				break;

			case STATE_ACTION:
				if (c == ' ' || c == '\t') {
					state = STATE_COMMAND;
					buf_index = 0;
				} else if (c == '\0' || c == '\n') {
					if (strcmp(action_buf, "ignore") == 0) {
						state = STATE_BEGINING;
						buf_index = 0;
						*cmd_buf = '\0';
						res = action_append(
								expr_buf,
								action_buf,
								cmd_buf);
						if (res < 0)
							error = abs(res);
					} else
						error = 1;
				} else if (isalpha(c))
					add_to_buff(action_buf, buf_index, c);
				else
					error = 1;
				break;

			case STATE_COMMAND:
				if (c == '\n' || c == '\0') {
					state = STATE_BEGINING;
					buf_index = 0;
					res = action_append(
							expr_buf,
							action_buf,
							cmd_buf);
					if (res < 0)
						error = abs(res);
				} else
					add_to_buff(cmd_buf, buf_index, c);
				break;
		}

		if (!error && (!eof || i < byte_cnt))
			continue;

		switch (state) {
			case STATE_DIRECTIVE:
				if (error == 7)
					fprintf(stderr, "Directive is too long "
							"(line %d)\n", line);
				else if (error == 8)
					fprintf(stderr, "Unknown directive "
							"(line %d)\n", line);
				else
					fprintf(stderr, "Only letters are "
							"allowed in directive "
							"name (line %d)\n",
							line);
				break;

			case STATE_EXPRESSION_START:
			case STATE_EXPRESSION:
				if (error == 7)
					fprintf(stderr, "Expression is too long "
							"(line %d)\n", line);
				else
					fprintf(stderr, "Expression must be "
							"enclosed with quotes "
							"\" (line %d)\n", line);
				break;

			case STATE_EXPRESSION_END:
				fprintf(stderr, "One space/tab and "
						"action must follow "
						"each expression"
						" (line %d)\n", line);
				break;

			case STATE_ACTION:
				if (error == 1)
					fprintf(stderr, "Only letters are "
							"allowed in action "
							"name (line %d)\n",
							line);
				else if (error == 101)
					fprintf(stderr, "Invalid expression "
							"(line %d)\n", line);
				else if (error == 7)
					fprintf(stderr, "Action is too long "
							"(line %d)\n", line);
				else
					fprintf(stderr, "Invalid action "
							"(line %d)\n", line);
				break;

			case STATE_VALUE:
				if (error == 1)
					fprintf(stderr, "Invalid directive "
							"(line %d)\n", line);
				else if (error == 7)
					fprintf(stderr, "Value is too long "
							"(line %d)\n", line);
				else
					fprintf(stderr, "Invalid value "
							"(line %d)\n", line);
				break;

			case STATE_COMMAND:
				if (error == 101)
					fprintf(stderr, "Invalid expression "
							"(line %d)\n", line);
				else if (error == 7)
					fprintf(stderr, "Command is too long "
							"(line %d)\n", line);
				else
					fprintf(stderr, "Invalid action "
							"(line %d)\n", line);
				break;

			case STATE_COMMENT:
				if (!eof)
					fprintf(stderr, "Unknown error "
							"(line %d)\n", line);
				else
					error = 0;
				break;

			case STATE_BEGINING:
				/* everything is OK */
				error = 0;
				break;
		}

		break;
	}

	close(f_rc);

	return_val(error);
}


/*
 * free rcconfig data
 */
void
rcconfig_free()
{
	GList		*list_node;

	begin_func("rcconfig_free");

	list_node = action_list;
	while (list_node) {
		ACTION		*action = list_node->data;

		g_free(action->command);
		regfree(&action->expression);
		g_free(action);
		list_node = list_node->next;
	}
	g_list_free(action_list);

	return_void();
}
