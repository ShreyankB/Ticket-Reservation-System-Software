#define SEATS_OVERFLOW -2
#define CANCEL_SUCCESS 4

#define BOOK 1
#define VIEW 2
#define UPDATE 3
#define CANCEL 4
#define LOG_OUT 5

int customer_agent_task(int, int);
void view_booking(int);

void book_ticket(int, int);
void customer_print_all_trains(int);

void ask_update_info(int);
void update_booking(int, int);

void cancel_booking(int, int);