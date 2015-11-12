#ifndef   _GENERATECMD_H_
#define  _GENERATECMD_H_    1
# define  STANDBY_ON   		 			   	0x00
#define    STANDBY_OFF       			      0x01
#define    START_SAMPLING				   	0x02
#define     STOP_SAMPLING		       	  0x03
#define  START_PEAK_SAMPLING   	  0x04
#define    STOP_PEAK_SAMPLING  	 0x05
#define   SET_CHANNEL                	 0x06

unsigned char * generate_cmd(unsigned int cmd_with_para);

#endif
