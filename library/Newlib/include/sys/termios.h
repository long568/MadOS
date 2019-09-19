#ifndef	__NL__TERMIOS__H__
# define __NL__TERMIOS__H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

# define _XCGETA  (('x'<<8)|1)
# define _XCSETA  (('x'<<8)|2)
# define _XCSETAW (('x'<<8)|3)
# define _XCSETAF (('x'<<8)|4)
# define _TCSBRK  (('T'<<8)|5)
# define _TCFLSH  (('T'<<8)|7)
# define _TCXONC  (('T'<<8)|6)

# define TCOOFF 0
# define TCOON  1
# define TCIOFF 2
# define TCION  3

# define TCIFLUSH   0
# define TCOFLUSH   1
# define TCIOFLUSH  2

# define NCCS 13

# define TCSAFLUSH  _XCSETAF
# define TCSANOW    _XCSETA
# define TCSADRAIN  _XCSETAW
# define TCSADFLUSH _XCSETAF

# define IGNBRK 000001
# define BRKINT 000002
# define IGNPAR 000004
# define INPCK  000020
# define ISTRIP 000040
# define INLCR  000100
# define IGNCR  000200
# define ICRNL  000400
# define IXON   002000
# define IXOFF  010000

# define OPOST  000001
# define OCRNL  000004
# define ONLCR  000010
# define ONOCR  000020
# define TAB3   014000

# define CLOCAL 004000
# define CREAD  000200
# define CSIZE  000060
# define CS5    0
# define CS6    020
# define CS7    040
# define CS8    060
# define CSTOPB 000100
# define HUPCL  002000
# define PARENB 000400
# define PAODD  001000

#define CCTS_OFLOW 010000
#define CRTS_IFLOW 020000
#define CRTSCTS (CCTS_OFLOW | CRTS_IFLOW)

# define ECHO   0000010
# define ECHOE  0000020
# define ECHOK  0000040
# define ECHONL 0000100
# define ICANON 0000002
# define IEXTEN 0000400 /* anybody know *what* this does?! */
# define ISIG   0000001
# define NOFLSH 0000200
# define TOSTOP 0001000

# define VEOF   4 /* also VMIN -- thanks, AT&T */
# define VEOL   5 /* also VTIME -- thanks again */
# define VERASE 2
# define VINTR  0
# define VKILL  3
# define VMIN   4 /* also VEOF */
# define VQUIT  1
# define VSUSP  10
# define VTIME  5 /* also VEOL */
# define VSTART 11
# define VSTOP  12

# define B0      0
# define B50     50
# define B75     75
# define B110    110
# define B134    134
# define B150    150
# define B200    200
# define B300    300
# define B600    600
# define B1200   1200
# define B1800   1800
# define B2400   2400
# define B4800   4800
# define B9600   9600
# define B19200  19200
# define B38400  38400
# define B57600  57600
# define B115200 115200

typedef unsigned char cc_t;
typedef unsigned short tcflag_t;
typedef unsigned long speed_t;

struct termios {
  tcflag_t  c_iflag;
  tcflag_t  c_oflag;
  tcflag_t  c_cflag;
  tcflag_t  c_lflag;
  char      c_line;
  cc_t      c_cc[NCCS];
  speed_t   c_ispeed;
  speed_t   c_ospeed;
};

# define O_NDELAY _FNDELAY
# define PARODD   PAODD
# define IXANY    0x00000800

# define cfgetospeed(tp)	  ((tp)->c_ospeed)
# define cfgetispeed(tp)	  ((tp)->c_ispeed)
# define cfsetospeed(tp, s)	(((tp)->c_ospeed = (s)), 0)
# define cfsetispeed(tp, s)	(((tp)->c_ispeed = (s)), 0)

int tcgetattr(int fd, struct termios *termios_p);
int tcsetattr(int fd, int actions, const struct termios *termios_h);
// int tcdrain(int fd);
// int tcflow(int, int flowtype);
int tcflush(int fd, int in_out_selector);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* _SYS_TERMIOS_H */
