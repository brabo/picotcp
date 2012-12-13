#include "pico_stack.h"
#include "pico_config.h"
#include "pico_dev_vde.h"
#include "pico_ipv4.h"
#include "pico_socket.h"

#include <signal.h>
#include <unistd.h>

static struct pico_socket *client = NULL;
static struct pico_socket *send;

void wakeup(uint16_t ev, struct pico_socket *s)
{
  char buf[30];
  int r;
  uint32_t peer;
  uint16_t port;

  printf("Called wakeup\n");
  if (ev == PICO_SOCK_EV_RD) { 
    do {
      r = pico_socket_recvfrom(s, buf, 30, &peer, &port);
      printf("------------------------------------- Receive: %d\n", r);
      if (r > 0) {
        pico_socket_write(s, buf, r);
      } else if (r < 0) {
        printf("error recvfrom - %d\n",pico_err);
      }
    } while(r>0);
  }
  if (ev == PICO_SOCK_EV_CONN) { 
    if (client) {
      printf("Socket busy: try again later.\n");
    } else {
      client = pico_socket_accept(s, &peer, &port);
      send = client;
      if (client)
        printf("Connection established.\n");
      else
        printf("accept: Error.\n");
    }
  }
  if (ev == PICO_SOCK_EV_CLOSE) {
    printf("Socket received event close\n");
    
    /* test read on shut socket */
    r = pico_socket_recvfrom(s, buf, 30, &peer, &port);
    if (r >= 0)
      printf("messed up reading from closed socket\n"); //DELME
    else if (r < 0)
      printf("pico_err - socket_recvfrom - %s - good?\n",strerror(pico_err));
    
    //pico_socket_shutdown(send, PICO_SHUT_WR);
    kill(getpid(),SIGUSR1);
  }
  if (ev == PICO_SOCK_EV_FIN) {
    printf("Socket is going to be closed!\n");
    /* test read on shut socket */
    r = pico_socket_recvfrom(s, buf, 30, &peer, &port);
    if (r >= 0)
      printf("messed up reading from closed socket\n"); //DELME
    else if (r < 0)
      printf("pico_err - socket_recvfrom - %s - good?\n",strerror(pico_err));
  }
}


void callback_exit(int signum)
{
  if (signum == SIGUSR1) {
    printf("SERVER > EXIT WRITE ISSUED\n");
    pico_socket_shutdown(client, PICO_SHUT_WR);
  }
}



int main(void)
{
  unsigned char macaddr0[6] = {0,0,0,0xa,0xb,0xc};
  struct pico_device *vde0;
  struct pico_ip4 address0, netmask0, address1, netmask1;

  struct pico_socket *sk_udp, *sk_tcp;
  uint16_t port = short_be(5555);

  pico_stack_init();

  address0.addr = 0x0300280a; //  10.40.0.3
  netmask0.addr = 0x00FFFFFF;

  address1.addr = 0x0300290a; //  10.41.0.3
  netmask1.addr = 0x00FFFFFF;

  vde0 = pico_vde_create("/tmp/pic0.ctl", "vde0", macaddr0);
  if (!vde0)
    return 1;

  //vde1 = pico_vde_create("/tmp/pic1.ctl", "vde1", macaddr1);
  //if (!vde1)
  //  return 1;

  pico_ipv4_link_add(vde0, address0, netmask0);
  //pico_ipv4_link_add(vde1, address1, netmask1);

  sk_udp = pico_socket_open(PICO_PROTO_IPV4, PICO_PROTO_UDP, &wakeup);
  if (!sk_udp)
    return 2;

  if (pico_socket_bind(sk_udp, &address0, &port)!= 0)
    return 3;

  sk_tcp = pico_socket_open(PICO_PROTO_IPV4, PICO_PROTO_TCP, &wakeup);
  if (!sk_tcp)
    return 2;

  if (pico_socket_bind(sk_tcp, &address0, &port)!= 0)
    return 3;

  if (pico_socket_listen(sk_tcp, 3)!=0)
    return 3;

  send = sk_tcp;

  signal(SIGUSR1, callback_exit);

  while(1) {
    pico_stack_tick();
    usleep(2000);
  }

  return 0;

}


