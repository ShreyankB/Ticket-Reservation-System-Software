#ifndef S_CONSTANTS_H
#define S_CONSTANTS_H

#define PORT 9912
#define PASS_LENGTH 20
#define TRAIN_DB "./db/train"
#define BOOKING_DB "./db/booking"
#define LOGIN 1
#define SIGNUP 2

#endif

#ifndef ACCOUNT_PATH
#define ACCOUNT_PATH

char *db_path[3] = {"./db/accounts/customer", "./db/accounts/agent", "./db/accounts/admin"};

#endif

#ifndef ACCOUNT_STRUCT
#define ACCOUNT_STRUCT

struct account{
	int id;
	char name[10];
	char pass[PASS_LENGTH];
};

#endif

#ifndef TRAIN_STRUCT
#define TRAIN_STRUCT

struct train{
	int tid;
	char train_name[20];
	int train_no;
	int av_seats;
	int last_seatno_used;
	int total;
};

#endif

#ifndef BOOKINGS_STRUCT
#define BOOKINGS_STRUCT

struct bookings{
	int bid;
	int account_type;
	int acc_no;
	int tr_id;
	char trainname[20];
	int seat_start;
	int seat_end;
	int cancelled;
};

#endif

#define AVAILABLE_SEATS 20