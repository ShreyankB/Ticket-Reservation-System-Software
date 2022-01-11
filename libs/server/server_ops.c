#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "../../constants/s_constants.h"
#include "../../header/server/server_ops.h"

void start(int sd)
{
	int choice;

	read(sd, &choice, sizeof(int));
	printf("Client connected...\n");

	while (1)
	{
		switch (choice)
		{
		case LOGIN:
			login(sd);
			read(sd, &choice, sizeof(int));
			break;
		case SIGNUP:
			signup(sd);
			read(sd, &choice, sizeof(int));
			break;
		default:
			return;
		}
	}

	close(sd);
	printf("Client disconnected\n");
}

int login(int sd)
{
	int account_type, id, fd, valid = 1, invalid = 0, login_success = 0;
	char password[PASS_LENGTH];
	struct account temp;
	struct flock lock;

	read(sd, &account_type, sizeof(account_type));
	read(sd, &id, sizeof(id));
	memset(password, 0, PASS_LENGTH);
	read(sd, &password, sizeof(password));

	if ((fd = open(db_path[account_type - 1], O_RDWR)) == -1)
		printf("File Error\n");

	lock.l_start = (id - 1) * sizeof(struct account);
	lock.l_len = sizeof(struct account);
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	if (account_type == 1)
	{
		lock.l_type = F_WRLCK;
		fcntl(fd, F_SETLKW, &lock);
		lseek(fd, (id - 1) * sizeof(struct account), SEEK_CUR);
		read(fd, &temp, sizeof(struct account));

		if (temp.id == id)
		{
			if (!strcmp(temp.pass, password))
			{
				write(sd, &valid, sizeof(valid));
				while (-1 != client_agent_options(sd, temp.id, account_type))
					;
				login_success = 1;
			}
		}

		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
		close(fd);

		if (login_success)
			return 3;
	}
	else if (account_type == 2)
	{
		lock.l_type = F_RDLCK;
		fcntl(fd, F_SETLKW, &lock);
		lseek(fd, (id - 1) * sizeof(struct account), SEEK_CUR);
		read(fd, &temp, sizeof(struct account));

		if (temp.id == id)
		{
			if (!strcmp(temp.pass, password))
			{
				write(sd, &valid, sizeof(valid));
				while (-1 != client_agent_options(sd, temp.id, account_type))
					;
				login_success = 1;
			}
		}

		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
		close(fd);
		if (login_success)
			return 3;
	}
	else if (account_type == 3)
	{
		lock.l_type = F_WRLCK;
		fcntl(fd, F_SETLKW, &lock);
		lseek(fd, (id - 1) * sizeof(struct account), SEEK_CUR);
		read(fd, &temp, sizeof(struct account));

		if (temp.id == id)
		{
			if (!strcmp(temp.pass, password))
			{
				write(sd, &valid, sizeof(valid));
				while (-1 != admin_options(sd, temp.id))
					;
				login_success = 1;
			}
		}

		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
		close(fd);
		if (login_success)
			return 3;
	}

	write(sd, &invalid, sizeof(invalid));
	return 3;
}

int signup(int sd)
{
	int account_type, fd;
	struct account temp;
	struct flock lock;

	read(sd, &account_type, sizeof(account_type));
	read(sd, &temp.name, sizeof(temp.name));
	read(sd, &temp.pass, sizeof(temp.pass));

	if ((fd = open(db_path[account_type - 1], O_RDWR)) == -1)
		printf("File Error\n");

	lock.l_type = F_WRLCK;
	lock.l_len = sizeof(struct account);
	lock.l_start = 0;
	lock.l_whence = SEEK_END;
	lock.l_pid = getpid();

	fcntl(fd, F_SETLKW, &lock);

	int fp = lseek(fd, 0, SEEK_END);

	temp.id = fp / sizeof(struct account) + 1;

	write(fd, &temp, sizeof(temp));
	write(sd, &temp.id, sizeof(temp.id));

	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);

	close(fd);
	return 3;
}

//client agent ops
int client_agent_options(int sd, int id, int account_type)
{
	int task;
	read(sd, &task, sizeof(task));

	switch (task)
	{
	case BOOK:
		return book_ticket(sd, task, account_type, id);
	case VIEW:
		view_booking(sd, id, account_type);
		write(sd, &task, sizeof(task));
		return 2;
	case UPDATE:
		update_booking(sd, task, account_type, id);
		return 3;
		break;
	case CANCEL:
		cancel_booking(sd, task, account_type, id);
		return 4;
		break;
	case LOG_OUT:
		write(sd, &task, sizeof(task));
		return -1;
		break;
	}
	return 0;
}

int book_ticket(int sd, int task, int account_type, int id)
{
	int fd = open(TRAIN_DB, O_RDWR);
	int trainid, seats;
	struct train temp;
	struct flock lock;

	lock.l_type = F_RDLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	fcntl(fd, F_SETLKW, &lock);

	int fp = lseek(fd, 0, SEEK_END);
	int no_of_trains = fp / sizeof(struct train);
	write(sd, &no_of_trains, sizeof(int));
	lseek(fd, 0, SEEK_SET);

	//print all trains
	while (fp != lseek(fd, 0, SEEK_CUR))
	{
		read(fd, &temp, sizeof(struct train));
		write(sd, &temp.tid, sizeof(int));
		write(sd, &temp.train_no, sizeof(int));
		write(sd, &temp.av_seats, sizeof(int));
		write(sd, &temp.train_name, sizeof(temp.train_name));
	}

	read(sd, &trainid, sizeof(trainid));
	if (trainid >= no_of_trains || trainid < 0)
	{
		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
		task = -1;
		write(sd, &task, sizeof(task));
		write(sd, &task, sizeof(task));
		return 1;
	}

	lseek(fd, 0, SEEK_SET);
	lseek(fd, trainid * sizeof(struct train), SEEK_CUR);
	read(fd, &temp, sizeof(struct train));

	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);

	if (!strcmp(temp.train_name, "deleted"))
	{
		task = -1;
		write(sd, &task, sizeof(task));
		write(sd, &task, sizeof(task));
		return 1;
	}

	write(sd, &temp.av_seats, sizeof(int));
	read(sd, &seats, sizeof(seats));

	lock.l_start = trainid * sizeof(struct train);
	lock.l_len = sizeof(struct train);
	lock.l_type = F_WRLCK;
	fcntl(fd, F_SETLKW, &lock);

	lseek(fd, 0, SEEK_SET);
	lseek(fd, trainid * sizeof(struct train), SEEK_CUR);
	read(fd, &temp, sizeof(struct train));

	int avseats = temp.av_seats;

	if (seats > 0 && avseats >= seats)
	{
		temp.av_seats -= seats;
		int fd2 = open(BOOKING_DB, O_RDWR);
		struct flock lock1 = lock;
		lock1.l_start = 0;
		lock1.l_len = 0;
		fcntl(fd2, F_SETLKW, &lock1);
		struct bookings bk;
		int fp2 = lseek(fd2, 0, SEEK_END);
		if (fp2 > 0)
		{
			lseek(fd2, -1 * sizeof(struct bookings), SEEK_CUR);
			read(fd2, &bk, sizeof(struct bookings));
			bk.bid++;
		}
		else
			bk.bid = 0;
		bk.account_type = account_type;
		bk.acc_no = id;
		bk.tr_id = trainid;
		bk.cancelled = 0;
		strcpy(bk.trainname, temp.train_name);
		bk.seat_start = temp.last_seatno_used + 1;
		bk.seat_end = temp.last_seatno_used + seats;
		temp.last_seatno_used = bk.seat_end;
		write(fd2, &bk, sizeof(bk));
		lock.l_type = F_UNLCK;
		fcntl(fd2, F_SETLK, &lock);
		close(fd2);
	}
	lseek(fd, -1 * sizeof(struct train), SEEK_CUR);
	write(fd, &temp, sizeof(temp));
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);
	if (seats <= 0 || avseats < seats)
		task = -1;
	write(sd, &task, sizeof(task));
	return 1;
}

void view_booking(int sd, int id, int account_type)
{
	int fd = open(BOOKING_DB, O_RDONLY);
	struct flock lock;
	lock.l_type = F_RDLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	fcntl(fd, F_SETLKW, &lock);

	int fp = lseek(fd, 0, SEEK_END);
	int entries = 0;
	if (fp == 0)
		write(sd, &entries, sizeof(entries));
	else
	{
		struct bookings bk[10];
		while (fp > 0 && entries < 10)
		{
			struct bookings temp;
			fp = lseek(fd, -1 * sizeof(struct bookings), SEEK_CUR);
			read(fd, &temp, sizeof(struct bookings));
			if (temp.acc_no == id && temp.account_type == account_type)
				bk[entries++] = temp;
			fp = lseek(fd, -1 * sizeof(struct bookings), SEEK_CUR);
		}
		write(sd, &entries, sizeof(entries));
		for (fp = 0; fp < entries; fp++)
		{
			write(sd, &bk[fp].bid, sizeof(bk[fp].bid));
			write(sd, &bk[fp].trainname, sizeof(bk[fp].trainname));
			write(sd, &bk[fp].seat_start, sizeof(int));
			write(sd, &bk[fp].seat_end, sizeof(int));
			write(sd, &bk[fp].cancelled, sizeof(int));
		}
	}
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);
}

void update_booking(int sd, int task, int account_type, int id)
{
	view_booking(sd, id, account_type);

	int fd1 = open(TRAIN_DB, O_RDWR);
	int fd2 = open(BOOKING_DB, O_RDWR);
	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	fcntl(fd1, F_SETLKW, &lock);
	fcntl(fd2, F_SETLKW, &lock);

	int val;
	struct train temp1;
	struct bookings temp2;
	read(sd, &val, sizeof(int));

	lseek(fd2, 0, SEEK_SET);
	lseek(fd2, val * sizeof(struct bookings), SEEK_CUR);
	read(fd2, &temp2, sizeof(temp2));
	lseek(fd2, -1 * sizeof(struct bookings), SEEK_CUR);
	printf("%d %s %d\n", temp2.tr_id, temp2.trainname, temp2.seat_end);

	lseek(fd1, 0, SEEK_SET);
	lseek(fd1, (temp2.tr_id) * sizeof(struct train), SEEK_CUR);
	read(fd1, &temp1, sizeof(temp1));
	lseek(fd1, -1 * sizeof(struct train), SEEK_CUR);
	printf("%d %s %d\n", temp1.tid, temp1.train_name, temp1.av_seats);

	read(sd, &val, sizeof(int));

	if (val == 1)
	{
		read(sd, &val, sizeof(int));
		if (temp1.av_seats >= val)
		{
			temp2.cancelled = 1;
			int booked = temp2.seat_end - temp2.seat_start + 1;
			write(fd2, &temp2, sizeof(temp2));

			int tot_seats = booked + val;

			printf("Tot_seats : %d\n", tot_seats);

			struct bookings bk;

			int fp2 = lseek(fd2, 0, SEEK_END);
			lseek(fd2, -1 * sizeof(struct bookings), SEEK_CUR);
			read(fd2, &bk, sizeof(struct bookings));

			bk.bid++;
			bk.account_type = temp2.account_type;
			bk.acc_no = temp2.acc_no;
			bk.tr_id = temp2.tr_id;
			bk.cancelled = 0;
			strcpy(bk.trainname, temp2.trainname);
			bk.seat_start = temp1.last_seatno_used + 1;
			bk.seat_end = temp1.last_seatno_used + tot_seats;

			temp1.av_seats -= val;

			printf("Available seats : %d\n", temp1.av_seats);

			temp1.last_seatno_used = bk.seat_end;

			printf("Last ticket : %d\n", temp1.last_seatno_used);

			write(fd2, &bk, sizeof(bk));
			write(fd1, &temp1, sizeof(temp1));
		}
		else
		{
			task = -2;
			write(sd, &task, sizeof(task));
		}
	}
	else
	{
		read(sd, &val, sizeof(int));
		if (temp2.seat_end - val < temp2.seat_start)
		{
			temp2.cancelled = 1;
			temp1.av_seats += val;
		}
		else
		{
			temp2.seat_end -= val;
			temp1.av_seats += val;
		}
		write(fd2, &temp2, sizeof(temp2));
		write(fd1, &temp1, sizeof(temp1));
	}
	lock.l_type = F_UNLCK;
	fcntl(fd1, F_SETLK, &lock);
	fcntl(fd2, F_SETLK, &lock);
	close(fd1);
	close(fd2);
	if (task > 0)
		write(sd, &task, sizeof(task));
}

void cancel_booking(int sd, int task, int account_type, int id)
{
	int val;
	view_booking(sd, id, account_type);
	read(sd, &val, sizeof(int));
	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = val * sizeof(struct bookings);
	lock.l_len = sizeof(struct bookings);
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();
	int train_id, no_of_seats;
	int fd = open(BOOKING_DB, O_RDWR);
	struct bookings bk;

	fcntl(fd, F_SETLKW, &lock);
	lseek(fd, lock.l_start, lock.l_whence);
	read(fd, &bk, sizeof(bk));
	lseek(fd, -1 * sizeof(bk), SEEK_CUR);
	bk.cancelled = 1;
	train_id = bk.tr_id;
	no_of_seats = bk.seat_end - bk.seat_start + 1;
	write(fd, &bk, sizeof(bk));
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);

	fd = open(TRAIN_DB, O_RDWR);
	lock.l_start = train_id * sizeof(struct train);
	lock.l_len = sizeof(struct train);
	lock.l_type = F_WRLCK;
	fcntl(fd, F_SETLKW, &lock);

	struct train tr;
	lseek(fd, lock.l_start, lock.l_whence);
	read(fd, &tr, sizeof(tr));
	tr.av_seats += no_of_seats;
	lseek(fd, -1 * sizeof(tr), SEEK_CUR);
	write(fd, &tr, sizeof(tr));

	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);

	write(sd, &task, sizeof(task));
}

// admin operations
void add_train(int sd, int task)
{
	int tid = 0, tno;
	char tname[20];
	struct train temp, temp2;

	read(sd, &temp.train_name, sizeof(temp.train_name));
	read(sd, &temp.train_no, sizeof(temp.train_no));

	temp.tid = tid;
	temp.av_seats = AVAILABLE_SEATS;
	temp.total = AVAILABLE_SEATS;
	temp.last_seatno_used = 0;

	int fd = open(TRAIN_DB, O_RDWR);
	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	fcntl(fd, F_SETLKW, &lock);

	int fp = lseek(fd, 0, SEEK_END);
	if (fp == 0)
	{
		write(fd, &temp, sizeof(temp));
		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
		close(fd);
		write(sd, &task, sizeof(task));
	}
	else
	{
		lseek(fd, -1 * sizeof(struct train), SEEK_CUR);
		read(fd, &temp2, sizeof(temp2));
		temp.tid = temp2.tid + 1;
		write(fd, &temp, sizeof(temp));
		write(sd, &task, sizeof(task));
		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
		close(fd);
	}
}

void delete_train(int sd)
{
	int fd = open(TRAIN_DB, O_RDWR);

	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	fcntl(fd, F_SETLKW, &lock);

	int fp = lseek(fd, 0, SEEK_END);
	int no_of_trains = fp / sizeof(struct train);
	write(sd, &no_of_trains, sizeof(int));
	lseek(fd, 0, SEEK_SET);
	while (fp != lseek(fd, 0, SEEK_CUR))
	{
		struct train temp;
		read(fd, &temp, sizeof(struct train));
		write(sd, &temp.tid, sizeof(int));
		write(sd, &temp.train_name, sizeof(temp.train_name));
		write(sd, &temp.train_no, sizeof(int));
	}

	read(sd, &no_of_trains, sizeof(int));

	if (no_of_trains == 0)
		write(sd, &no_of_trains, sizeof(int));
	else
	{
		struct train temp;
		lseek(fd, 0, SEEK_SET);
		lseek(fd, (no_of_trains - 1) * sizeof(struct train), SEEK_CUR);
		read(fd, &temp, sizeof(struct train));
		if (temp.total == temp.av_seats)
		{
			printf("%s is deleted\n", temp.train_name);
			strcpy(temp.train_name, "deleted");
			temp.av_seats = 0;
			lseek(fd, -1 * sizeof(struct train), SEEK_CUR);
			write(fd, &temp, sizeof(struct train));
			write(sd, &no_of_trains, sizeof(int));
		}
		else
		{
			no_of_trains = -5;
			write(sd, &no_of_trains, sizeof(int));
		}
	}

	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);
}

void modify_train(int sd)
{
	int fd = open(TRAIN_DB, O_RDWR);

	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	fcntl(fd, F_SETLKW, &lock);

	int fp = lseek(fd, 0, SEEK_END);
	int no_of_trains = fp / sizeof(struct train);
	write(sd, &no_of_trains, sizeof(int));
	lseek(fd, 0, SEEK_SET);
	while (fp != lseek(fd, 0, SEEK_CUR))
	{
		struct train temp;
		read(fd, &temp, sizeof(struct train));
		write(sd, &temp.tid, sizeof(int));
		write(sd, &temp.train_name, sizeof(temp.train_name));
		write(sd, &temp.train_no, sizeof(int));
	}
	read(sd, &no_of_trains, sizeof(int));

	struct train temp;
	lseek(fd, 0, SEEK_SET);
	lseek(fd, (no_of_trains - 1) * sizeof(struct train), SEEK_CUR);
	read(fd, &temp, sizeof(struct train));

	read(sd, &no_of_trains, sizeof(int));
	if (no_of_trains == 1)
	{
		write(sd, &temp.train_no, sizeof(temp.train_no));
		read(sd, &temp.train_no, sizeof(temp.train_no));
	}
	else
	{
		write(sd, &temp.av_seats, sizeof(temp.av_seats));
		int current_av = temp.av_seats;
		read(sd, &temp.av_seats, sizeof(temp.av_seats));
		temp.total = temp.total - current_av + temp.av_seats;
	}

	no_of_trains = 3;
	lseek(fd, -1 * sizeof(struct train), SEEK_CUR);
	write(fd, &temp, sizeof(struct train));
	write(sd, &no_of_trains, sizeof(int));

	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);
}

void add_user(int sd, int task)
{
	int account_type = 3, fd, acc_no = 0;
	char password[PASS_LENGTH], name[10];
	struct account temp;
	read(sd, &account_type, sizeof(account_type));
	read(sd, &name, sizeof(name));
	read(sd, &password, sizeof(password));

	if ((fd = open(db_path[account_type - 1], O_RDWR)) == -1)
		printf("File Error\n");
	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_len = sizeof(struct account);
	lock.l_whence = SEEK_END;
	lock.l_pid = getpid();

	fcntl(fd, F_SETLKW, &lock);
	int fp = lseek(fd, 0, SEEK_END);
	temp.id = fp / sizeof(struct account) + 1;
	strcpy(temp.name, name);
	strcpy(temp.pass, password);
	write(fd, &temp, sizeof(temp));
	write(sd, &temp.id, sizeof(temp.id));
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);

	close(fd);
	task = 4;
	write(sd, &task, sizeof(task));
}

void delete_user(int sd, int task)
{
	int account_type, id;
	struct account var;
	read(sd, &account_type, sizeof(account_type));

	int fd = open(db_path[account_type - 1], O_RDWR);
	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;
	lock.l_pid = getpid();

	fcntl(fd, F_SETLKW, &lock);

	int fp = lseek(fd, 0, SEEK_END);
	int users = fp / sizeof(struct account);
	write(sd, &users, sizeof(int));

	lseek(fd, 0, SEEK_SET);
	while (fp != lseek(fd, 0, SEEK_CUR))
	{
		read(fd, &var, sizeof(struct account));
		write(sd, &var.id, sizeof(var.id));
		write(sd, &var.name, sizeof(var.name));
	}
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);

	read(sd, &id, sizeof(id));
	if (id == 0)
	{
		write(sd, &task, sizeof(task));
	}
	else
	{
		lock.l_start = (id - 1) * sizeof(struct account);
		lock.l_len = sizeof(struct account);
		lock.l_type = F_WRLCK;
		fcntl(fd, F_SETLKW, &lock);
		lseek(fd, 0, SEEK_SET);
		lseek(fd, (id - 1) * sizeof(struct account), SEEK_CUR);
		read(fd, &var, sizeof(struct account));
		lseek(fd, -1 * sizeof(struct account), SEEK_CUR);
		strcpy(var.name, "deleted");
		strcpy(var.pass, "");
		write(fd, &var, sizeof(struct account));
		write(sd, &task, sizeof(task));
	}

	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);

	close(fd);
}

void modify_user(int sd, int task)
{
	int account_type, id;
	struct account var;
	char updated_name[10], updated_pass[PASS_LENGTH];
	read(sd, &account_type, sizeof(account_type));

	int fd = open(db_path[account_type - 1], O_RDWR);
	struct flock lock;
	lock.l_type = F_RDLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;
	lock.l_pid = getpid();

	fcntl(fd, F_SETLKW, &lock);

	int fp = lseek(fd, 0, SEEK_END);
	int users = fp / sizeof(struct account);
	write(sd, &users, sizeof(int));

	lseek(fd, 0, SEEK_SET);
	while (fp > lseek(fd, 0, SEEK_CUR))
	{
		read(fd, &var, sizeof(struct account));
		write(sd, &var.id, sizeof(var.id));
		write(sd, &var.name, sizeof(var.name));
	}
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);

	lock.l_type = F_WRLCK;
	read(sd, &id, sizeof(id));

	lock.l_start = (id - 1) * sizeof(struct account);
	lock.l_len = sizeof(struct account);
	fcntl(fd, F_SETLKW, &lock);
	lseek(fd, lock.l_start, SEEK_SET);
	read(fd, &var, sizeof(var));
	write(sd, &var.name, sizeof(var.name));
	write(sd, &var.pass, sizeof(var.pass));

	lseek(fd, lock.l_start, SEEK_SET);
	read(sd, &id, sizeof(id));

	if (id == 1)
	{
		read(sd, &updated_name, sizeof(updated_name));
		strcpy(var.name, updated_name);
	}
	else if (id == 2)
	{
		read(sd, &updated_pass, sizeof(updated_pass));
		strcpy(var.pass, updated_pass);
	}

	write(fd, &var, sizeof(var));
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);
	write(sd, &task, sizeof(task));
}

void view_all_users(int sd, int task)
{
	int fd1 = open(db_path[0], O_RDONLY);
	int fd2 = open(db_path[1], O_RDONLY);
	int fd3 = open(db_path[2], O_RDONLY);

	struct account arr1[100], arr2[100], arr3[100];
	int k1 = 0, k2 = 0, k3 = 0, fp;
	int n = sizeof(struct account);
	int str = sizeof(char) * 20;

	fp = lseek(fd1, 0, SEEK_END);
	lseek(fd1, 0, SEEK_SET);
	while (fp > lseek(fd1, 0, SEEK_CUR))
	{
		read(fd1, &arr1[k1++], n);
	}
	close(fd1);

	fp = lseek(fd2, 0, SEEK_END);
	lseek(fd2, 0, SEEK_SET);
	while (fp > lseek(fd2, 0, SEEK_CUR))
	{
		read(fd2, &arr2[k2++], n);
	}
	close(fd2);

	fp = lseek(fd3, 0, SEEK_END);
	lseek(fd3, 0, SEEK_SET);
	while (fp > lseek(fd3, 0, SEEK_CUR))
	{
		read(fd3, &arr3[k3++], n);
	}
	close(fd3);

	fp = k1 + k2 + k3;

	write(sd, &fp, sizeof(fp));
	fp = 1;
	while (k1--)
	{
		write(sd, &arr1[k1].id, sizeof(int));
		write(sd, &fp, sizeof(fp));
		write(sd, &arr1[k1].name, str / 2);
		write(sd, &arr1[k1].pass, str);
	}
	fp = 2;
	while (k2--)
	{
		write(sd, &arr2[k2].id, sizeof(int));
		write(sd, &fp, sizeof(fp));
		write(sd, &arr2[k2].name, str / 2);
		write(sd, &arr2[k2].pass, str);
	}
	fp = 3;
	while (k3--)
	{
		write(sd, &arr3[k3].id, sizeof(int));
		write(sd, &fp, sizeof(fp));
		write(sd, &arr3[k3].name, str / 2);
		write(sd, &arr3[k3].pass, str);
	}

	write(sd, &task, sizeof(task));
}

void view_all_trains(int sd, int task)
{
	int fp, entries, k = 0;
	struct train arr[100];

	int fd = open(TRAIN_DB, O_RDONLY);

	fp = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	while (fp > lseek(fd, 0, SEEK_CUR))
	{
		read(fd, &arr[k++], sizeof(struct train));
	}
	close(fd);

	write(sd, &k, sizeof(k));
	while (k--)
	{
		write(sd, &arr[k].tid, sizeof(int));
		write(sd, &arr[k].train_no, sizeof(int));
		write(sd, &arr[k].train_name, sizeof(char) * 20);
		write(sd, &arr[k].av_seats, sizeof(int));
	}

	write(sd, &task, sizeof(task));
}

void view_all_bookings(int sd, int task)
{
	int fd = open(BOOKING_DB, O_RDONLY);
	struct flock lock;
	lock.l_type = F_RDLCK;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	fcntl(fd, F_SETLKW, &lock);

	int fp = lseek(fd, 0, SEEK_END);
	int entries = 0;
	if (fp == 0)
		write(sd, &entries, sizeof(entries));
	else
	{
		struct bookings bk;
		entries = fp / sizeof(struct bookings);
		write(sd, &entries, sizeof(entries));
		lseek(fd, 0, SEEK_SET);
		while (fp > lseek(fd, 0, SEEK_CUR))
		{
			read(fd, &bk, sizeof(bk));
			write(sd, &bk.bid, sizeof(bk.bid));
			write(sd, &bk.acc_no, sizeof(bk.acc_no));
			write(sd, &bk.account_type, sizeof(bk.account_type));
			write(sd, &bk.trainname, sizeof(bk.trainname));
			write(sd, &bk.seat_start, sizeof(int));
			write(sd, &bk.seat_end, sizeof(int));
			write(sd, &bk.cancelled, sizeof(int));
		}
	}
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);
	write(sd, &task, sizeof(task));
}

void search_users(int sd, int task)
{
	int fd1 = open(db_path[0], O_RDONLY);
	int fd2 = open(db_path[1], O_RDONLY);
	int fd3 = open(db_path[2], O_RDONLY);

	int arr1[100], arr2[100], arr3[100];
	int k1 = 0, k2 = 0, k3 = 0, fp;

	char name[10];
	struct account temp;

	read(sd, &name, sizeof(name));

	fp = lseek(fd1, 0, SEEK_END);
	lseek(fd1, 0, SEEK_SET);
	while (fp > lseek(fd1, 0, SEEK_CUR))
	{
		read(fd1, &temp, sizeof(temp));
		if (!strcmp(temp.name, name))
			arr1[k1++] = temp.id;
	}
	close(fd1);

	fp = lseek(fd2, 0, SEEK_END);
	lseek(fd2, 0, SEEK_SET);
	while (fp > lseek(fd2, 0, SEEK_CUR))
	{
		read(fd2, &temp, sizeof(temp));
		if (!strcmp(temp.name, name))
			arr2[k2++] = temp.id;
	}
	close(fd2);

	fp = lseek(fd3, 0, SEEK_END);
	lseek(fd3, 0, SEEK_SET);
	while (fp > lseek(fd3, 0, SEEK_CUR))
	{
		read(fd3, &temp, sizeof(temp));
		if (!strcmp(temp.name, name))
			arr3[k3++] = temp.id;
	}
	close(fd3);

	fp = k1 + k2 + k3;
	write(sd, &fp, sizeof(fp));
	fp = 1;
	while (k1--)
	{
		write(sd, &arr1[k1], sizeof(int));
		write(sd, &fp, sizeof(fp));
	}
	fp = 2;
	while (k2--)
	{
		write(sd, &arr2[k2], sizeof(int));
		write(sd, &fp, sizeof(fp));
	}
	fp = 3;
	while (k3--)
	{
		write(sd, &arr3[k3], sizeof(int));
		write(sd, &fp, sizeof(fp));
	}

	write(sd, &task, sizeof(task));
}

void search_trains(int sd, int task)
{
	int fp, entries, arrid[100], tno[100], seats[100], k = 0;

	char name[20];
	struct train temp;

	read(sd, &name, sizeof(name));

	int fd = open(TRAIN_DB, O_RDONLY);

	fp = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	while (fp > lseek(fd, 0, SEEK_CUR))
	{
		read(fd, &temp, sizeof(temp));
		if (!strcmp(temp.train_name, name))
		{
			arrid[k] = temp.tid;
			seats[k] = temp.av_seats;
			tno[k++] = temp.train_no;
		}
	}
	close(fd);

	write(sd, &k, sizeof(k));
	while (k--)
	{
		write(sd, &arrid[k], sizeof(int));
		write(sd, &tno[k], sizeof(int));
		write(sd, &seats[k], sizeof(int));
	}

	write(sd, &task, sizeof(task));
}

int admin_options(int sd, int id)
{
	int task;
	read(sd, &task, sizeof(task));

	switch (task)
	{
	case ADD_TRAIN:
		add_train(sd, task);
		return task;
	case DELETE_TRAIN:
		delete_train(sd);
		return task;
	case MODIFY_TRAIN:
		modify_train(sd);
		return task;
	case ADD_USER:
		add_user(sd, task);
		return task;
	case DELETE_USER:
		delete_user(sd, task);
		return task;
	case MODIFY_USER:
		modify_user(sd, task);
		return task;
	case VIEW_ALL_USERS:
		view_all_users(sd, task);
		return task;
	case VIEW_ALL_TRAINS:
		view_all_trains(sd, task);
		return task;
	case VIEW_ALL_BOOKINGS:
		view_all_bookings(sd, task);
		return task;
	case SEARCH_USER:
		search_users(sd, task);
		return task;
	case SEARCH_TRAIN:
		search_trains(sd, task);
		return task;
	case LOGOUT:
		write(sd, &task, sizeof(task));
		return -1;
	}
}