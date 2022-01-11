#define LOGIN 1
#define SIGNUP 2

int client_ops(int sock);
int all_options(int sock, int type);

void ask_login_signup(char *);
void ask_account_type(char *);
void ask_account_number(char *);
void ask_password(char *);
void ask_username(char *);
void ask_customer_agent_choice(char *);

int invalid_input(char *, int);