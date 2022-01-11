#define BOOK 1
#define VIEW 2
#define UPDATE 3
#define CANCEL 4
#define LOG_OUT 5

#define ADD_TRAIN 1
#define DELETE_TRAIN 2
#define MODIFY_TRAIN 3
#define ADD_USER 4
#define DELETE_USER 5
#define MODIFY_USER 6
#define VIEW_ALL_USERS 7
#define VIEW_ALL_TRAINS 8
#define VIEW_ALL_BOOKINGS 9
#define SEARCH_USER 10
#define SEARCH_TRAIN 11
#define LOGOUT 12

void start(int);
int login(int);
int signup(int);
int admin_options(int, int);
int client_agent_options(int, int, int);
void view_booking(int, int, int);
int book_ticket(int, int, int, int);
void view_booking2(int, int, int, int);
void update_booking(int, int, int, int);
void cancel_booking(int, int, int, int);
void add_train(int, int);
void delete_train(int);
void modify_train(int);
void add_user(int, int);
void delete_user(int, int);
void modify_user(int, int);
void view_all_users(int, int);
void view_all_trains(int, int);
void view_all_bookings(int, int);
void search_users(int, int);