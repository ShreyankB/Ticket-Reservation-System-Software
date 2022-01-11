#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../../constants/c_constants.h"
#include "../../header/client/login_signup.h"
#include "../../header/client/customer_ops.h"
#include "../../header/client/admin_ops.h"

int client_ops(int sd)
{
	int opt;
	char buf[100];
	int type, acc_no, valid;
	char password[PASS_LENGTH], secret_pin[5], name[10];

	ask_login_signup(buf);
	opt = atoi(buf);

	if (opt < 1 || opt > 3)
		return invalid_input("Invalid input", 0);

	write(sd, &opt, sizeof(opt)); // 1

	switch (opt)
	{
	case LOGIN:
	{
		ask_account_type(buf);
		type = atoi(buf);

		if (type < 1 || type > 3)
			return invalid_input("Invalid input", 0);

		ask_account_number(buf);
		acc_no = atoi(buf);
		ask_password(password);

		write(sd, &type, sizeof(type));			// 2
		write(sd, &acc_no, sizeof(acc_no));		// 3
		write(sd, &password, strlen(password)); // 4
		read(sd, &valid, sizeof(valid));		// 5

		if (valid == 1)
		{
			while (all_options(sd, type) != -1)
				;
			system("clear");
			return 1;
		}
		else
			return invalid_input("Login Failed.", 1);

		break;
	}
	case SIGNUP:
	{
		ask_account_type(buf);
		type = atoi(buf);

		if (type < 1 || type > 3)
			return invalid_input("Invalid choice.", 0);

		ask_username(name);
		ask_password(password);

		if (type == 3)
		{
			while (1)
			{
				strcpy(secret_pin, getpass("PIN\t:"));

				if (strcmp(secret_pin, "admin") != 0)
					printf("Invalid PIN. Please try again.\n");
				else
					break;
			}
		}
		write(sd, &type, sizeof(type));
		write(sd, &name, sizeof(name));
		write(sd, &password, PASS_LENGTH);
		read(sd, &acc_no, sizeof(acc_no));

		printf("\nRemember the account no of further login: %d\n", acc_no);

		while (getchar() != '\n')
			;
		getchar();
		return 2;
	}
	default:
		return 3;
	}
}

void ask_login_signup(char *buff)
{
	system("clear");
	printf(":: Online Ticket Reservation System ::\n");
	printf("1. login\n2. signup\n3. exit\n:");
	scanf("%s", buff);
}

void ask_account_type(char *buff)
{
	system("clear");
	printf(":: Online Ticket Reservation System ::\n");
	printf("\n1. Customer\n2. Agent\n3. Admin\n:");
	scanf("%s", buff);
}

void ask_account_number(char *buff)
{
	system("clear");
	printf(":: Online Ticket Reservation System ::\n");
	printf("Enter Your Account Number: ");
	scanf("%s", buff);
}

int invalid_input(char *msg, int exit_code)
{
	printf("%s\nPress [Enter] to return...\n", msg);
	while (getchar() != '\n')
		;
	return 0;
}

void ask_password(char *password)
{
	memset(password, '\0', PASS_LENGTH);

	while (strlen(password) <= 0)
		strcpy(password, getpass("password:\t"));
}

void ask_username(char *buff)
{
	system("clear");
	printf(":: Online Ticket Reservation System ::\n");
	printf("username:\t");
	scanf("%s", buff);
}

void ask_customer_agent_choice(char *buff)
{
	system("clear");
	printf(":: Online Ticket Reservation System ::\n");
	printf("1. Book Tickets\n2. View Bookings\n3. Update Booking\n4. Cancel booking\n5. Logout\n:");
	scanf("%s", buff);
}

void ask_admin_choice(char *buff)
{
	system("clear");
	printf(":: Online Ticket Reservation System ::\n");
	printf("1. Add Train\n2. Delete Train\n3. Modify Train\n");
	printf("4. Add User\n5. Delete User\n6. Modify User\n");
	printf("7. View All Users\n8. View All Trains\n9. View All Bookings\n");
	printf("10. Search User\n11. Search Train\n12. Logout\n:");
	scanf("%s", buff);
}

int all_options(int sd, int type)
{
	int opt = 0;
	char buf[100];

	if (type == 1 || type == 2)
	{
		ask_customer_agent_choice(buf);
		opt = atoi(buf);

		if (opt < 1 || opt > 5)
		{
			invalid_input("Invalid choice", 0);
			return all_options(sd, type);
		}

		return customer_agent_task(sd, opt);
	}
	else
	{
		ask_admin_choice(buf);
		opt = atoi(buf);

		if (opt < 1 || opt > 12)
		{
			invalid_input("Invalid choice.", 0);
			return all_options(sd, type);
		}
		return admin_task(sd, opt);
	}
}
