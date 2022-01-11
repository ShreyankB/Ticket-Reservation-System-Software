#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../../header/client/customer_ops.h"
#include "../../header/client/admin_ops.h"

#define PASS_LENGTH 20

void enter_to_continue()
{
	printf("Press [enter] key to continue...\n");
	while (getchar() != '\n')
		;
}

void customer_print_all_trains(int sock)
{
	int train_count, trainid, trainavseats, trainno;
	char trainname[20];

	read(sock, &train_count, sizeof(train_count)); //no of train_count

	system("clear");
	printf("id\ttrain_no\tseats available\ttrain name\n");

	printf("-------------------------------------------------\n");
	while (train_count--)
	{
		read(sock, &trainid, sizeof(trainid));
		read(sock, &trainno, sizeof(trainno));
		read(sock, &trainavseats, sizeof(trainavseats));
		read(sock, &trainname, sizeof(trainname));

		if (strcmp(trainname, "deleted") != 0)
			printf("%d\t%d\t\t%d\t\t%s\n", trainid, trainno, trainavseats, trainname);
	}
	printf("-------------------------------------------------\n\n");
}

void book_ticket(int sock, int opt)
{
	int trainid, trainavseats, required_seats = 0;

	customer_print_all_trains(sock);

	printf("ID\t: ");
	scanf("%d", &trainid);

	write(sock, &trainid, sizeof(trainid));
	read(sock, &trainavseats, sizeof(trainavseats));

	if (trainavseats > 0)
	{
		printf("Enter the number of seats: ");
		scanf("%d", &required_seats);
	}

	if (trainavseats >= required_seats && required_seats > 0)
		write(sock, &required_seats, sizeof(required_seats));
	else
	{
		required_seats = -1;
		write(sock, &required_seats, sizeof(required_seats));
	}

	read(sock, &opt, sizeof(opt));

	if (opt == 1)
		printf("Booking succesful...\n");
	else
		printf("Error while booking, please try again...\n");

	enter_to_continue();
}

void update_booking(int sock, int opt)
{
	int val;
	view_booking(sock);

	printf("Id\t: ");
	scanf("%d", &val);
	write(sock, &val, sizeof(int)); //Booking ID

	ask_update_info(sock);

	read(sock, &opt, sizeof(opt));

	if (opt == SEATS_OVERFLOW)
		printf("seats exceed...\n");
	else
		printf("booking updated...\n");

	enter_to_continue();
}

void view_booking(int sock)
{
	int entries;
	read(sock, &entries, sizeof(int));

	printf("id\ttickets\ttrain\n");
	printf("-----------------------------------\n");
	while (entries--)
	{
		int bid, bks_seat, bke_seat, cancelled, uid, utype;
		char trainname[20];
		read(sock, &bid, sizeof(bid));

		read(sock, &trainname, sizeof(trainname));
		read(sock, &bks_seat, sizeof(int));
		read(sock, &bke_seat, sizeof(int));
		read(sock, &cancelled, sizeof(int));

		if (!cancelled)
			printf("%d\t%d\t%s\n", bid, bke_seat - bks_seat, trainname);
	}
	printf("-------------------------------------------\n");
	printf("Press [Enter] key to continue...\n");
	while (getchar() != '\n')
		;
	getchar();
}

void ask_update_info(int sock)
{
	int choice = -1, seats;

	while (choice > 2 || choice < 1)
	{
		printf("\n1. book more tickets\n2. cancel some tickets\n:");
		scanf("%d", &choice);
	}
	write(sock, &choice, sizeof(int));

	printf("# seats: \t");
	scanf("%d", &seats);

	write(sock, &seats, sizeof(int));
}

void cancel_booking(int sock, int opt)
{
	int val;
	view_booking(sock);
	printf("Id\t: ");
	scanf("%d", &val);

	write(sock, &val, sizeof(int)); //Booking ID
	read(sock, &opt, sizeof(opt));

	if (opt == CANCEL_SUCCESS)
		printf("canceled booking %d.\n", val);
	else
		printf("error while cancelling, try again...\n");

	enter_to_continue();
}

int customer_agent_task(int sock, int opt)
{
	write(sock, &opt, sizeof(opt));

	switch (opt)
	{
	case BOOK:
	{
		book_ticket(sock, opt);
		return 1;
	}
	case VIEW:
	{
		view_booking(sock);
		read(sock, &opt, sizeof(opt));
		return 2;
	}
	case UPDATE:
	{
		//update bookings
		update_booking(sock, opt);
		return 3;
	}
	case CANCEL:
	{
		//cancel booking
		cancel_booking(sock, opt);
		return 4;
	}
	case LOG_OUT:
	{
		read(sock, &opt, sizeof(opt));

		if (opt == LOG_OUT)
			printf("Logged out successfully.\n");

		enter_to_continue();
		return -1;
		break;
	}
	default:
		return -1;
	}
}
