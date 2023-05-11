#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "product.h"

#define PORT 8080

int connect_to_server(char *ip, int port)
{
    int sd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(ip);
    serv.sin_port = htons(port);

    connect(sd, (struct sockaddr *)&serv, sizeof(serv));

    return sd;
}

void generate_bill(struct product cart[])
{
    char filename[100];
    strcpy(filename, "bill_");
    char pid[10];
    sprintf(pid, "%d", getpid());
    strcat(filename, pid);
    strcat(filename, ".txt");
    int fd = open(filename, O_CREAT | O_RDWR, 0644), total = 0;
    write(fd, "P_ID\tP_Name\tCost\tQuantity\n", strlen("P_ID\tP_Name\tCost\tQuantity\n") * sizeof(char));
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        if (cart[i].P_ID != -1)
        {
            char buffer[1024];
            sprintf(buffer, "%d\t%s\t%d\t%d\n", cart[i].P_ID, cart[i].P_Name, cart[i].cost, cart[i].quantity);
            write(fd, buffer, strlen(buffer) * sizeof(char));
            total += cart[i].cost * cart[i].quantity;
        }
    }
    write(fd, "Total: ", strlen("Total: ") * sizeof(char));
    char total_buffer[10];
    sprintf(total_buffer, "%d", total);
    write(fd, total_buffer, strlen(total_buffer) * sizeof(char));
    close(fd);
    printf("Bill generated\n");
}

int main()
{
    int sd = connect_to_server("127.0.0.1", PORT);
    struct product products[MAX_PRODUCTS];
    if (sd < 0)
    {
        printf("Connection failed\n");
        return 1;
    }
    int pid = getpid();
    int product_id, quantity, flag;
    write(sd, &pid, sizeof(pid));
    read(sd, products, sizeof(products));
    printf("Connected to server\n");
    while (1)
    {
        printf("\nChoose an option:\n");
        printf("1. Show products\n");
        printf("2. Display cart\n");
        printf("3. Add to cart\n");
        printf("4. Edit cart\n");
        printf("5. Payment window\n");
        printf("6. Exit\n");
        int choice;
        char buffer[1024];
        struct product temp;
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            strcpy(temp.P_Name, "show");
            write(sd, &temp, sizeof(temp));
            read(sd, products, sizeof(products));
            printf("P_ID\tP_Name\tCost\tQuantity\n");
            for (int i = 0; i < MAX_PRODUCTS; i++)
            {
                if (products[i].P_ID != -1)
                {
                    printf("%d\t%s\t%d\t%d\n", products[i].P_ID, products[i].P_Name, products[i].cost, products[i].quantity);
                }
            }
            break;
        case 2:
            strcpy(temp.P_Name, "cart");
            write(sd, &temp, sizeof(temp));
            read(sd, products, sizeof(products));
            printf("P_ID\tP_Name\tCost\tQuantity\n");
            for (int i = 0; i < MAX_PRODUCTS; i++)
            {
                if (products[i].P_ID != -1)
                {
                    printf("%d\t%s\t%d\t%d\n", products[i].P_ID, products[i].P_Name, products[i].cost, products[i].quantity);
                }
            }
            break;
        case 3:
            printf("Enter product id: ");

            scanf("%d", &product_id);
            printf("Enter quantity: ");
            scanf("%d", &quantity);
            flag = 0;
            strcpy(temp.P_Name, "show");
            write(sd, &temp, sizeof(temp));
            read(sd, products, sizeof(products));
            for (int i = 0; i < MAX_PRODUCTS; i++)
            {
                if (products[i].P_ID == product_id)
                {
                    if (products[i].quantity < quantity)
                    {
                        printf("Insufficient quantity\n");
                        flag = 1;
                    }
                    break;
                }
                if (i == MAX_PRODUCTS - 1 && flag == 0)
                {
                    flag = 1;
                    printf("Invalid product id\n");
                }
            }
            if (flag)
                break;
            strcpy(temp.P_Name, "add");
            temp.P_ID = product_id;
            temp.quantity = quantity;
            write(sd, &temp, sizeof(temp));
            read(sd, buffer, sizeof(buffer));
            if (strcmp(buffer, "success") == 0)
            {
                printf("Items added to the cart\n");
            }
            else
            {
                printf("Item already in cart\n");
            }
            break;
        case 4:
            printf("Enter product id: ");
            scanf("%d", &product_id);
            printf("Enter quantity: ");
            scanf("%d", &quantity);
            flag = 0;
            strcpy(temp.P_Name, "show");
            write(sd, &temp, sizeof(temp));
            read(sd, products, sizeof(products));
            for (int i = 0; i < MAX_PRODUCTS; i++)
            {
                if (products[i].P_ID == product_id)
                {
                    if (products[i].quantity < quantity)
                    {
                        printf("Insufficient quantity\n");
                        flag = 1;
                    }
                    break;
                }
                if (i == MAX_PRODUCTS - 1 && flag == 0)
                {
                    flag = 1;
                    printf("Invalid product id\n");
                }
            }
            strcpy(temp.P_Name, "cart");
            write(sd, &temp, sizeof(temp));
            read(sd, products, sizeof(products));
            for (int i = 0; i < MAX_PRODUCTS; i++)
            {
                if (products[i].P_ID == product_id)
                {
                    break;
                }
                if (i == MAX_PRODUCTS - 1 && flag == 0)
                {
                    flag = 1;
                    printf("Product not in cart\n");
                }
            }
            if (flag)
                break;
            strcpy(temp.P_Name, "edit");
            temp.P_ID = product_id;
            temp.quantity = quantity;
            write(sd, &temp, sizeof(temp));
            read(sd, buffer, sizeof(buffer));
            if (strcmp(buffer, "success") == 0)
            {
                printf("Cart items have been edited\n");
            }
            else
            {
                printf("Error\n");
            }
            break;
        case 5:
            strcpy(temp.P_Name, "cart");
            write(sd, &temp, sizeof(temp));
            read(sd, products, sizeof(products));
            int total = 0, amount;
            printf("P_ID\tP_Name\tCost\tQuantity\n");
            for (int i = 0; i < MAX_PRODUCTS; i++)
            {
                if (products[i].P_ID != -1)
                {
                    printf("%d\t%s\t%d\t%d\n", products[i].P_ID, products[i].P_Name, products[i].cost, products[i].quantity);
                    total += products[i].cost * products[i].quantity;
                }
            }
            if (total == 0)
            {
                printf("Cart is empty\n");
            }
            else
            {
                printf("Checking out...\n");
                strcpy(temp.P_Name, "lockcart");
                write(sd, &temp, sizeof(temp));
                read(sd, buffer, sizeof(buffer));
                if (strcmp(buffer, "success") == 0)
                {
                    printf("Total amount: %d\n", total);
                    printf("Enter amount: ");
                    scanf("%d", &amount);
                    if (amount != total)
                    {
                        printf("Incorrect amount\n");
                        strcpy(temp.P_Name, "unlockcart");
                        write(sd, &temp, sizeof(temp));
                    }
                    else
                    {
                        strcpy(temp.P_Name, "pay");
                        write(sd, &temp, sizeof(temp));
                        read(sd, buffer, sizeof(buffer));
                        if (strcmp(buffer, "success") == 0)
                        {
                            printf("Payment successful\n");
                            generate_bill(products);
                        }
                        else
                        {
                            printf("Payment failed\n");
                        }
                    }
                }
                else
                {
                    printf("Check out failed\n");
                }
            }
            break;
        case 6:
            strcpy(temp.P_Name, "exit");
            write(sd, &temp, sizeof(temp));
            close(sd);
            return 0;
            break;
        default:
            printf("Invalid choice\n");
            break;
        }
    }
}