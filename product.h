#ifndef PRODUCT_H
#define PRODUCT_H
#define DATAFILE "product.dat"
#define LOGFILE "update.log"
#define MAX_PRODUCTS 100

struct product
{
    int P_ID;
    char P_Name[20];
    int cost;
    int quantity;
};

void create_datafile();
void read_from_file(struct product products[MAX_PRODUCTS], int size);
int add_product();
int del_product();
int update_product();
void show_products();
void get_name(char *name, int id);
int get_cost(int id);
int unlock_cart(struct product cart[MAX_PRODUCTS]);
int lock_cart(struct product cart[MAX_PRODUCTS]);
int buy_product(struct product p);
int buy_products(struct product cart[MAX_PRODUCTS]);

#endif