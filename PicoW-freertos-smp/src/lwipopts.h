#ifndef _LWIPOPTS_H_
#define _LWIPOPTS_H_

// Use FreeRTOS sys_arch
#define NO_SYS                          0
#define LWIP_TIMERS                     1

// Enable sockets and netconn
#define LWIP_SOCKET                     1
#define LWIP_NETCONN                    1

// Enable DHCP
#define LWIP_DHCP                       1

// Enable DNS
#define LWIP_DNS                        1

// Memory options
#define MEM_ALIGNMENT                   4
#define MEM_SIZE                        4096

// PBUF options
#define PBUF_POOL_SIZE                  16
#define PBUF_POOL_BUFSIZE               512

// TCP options
#define LWIP_TCP                        1
#define TCP_TTL                         255
#define TCP_QUEUE_OOSEQ                 0
#define TCP_MSS                         1460
#define TCP_SND_BUF                     (4 * TCP_MSS)
#define TCP_WND                         (4 * TCP_MSS)
#define LWIP_TIMEVAL_PRIVATE 0

// UDP options
#define LWIP_UDP                        1
#define UDP_TTL                         255

// ICMP
#define LWIP_ICMP                       1

// Threadsafe background (pico_cyw43_arch_lwip_threadsafe_background)
#define CYW43_LWIP                         1

#endif /* _LWIPOPTS_H_ */
