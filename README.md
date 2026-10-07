Ainda irei fazer as inforamções necesárias aqui
apenas salvando enquanto isso.


novas modificação save

Para rodar o "Teste do Caos" (Sem Mutex):
gcc -Wall -pthread -DSEM_SINCRONIZACAO codigo.c -o le_caos
./le_caos


Para rodar o modo Normal (Com Mutex):
gcc -Wall -pthread codigo.c -o le
./le