#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "product.h"
#define PORT 8080

int setup_server(int port, int *addlen)
{
    int sd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv, cli;
    *addlen = sizeof(cli);

    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(port);
    bind(sd, (struct sockaddr *)&serv, sizeof(serv));

    listen(sd, 5);

    return sd;
}

void clear_cart(struct product cart[MAX_PRODUCTS])
{
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        cart[i].P_ID = -1; // -1 indicates empty cart[i]
    }
}

int main()
{
    int sd, addlen;
    struct sockaddr_in cli;
    sd = setup_server(PORT, &addlen);
    printf("Server started\n");
    struct product products[MAX_PRODUCTS];
    if (!fork())
    {
        printf("Welcome, admin!\n");
        while (1)
        {
            printf("\nChoose an option:\n");
            printf("1. Add product\n");
            printf("2. Remove product\n");
            printf("3. Show products\n");
            printf("4. Update price/quantity\n");
            printf("5. Exit\n");
            int choice;
            struct product p;
            scanf("%d", &choice);
            switch (choice)
            {
            case 1:
                add_product();
                break;
            case 2:
                del_product();
                break;
            case 3:
                show_products();
                break;
            case 4:
                update_product();
                break;
            case 5:
                printf("Exiting...\n");
                kill(getppid(), SIGKILL);
                printf("Server down\n");
                return 0;
            default:
                printf("Invalid choice\n");
                break;
            }
        }
    }
    else
    {
        while (1)
        {
            printf("Waiting for connection...\n");
            int nsd = accept(sd, (struct sockaddr *)&cli, &addlen);
            if (!fork())
            {
                close(sd);
                int cli_pid;
                struct product cart[MAX_PRODUCTS];
                clear_cart(cart);
                read(nsd, &cli_pid, sizeof(cli_pid));
                write(nsd, products, sizeof(products));
                printf("Connection established with : %d\n", cli_pid);
                while (1)
                {
                    char buffer[1024];
                    struct product temp;
                    read(nsd, &temp, sizeof(temp));
                    if (strcmp(temp.P_Name, "show") == 0)
                    {
                        struct product products[100];
                        read_from_file(products, sizeof(products));
                        write(nsd, products, sizeof(products));
                    }
                    else if (strcmp(temp.P_Name, "cart") == 0)
                    {
                        write(nsd, cart, sizeof(cart));
                    }
                    else if (strcmp(temp.P_Name, "add") == 0)
                    {
                        get_name(temp.P_Name, temp.P_ID);
                        temp.cost = get_cost(temp.P_ID);
                        int flag = 0;
                        for (int i = 0; i < MAX_PRODUCTS; i++)
                        {
                            if (cart[i].P_ID == temp.P_ID)
                            {
                                flag = 1;
                            }
                        }
                        if (flag)
                        {
                            strcpy(buffer, "fail");
                            write(nsd, buffer, sizeof(buffer));
                        }
                        else
                        {
                            for (int i = 0; i < MAX_PRODUCTS; i++)
                            {
                                if (cart[i].P_ID == -1)
                                {
                                    cart[i] = temp;
                                    break;
                                }
                            }
                            strcpy(buffer, "success");
                            write(nsd, buffer, sizeof(buffer));
                        }
                    }
                    else if (strcmp(temp.P_Name, "edit") == 0)
                    {
                        get_name(temp.P_Name, temp.P_ID);
                        temp.cost = get_cost(temp.P_ID);
                        int flag = 0;
                        for (int i = 0; i < MAX_PRODUCTS; i++)
                        {
                            if (cart[i].P_ID == temp.P_ID)
                            {
                                if (temp.quantity == 0)
                                {
                                    temp.P_ID = -1;
                                }
                                cart[i] = temp;
                                break;
                            }
                        }
                        strcpy(buffer, "success");
                        write(nsd, buffer, sizeof(buffer));
                    }
                    else if (strcmp(temp.P_Name, "pay") == 0)
                    {
                        int flag = 0;
                        for (int i = 0; i < MAX_PRODUCTS; i++)
                        {
                            if (cart[i].P_ID != -1)
                            {
                                if (cart[i].quantity > set_buy_lock(cart[i].P_ID))
                                {
                                    flag = 1;
                                }
                            }
                        }
                        for (int i = 0; i < MAX_PRODUCTS; i++)
                        {
                            if (cart[i].P_ID != -1)
                            {
                                buy_product(cart[i], flag);
                            }
                        }
                        if (flag)
                        {
                            strcpy(buffer, "fail");
                            write(nsd, buffer, sizeof(buffer));
                        }
                        else
                        {
                            clear_cart(cart);
                            strcpy(buffer, "success");
                            write(nsd, buffer, sizeof(buffer));
                        }
                    }
                    else if (strcmp(temp.P_Name, "exit") == 0)
                    {
                        printf("Closing connection with %d...\n", cli_pid);
                        close(nsd);
                        exit(0);
                    }
                }
                exit(0);
            }
            else
            {
                close(nsd);
            }
        }
        exit(0);
    }
}