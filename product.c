#include "product.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

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
    struct flock lock;
    lock.l_type = F_RDLCK;
    int fd = open(DATAFILE, O_RDONLY, 0);
    if (fd < 0)
    {
        create_datafile();
        fd = open(DATAFILE, O_RDONLY, 0);
    }
    fcntl(fd, F_SETLKW, &lock);
    read(fd, products, size);
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
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

void lock_product(int index, int fd)
{
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = index * sizeof(struct product);
    lock.l_len = sizeof(struct product);
    fcntl(fd, F_SETLKW, &lock);
}

void unlock_product(int index, int fd)
{
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = index * sizeof(struct product);
    lock.l_len = sizeof(struct product);
    fcntl(fd, F_SETLK, &lock);
}

void write_product_to_file(int fd, struct product product, int index)
{
    lseek(fd, index * sizeof(struct product), SEEK_SET);
    write(fd, &product, sizeof(struct product));
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
            int fd = open(DATAFILE, O_WRONLY, 0);
            lock_product(i, fd);
            printf("Enter product id: ");
            scanf("%d", &product.P_ID);
            printf("Enter product name: ");
            scanf("%s", product.P_Name);
            printf("Enter product price: ");
            scanf("%d", &product.cost);
            printf("Enter product quantity: ");
            scanf("%d", &product.quantity);
            products[i] = product;
            write_product_to_file(fd, product, i);
            unlock_product(i, fd);
            close(fd);
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
            int fd = open(DATAFILE, O_WRONLY, 0);
            lock_product(i, fd);
            product.P_ID = -1;
            write_product_to_file(fd, product, i);
            unlock_product(i, fd);
            close(fd);
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
            int fd = open(DATAFILE, O_WRONLY, 0);
            // lock_product(i, fd);
            struct flock lock;
            lock.l_type = F_WRLCK;
            lock.l_whence = SEEK_SET;
            lock.l_start = i * sizeof(struct product);
            lock.l_len = sizeof(struct product);
            fcntl(fd, F_SETLKW, &lock);
            printf("Enter new price: ");
            scanf("%d", &product.cost);
            printf("Enter new quantity: ");
            scanf("%d", &product.quantity);
            products[i].cost = product.cost;
            products[i].quantity = product.quantity;
            write_product_to_file(fd, products[i], i);
            // unlock_product(i, fd);
            lock.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lock);
            close(fd);
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

int set_buy_lock(int id)
{
    struct product products[MAX_PRODUCTS];
    read_from_file(products, sizeof(products));
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        if (products[i].P_ID == id)
        {
            struct flock lock;
            lock.l_type = F_WRLCK;
            lock.l_whence = SEEK_SET;
            lock.l_start = i * sizeof(struct product);
            lock.l_len = sizeof(struct product);
            int fd = open(DATAFILE, O_RDWR, 0);
            fcntl(fd, F_SETLKW, &lock);
            return products[i].quantity;
        }
    }
}

int buy_product(struct product product, int flag)
{
    struct product products[MAX_PRODUCTS];
    read_from_file(products, sizeof(products));
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        if (products[i].P_ID == product.P_ID)
        {
            struct flock lock;
            lock.l_type = F_UNLCK;
            lock.l_whence = SEEK_SET;
            lock.l_start = i * sizeof(struct product);
            lock.l_len = sizeof(struct product);
            int fd = open(DATAFILE, O_RDWR, 0);
            if (flag)
            {
                fcntl(fd, F_SETLK, &lock);
                close(fd);
                return -1;
            }
            if (products[i].quantity >= product.quantity)
            {
                products[i].quantity -= product.quantity;
                lseek(fd, i * sizeof(struct product), SEEK_SET);
                write(fd, &products[i], sizeof(struct product));
                fcntl(fd, F_SETLK, &lock);
                return 0;
            }
            else
            {
                return -1;
            }
        }
    }
}