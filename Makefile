all: admin.out client.out init.out

admin.out: admin.c product.c
	cc admin.c product.c -o admin.out -g

client.out: client.c product.c
	cc client.c product.c -o client.out -g

init.out: init.c product.c
	cc init.c product.c -o init.out -g

clean:
	rm -f admin.out client.out init.out