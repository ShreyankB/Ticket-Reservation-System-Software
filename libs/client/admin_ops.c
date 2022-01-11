#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../../header/client/admin_ops.h"

#define PASS_LENGTH 20

int admin_task(int sd, int option)
{
    switch (option)
    {
    case ADD_TRAIN:
    {
        int tno;
        char tname[20];
        write(sd, &option, sizeof(option));
        printf("Train Name: ");
        scanf("%s", tname);
        printf("Train No. : ");
        scanf("%d", &tno);
        write(sd, &tname, sizeof(tname));
        write(sd, &tno, sizeof(tno));
        read(sd, &option, sizeof(option));
        if (option == 1)
            printf("Train Added Successfully...\n");
        enter_to_continue_();
        return option;
        break;
    }
    case DELETE_TRAIN:
    {
        int no_of_trains;
        write(sd, &option, sizeof(option));
        read(sd, &no_of_trains, sizeof(int));
        printf("id\ttrain no\ttrain name\n");
        printf("---------------------------------\n");
        while (no_of_trains > 0)
        {
            int tid, tno;
            char tname[20];
            read(sd, &tid, sizeof(tid));
            read(sd, &tname, sizeof(tname));
            read(sd, &tno, sizeof(tno));
            if (!strcmp(tname, "deleted"))
                ;
            else
                printf("%d\t%d\t\t%s\n", tid + 1, tno, tname);
            no_of_trains--;
        }
        printf("---------------------------------\n");
        printf("\nPress 0 to cancel.\nID to delete: ");
        scanf("%d", &no_of_trains);
        write(sd, &no_of_trains, sizeof(int));
        read(sd, &option, sizeof(option));
        if (option == no_of_trains && option)
            printf("Deleted successfully\n");
        while (getchar() != '\n')
            ;
        getchar();
        return option;
        break;
    }
    case MODIFY_TRAIN:
    {
        int no_of_trains;
        write(sd, &option, sizeof(option));
        read(sd, &no_of_trains, sizeof(int));

        printf("id\ttrain no\ttrain name\n");
        printf("---------------------------------\n");
        while (no_of_trains > 0)
        {
            int tid, tno;
            char tname[20];
            read(sd, &tid, sizeof(tid));
            read(sd, &tname, sizeof(tname));
            read(sd, &tno, sizeof(tno));
            if (!strcmp(tname, "deleted"))
                ;
            else
                printf("%d.\t%d\t\t%s\n", tid + 1, tno, tname);
            no_of_trains--;
        }
        printf("---------------------------------\n");

        printf("\nPress 0 to cancel.\n\ntrain ID to modify: ");
        scanf("%d", &no_of_trains);
        write(sd, &no_of_trains, sizeof(int));
        printf("::Parameter to modify\n1. Train No.\n2. Available Seats\n:");
        scanf("%d", &no_of_trains);
        write(sd, &no_of_trains, sizeof(int));
        read(sd, &no_of_trains, sizeof(int));
        printf("Current Value: %d\n", no_of_trains);
        printf("Enter Value: ");
        scanf("%d", &no_of_trains);
        write(sd, &no_of_trains, sizeof(int));

        read(sd, &option, sizeof(option));

        if (option == MODIFY_TRAIN)
            printf("Data Modified Successfully\n");
        while (getchar() != '\n')
            ;
        getchar();
        return option;
        break;
    }
    case ADD_USER:
    {
        write(sd, &option, sizeof(option));
        char pass[PASS_LENGTH], name[10], buf[100];
    jump:
        printf("\n1. User\n2.Agent\n3.Admin\n:");
        scanf("%s", buf);
        int type = atoi(buf);

        if (type < 1 || type > 3)
        {
            printf("Invalid Account Type. Try Again...");
            while (getchar() != '\n')
                ;
            getchar();
            goto jump;
        }
        write(sd, &type, sizeof(type));
        printf("Enter the name: ");
        scanf("%s", name);

        strcpy(pass, getpass("Enter a password"));

        write(sd, &name, sizeof(name));
        write(sd, &pass, sizeof(pass));
        read(sd, &option, sizeof(option));
        printf(" Account Number: %d\n", option);
        read(sd, &option, sizeof(option));
        if (option == 4)
            printf("Successfully created %s account\n", type == 1 ? "USER" : type == 2 ? "AGENT"
                                                                                       : "ADMIN");
        while (getchar() != '\n')
            ;
        getchar();
        return option;
        break;
    }
    case DELETE_USER:
    {
        int choice, users, id;
        write(sd, &option, sizeof(option));
        printf("1. Customer\n2. Agent\n3. Admin\n:");
        scanf("%d", &choice);
        write(sd, &choice, sizeof(choice));
        read(sd, &users, sizeof(users));
        while (users--)
        {
            char name[10];
            read(sd, &id, sizeof(id));
            read(sd, &name, sizeof(name));
            if (strcmp(name, "deleted") != 0)
                printf("%d\t%s\n", id, name);
        }
        printf("\nID:\t");
        scanf("%d", &id);
        write(sd, &id, sizeof(id));
        read(sd, &option, sizeof(option));
        if (option == 5)
            printf("Successfully deleted user\n");
        while (getchar() != '\n')
            ;
        getchar();
        return option;
    }
    case MODIFY_USER:
    {
        int choice, users, id;
        char name[10], pass[20], buf[100];
        write(sd, &option, sizeof(option));
    jump3:
        printf("1. Customer\n2. Agent\n3. Admin\n:");
        scanf("%s", buf);
        choice = atoi(buf);
        if (choice < 1 || choice > 3)
        {
            printf("Invalid Choice. Please try again...\n");
            while (getchar() != '\n')
                ;
            getchar();
            goto jump3;
        }
        write(sd, &choice, sizeof(choice));
        read(sd, &users, sizeof(users));
        while (users--)
        {
            read(sd, &id, sizeof(id));
            read(sd, &name, sizeof(name));
            if (strcmp(name, "deleted") != 0)
                printf("%d\t%s\n", id, name);
        }
    jump2:
        printf("\nID:\t");
        scanf("%s", buf);
        id = atoi(buf);
        if (id == 0)
        {
            printf("Invalid Account ID. Please try again...\n");
            while (getchar() != '\n')
                ;
            getchar();
            goto jump2;
        }
        write(sd, &id, sizeof(id));
        read(sd, &name, sizeof(name));
        read(sd, &pass, sizeof(pass));
        printf("What data do you want to update:\n");
        printf("1. Name\n2. Password\n:");
        scanf("%d", &id);
        if (id == 1)
        {
            printf("Current Name = %s\n", name);
            printf("New Name     = ");
            scanf("%s", name);
        }
        else if (id == 2)
        {
            printf("Current Password = %s\n", pass);
            printf("New Password     = ");
            scanf("%s", pass);
        }
        write(sd, &id, sizeof(id));
        if (id == 1)
            write(sd, &name, sizeof(name));
        if (id == 2)
            write(sd, &pass, sizeof(pass));

        read(sd, &option, sizeof(option));
        if (option == 6)
            printf("Updated Successfully\n");
        while (getchar() != '\n')
            ;
        getchar();
        return option;
        break;
    }
    case VIEW_ALL_USERS:
    {
        //view users
        write(sd, &option, sizeof(option));
        int entries;
        read(sd, &entries, sizeof(entries));

        if (entries <= 0)
            printf("No user found\n");
        else
            printf("id\t\ttype\t\tname\t\tpassword\n");
        printf("--------------------------------------------------------------------\n");
        while (entries--)
        {
            int id, type;
            char name[10], password[20];
            read(sd, &id, sizeof(id));
            read(sd, &type, sizeof(type));
            read(sd, &name, sizeof(name));
            read(sd, &password, sizeof(password));
            if (strcmp(name, "deleted") != 0)
                printf("%d\t\t%s\t\t%s\t\t%s\n", id, type == 1 ? "USER " : type == 2 ? "AGENT"
                                                                                     : "ADMIN",
                       name, password);
        }
        printf("--------------------------------------------------------------------\n");

        read(sd, &option, sizeof(option));
        while (getchar() != '\n')
            ;
        getchar();
        return option;
        break;
    }
    case VIEW_ALL_TRAINS:
    {
        //view trains
        write(sd, &option, sizeof(option));
        int entries;
        read(sd, &entries, sizeof(entries));

        if (entries <= 0)
            printf("No train found\n");
        else
            printf("id\ttrain no\t\t train name\t\tavailable seats\n");
        printf("--------------------------------------------------------------\n");
        while (entries--)
        {
            int id, tno, av_seats;
            char name[20];
            read(sd, &id, sizeof(id));
            read(sd, &tno, sizeof(tno));
            read(sd, &name, sizeof(name));
            read(sd, &av_seats, sizeof(av_seats));
            if (strcmp(name, "deleted") != 0)
                printf("%d\t%d\t\t%s\t\t%d\n", id, tno, name, av_seats);
        }
        printf("--------------------------------------------------------------\n");

        read(sd, &option, sizeof(option));
        while (getchar() != '\n')
            ;
        getchar();
        return option;
        break;
    }
    case VIEW_ALL_BOOKINGS:
    {
        //view bookings
        write(sd, &option, sizeof(option));
        view_booking_admin(sd);
        read(sd, &option, sizeof(option));
        return option;
        break;
    }
    case SEARCH_USER:
    {
        //search users
        write(sd, &option, sizeof(option));
        char name[10];
        int entries;
        printf("username:\t");
        scanf("%s", name);

        write(sd, &name, sizeof(name));

        read(sd, &entries, sizeof(entries));

        if (entries <= 0)
            printf("No user found\n");
        else
            while (entries--)
            {
                int id, type;
                read(sd, &id, sizeof(id));
                read(sd, &type, sizeof(type));
                printf("id: %d\taccount type: %d\n", id, type);
            }

        read(sd, &option, sizeof(option));
        while (getchar() != '\n')
            ;
        getchar();
        return option;
        break;
    }
    case SEARCH_TRAIN:
    {
        //search trains
        write(sd, &option, sizeof(option));

        char name[20];
        int entries;
        printf("train name:\t");
        scanf("%s", name);

        write(sd, &name, sizeof(name));

        read(sd, &entries, sizeof(entries));

        if (entries <= 0)
            printf("No train found\n");
        else
            printf("id\ttrain no\t\tavailable seats\n");
        printf("-------------------------------------------------\n");
        while (entries--)
        {
            int id, type, av_seats;
            read(sd, &id, sizeof(id));
            read(sd, &type, sizeof(type));
            read(sd, &av_seats, sizeof(av_seats));
            printf("%d\t%d\t\t%d\n", id, type, av_seats);
        }
        printf("-------------------------------------------------\n");
        read(sd, &option, sizeof(option));
        while (getchar() != '\n')
            ;
        getchar();
        return option;
        break;
    }
    case LOGOUT:
    {
        write(sd, &option, sizeof(option));
        read(sd, &option, sizeof(option));
        if (option == 12)
            printf("Logged out successfully.\n");
        while (getchar() != '\n')
            ;
        getchar();
        return -1;
        break;
    }
    default:
        return -1;
    }
}

void enter_to_continue_()
{
    printf("Press [enter] key to continue...\n");
    while (getchar() != '\n')
        ;
}

void view_booking_admin(int sd)
{
    int entries;
    read(sd, &entries, sizeof(int));

    printf("bookid\tseats\ttrain name\tusertype\tstatus\n");
    printf("--------------------------------------------------\n");

    while (entries--)
    {
        int bid, bks_seat, bke_seat, cancelled, uid, utype;
        char trainname[20];
        read(sd, &bid, sizeof(bid));
        
        read(sd, &uid, sizeof(uid));
        read(sd, &utype, sizeof(utype));
    
        read(sd, &trainname, sizeof(trainname));
        read(sd, &bks_seat, sizeof(int));
        read(sd, &bke_seat, sizeof(int));
        read(sd, &cancelled, sizeof(int));
        printf("%d\t%d\t%s\t%d\t%s\t%s\n", bid, bke_seat-bks_seat, trainname, uid, utype == 1 ? "USER" : "AGENT", cancelled == 0 ? "CONFIRMED" : "CANCELLED");
    }
    printf("--------------------------------------------------\n");

    printf("Press [Enter] key to continue...\n");
    while (getchar() != '\n')
        ;
    getchar();
}
