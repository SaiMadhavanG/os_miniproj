#include "product.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>

void create_datafile()
{
    int fd = open(DATAFILE, O_CREAT | O_RDWR, 0644);
    struct product products[MAX_PRODUCTS];
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        products[i].P_ID = -1;
        products[i].quantity = 0;
        products[i].cost = 0;
        strcpy(products[i].P_Name, "");
    }
    write(fd, products, sizeof(products));
    close(fd);
    return;
}

void read_from_file(struct product products[MAX_PRODUCTS], int size)
{
    int key = ftok(".", 'a');
    int semid = semget(key, MAX_PRODUCTS, 0);
    int fd = open(DATAFILE, O_RDONLY, 0);
    if (fd < 0)
    {
        create_datafile();
        fd = open(DATAFILE, O_RDONLY, 0);
    }
    struct sembuf sb;
    sb.sem_flg = 0;
    sb.sem_num = MAX_PRODUCTS;
    sb.sem_op = -1;
    semop(semid, &sb, 1);
    read(fd, products, size);
    sb.sem_op = 1;
    semop(semid, &sb, 1);
    close(fd);
}

void write_to_file(struct product products[MAX_PRODUCTS], int size)
{
    int fd = open(DATAFILE, O_WRONLY, 0);
    if (fd < 0)
    {
        create_datafile();
        fd = open(DATAFILE, O_WRONLY, 0);
    }
    write(fd, products, size);
    close(fd);
}

void lock_product(int product_id)
{
    struct product products[MAX_PRODUCTS];
    read_from_file(products, MAX_PRODUCTS * sizeof(struct product));
    int index = -1;
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        if (product_id == products[i].P_ID)
        {
            index = i;
            break;
        }
    }
    int key = ftok(".", 'a');
    int semid = semget(key, MAX_PRODUCTS + 1, 0);
    struct sembuf sb;
    sb.sem_flg = 0;
    sb.sem_num = index;
    sb.sem_op = -1;
    semop(semid, &sb, 1);
    printf("Product %d is locked\n", product_id);
}

void unlock_product(int product_id)
{
    struct product products[MAX_PRODUCTS];
    read_from_file(products, MAX_PRODUCTS * sizeof(struct product));
    int index = -1;
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        if (product_id == products[i].P_ID)
        {
            index = i;
            break;
        }
    }
    int key = ftok(".", 'a');
    int semid = semget(key, MAX_PRODUCTS + 1, 0);
    struct sembuf sb;
    sb.sem_flg = 0;
    sb.sem_num = index;
    sb.sem_op = 1;
    semop(semid, &sb, 1);
    printf("Product %d is unlocked\n", product_id);
}

void write_product_to_file(struct product product, int index)
{
    int fd = open(DATAFILE, O_WRONLY, 0);
    if (fd < 0)
    {
        create_datafile();
        fd = open(DATAFILE, O_WRONLY, 0);
    }
    lseek(fd, index * sizeof(struct product), SEEK_SET);
    write(fd, &product, sizeof(struct product));
    close(fd);
}

void generate_log_file()
{
    int fd = open(LOGFILE, O_CREAT | O_RDWR, 0644), res;
    write(fd, "P_ID\tP_Name\tCost\tQuantity\n", strlen("P_ID\tP_Name\tCost\tQuantity\n") * sizeof(char));
    struct product products[MAX_PRODUCTS];
    read_from_file(products, sizeof(products));
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        if (products[i].P_ID != -1)
        {
            char buffer[1024];
            sprintf(buffer, "%d\t%s\t%d\t%d\n", products[i].P_ID, products[i].P_Name, products[i].cost, products[i].quantity);
            write(fd, buffer, strlen(buffer) * sizeof(char));
        }
    }
    close(fd);
    printf("Log file generated\n");
}

int add_product()
{
    struct product products[MAX_PRODUCTS];
    struct product product;
    read_from_file(products, sizeof(products));
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        if (products[i].P_ID == -1)
        {
            printf("Enter product id: ");
            scanf("%d", &product.P_ID);
            printf("Enter product name: ");
            scanf("%s", product.P_Name);
            printf("Enter product price: ");
            scanf("%d", &product.cost);
            printf("Enter product quantity: ");
            scanf("%d", &product.quantity);
            products[i] = product;
            write_product_to_file(product, i);
            printf("Product added\n");
            generate_log_file();
            return 0;
        }
    }
    printf("Max products reached\n");
}

int del_product()
{
    struct product products[MAX_PRODUCTS];
    struct product product;
    read_from_file(products, sizeof(products));
    printf("Enter product id: ");
    scanf("%d", &product.P_ID);
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        if (products[i].P_ID == product.P_ID)
        {
            lock_product(products[i].P_ID);
            product.P_ID = -1;
            write_product_to_file(product, i);
            unlock_product(products[i].P_ID);
            printf("Product removed\n");
            generate_log_file();
            return 0;
        }
    }
}

int update_product()
{
    struct product products[MAX_PRODUCTS];
    struct product product;
    read_from_file(products, sizeof(products));
    printf("Enter product id: ");
    scanf("%d", &product.P_ID);
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        if (products[i].P_ID == product.P_ID)
        {
            lock_product(products[i].P_ID);
            printf("Enter new price: ");
            scanf("%d", &product.cost);
            printf("Enter new quantity: ");
            scanf("%d", &product.quantity);
            products[i].cost = product.cost;
            products[i].quantity = product.quantity;
            write_product_to_file(products[i], i);
            unlock_product(products[i].P_ID);
            printf("Product updated\n");
            generate_log_file();
            return 0;
        }
    }
}

void show_products()
{
    struct product products[MAX_PRODUCTS];
    read_from_file(products, sizeof(products));
    printf("P_ID\tP_Name\tCost\tQuantity\n");
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        if (products[i].P_ID != -1)
        {
            printf("%d\t%s\t%d\t%d\n", products[i].P_ID, products[i].P_Name, products[i].cost, products[i].quantity);
        }
    }
}

void get_name(char *name, int id)
{
    struct product products[MAX_PRODUCTS];
    read_from_file(products, sizeof(products));
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        if (products[i].P_ID == id)
        {
            strcpy(name, products[i].P_Name);
        }
    }
}

int get_cost(int id)
{
    struct product products[MAX_PRODUCTS];
    read_from_file(products, sizeof(products));
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        if (products[i].P_ID == id)
        {
            return products[i].cost;
        }
    }
}

int buy_product(struct product product)
{
    struct product products[MAX_PRODUCTS];
    read_from_file(products, sizeof(products));
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        if (products[i].P_ID == product.P_ID)
        {
            products[i].quantity -= product.quantity;
            if (products[i].quantity < 0)
            {
                return -1;
            }
            write_product_to_file(products[i], i);
        }
    }
    return 0;
}

int get_quantity(int id)
{
    struct product products[MAX_PRODUCTS];
    read_from_file(products, sizeof(products));
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        if (products[i].P_ID == id)
        {
            return products[i].quantity;
        }
    }
}

int unlock_cart(struct product cart[MAX_PRODUCTS])
{
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        if (cart[i].P_ID != -1)
        {
            unlock_product(cart[i].P_ID);
        }
    }
}

int lock_cart(struct product cart[MAX_PRODUCTS])
{
    int flag = 0;
    struct product products[MAX_PRODUCTS];
    read_from_file(products, sizeof(products));
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        if (cart[i].P_ID != -1)
        {
            lock_product(cart[i].P_ID);
            if (cart[i].quantity > get_quantity(cart[i].P_ID))
            {
                flag = 1;
            }
        }
    }
    if (flag)
    {
        unlock_cart(cart);
        return -1;
    }
    return 0;
}

int buy_products(struct product cart[MAX_PRODUCTS])
{
    int flag = 0;
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        if (cart[i].P_ID != -1)
        {
            flag += buy_product(cart[i]);
            unlock_product(cart[i].P_ID);
        }
    }
    if (flag < 0)
    {
        return -1;
    }
    return 0;
}