#include "breathSensor.h"
#include"my_log.h"
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<termios.h>
#include<fcntl.h>
#include<jni.h>


#define BUF_READABLE(len)    (((m_writePointer + len) - m_readPointer)%len)   // 可读的字节数, 缓冲区为空时返回0
#define BUF_WRITEABLE(len)     (len -BUF_READABLE(len)-1)    //可写的字节数（缓冲区留一个字节不写）,缓冲区满时返回0

BreathSensor * BreathSensor:: m_sigInstance  = NULL; // 初始化静态成员变量

/*串口类的构造函数，完成对串口的初始化
 * */
BreathSensor::BreathSensor(JNIEnv *env, jobject breathSensor,
		const char * device_name, unsigned char * bufAdr, int  bufLen,int & status)
	:m_env(env),
	 m_breathSensor(env->NewGlobalRef(breathSensor)) ,
	 m_bufAdr(bufAdr), m_bufLen(bufLen)
{
	m_fd = open_port(device_name);
	if(m_fd == -1)
	{
		jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
		env->ThrowNew(exceptionClazz, "Unable to open the device file, no permission or no such file");
		status = -1;
		goto exit;

	}
	if(  config_port() == -1  )
	{
		status = -1;
		goto exit;
	}
	ThreadExist_Flag  = false; // 初始时接收线程不存在
	ThreadStop_Flag  = true; // 初始时接收进程停止，等待start button

	 m_readPointer =0;
	 m_writePointer =  0;
	mut = PTHREAD_MUTEX_INITIALIZER;
	cond = PTHREAD_COND_INITIALIZER;
exit:
			return;
}

/*
 * 由于设备的唯一性，这里使用了单例模式，由于不存在多线程竞争创建对象的问题，所以没有实现锁，如果需要可以使用
 * linux线程互斥量实现
 * */
BreathSensor * BreathSensor::getInstance(JNIEnv *env, jobject breathSensor,
		const  char * device_name, unsigned char * bufAdr, int bufLen, int & status)
{
	 if(NULL == m_sigInstance)
	   {
     // Lock();
	       if(NULL == m_sigInstance )
	   {
	    	   m_sigInstance = new BreathSensor(env,  breathSensor, device_name,  bufAdr, bufLen,status);
	     }
	   //     UnLock();
	    }
   return m_sigInstance;

}


/*****************************************************************
	* 名称：                    open_port
	* 功能：                    打开串口并返回串口设备文件描述
	* 入口参数：            fd    :文件描述符     port :串口号(ttyS0,ttyS1,ttyS2)
	* 出口参数：            正确返回为0，错误返回为-1
	*****************************************************************/
	int  BreathSensor:: open_port(const char* device_name)
	{
		  int fd;
		  if(access(device_name, F_OK) != 0)
		  {
			  MY_LOG_ERROR("The device not exist");
			  return(-1);
		  }
		  if(access(device_name,R_OK|W_OK) != 0)
		  {
			     MY_LOG_ERROR("No permission to  operate the device file");
				   return(-1);
		 }
	     fd = open( device_name, O_RDWR|O_NOCTTY|O_NONBLOCK|O_NDELAY);
	      if (-1== fd)
	        {
	    	  	  	MY_LOG_ERROR("ERROR when open the device file");
	                 return(-1);
	         }

	     MY_LOG_INFO("Device open success: fd = %d", fd);
	      return fd;
	}
	/*****************************************************************
		* 名称：                    config_port
		* 功能：                    设置串口波特率
		* 入口参数：
		* 		* 出口参数：            正确返回为0，错误返回为-1
		*****************************************************************/
	int  BreathSensor::config_port( )
	{
		        struct termios cfg;
		        if (tcgetattr(m_fd, &cfg) != 0)
		        {
		         MY_LOG_ERROR("Configure port tcgetattr() failed 1");
		            close(m_fd);
		            return -1;
		        }
		    /* cfmakeraw()  sets  the terminal to something like the "raw" mode (man termios)
          termios_p->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP| INLCR | IGNCR | ICRNL | IXON);
           termios_p->c_oflag &= ~OPOST;
           termios_p->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
           termios_p->c_cflag &= ~(CSIZE | PARENB);
           termios_p->c_cflag |= CS8; */
		   	  // 默认 8位数据位，1位停止位，无校验，无对输入输出处理
		        cfmakeraw(&cfg);
		        cfsetispeed(&cfg, B57600);  //波特率写死 57600
		        cfsetospeed(&cfg, B57600);
		        if (tcsetattr(m_fd, TCSANOW, &cfg) != 0)
		        {
		          MY_LOG_ERROR("Configure port tcsetattr() failed 2");
		             close(m_fd);
		             return -1;
		        }
		        return 0;
	}
	/*******************************************************************
	* 名称：                send_cmd
	* 功能：                发送数据
	* 入口参数：        fd                  :文件描述符
	*                      cmd_buf  		  :存放要发送的命令
	*                      cmd_len          :命令的长度
	* 出口参数：        正确返回为命令的长度，错误返回为-1
	*******************************************************************/
	int BreathSensor::send_cmd(const  unsigned char *cmd_buf, size_t cmd_len)
	{
	    int count = 0;
	   count = write(m_fd, cmd_buf, cmd_len); // 非阻塞输入
	    if (count == cmd_len )
	       {
	              return count;
	       }
	    else
	        {

	            tcflush(m_fd,TCOFLUSH);// 没有一次性写入成功，擦除发送缓冲区
	                return -1;
	        }
    }
	/*******************************************************************
		* 名称：                recv_thread
		* 功能：              数据接收线程，以非阻塞方式往recv_buf中写数据
		* 入口参数：        xargs             :传给线程的参数
		*
		* 出口参数：
		*******************************************************************/
	void BreathSensor::recv_thread()
	{
		   MY_LOG_DEBUG("the  true thread  is starting....");
			while(1)
			{
						pthread_mutex_lock(&mut);
						MY_LOG_DEBUG("the  true thread  get the mutex lock");
						while( ThreadStop_Flag == true ) //线程停止
						{
								MY_LOG_DEBUG("the  true thread  blocked on the cond");
								pthread_cond_wait(&cond, &mut);   //  线程阻塞后，会自动释放互斥量，唤醒后锁住互斥量
						}

						pthread_mutex_unlock(&mut);
						MY_LOG_DEBUG("the  true thread  release the mutex lock");
						if( BUF_WRITEABLE(m_bufLen)  != 0) //线程启动，且缓冲区可写
						{
								int count;
								size_t nBytesToRead;
								if(m_writePointer >=m_readPointer)
								{

											nBytesToRead =m_bufLen - m_writePointer;
											if(m_readPointer  == 0) // 如果readPointer ==0,将buf的最后一个byte留空，符合判满公式
													--nBytesToRead;
											count = read(m_fd, m_bufAdr+m_writePointer, nBytesToRead );
											if(count == -1 )
											{
														if( errno != EAGAIN) // 发生错误
														{
																MY_LOG_ERROR("error when read data from linux kernel");
																goto exit;
														}
														else // 没有数据可读   errno == EAGAIN
														{
																count = 0;
														//		MY_LOG_DEBUG(" errno == EAGAIN, wait ...");
														//		sleep(1);

														}
											}

											m_writePointer += count;  // 更新写指针

											MY_LOG_DEBUG("the writePointer is :%d", m_writePointer);
											if( m_writePointer  == m_bufLen) // 缓冲区后半部分已经写满
											{
													m_writePointer  = 0;
											}
								}

//								pthread_mutex_lock(&mut);  // 改善对stop键的响应
//								while( ThreadStop_Flag == true ) //线程停止
//											pthread_cond_wait(&cond, &mut);
//								pthread_mutex_unlock(&mut);

								else //(m_writePointer <m_readPointer)
								{
										nBytesToRead = m_readPointer - m_writePointer -1; // 留一个byte，以满足判满公式
										count = read(m_fd, m_bufAdr+m_writePointer, nBytesToRead );
										if(count == -1 )
										{
													if( errno != EAGAIN) // 发生错误
													{
															MY_LOG_ERROR("error when read data from linux kernel");
															goto exit;
													}
													else // 没有数据可读
													{
																count = 0;
															//	sleep(1);

													}
										}
										m_writePointer  += count;
								}
						}
						else //   not   BUF_WRITEABLE(m_bufLen)缓冲区不可写，循环等待
								usleep(10000);  // 休眠10 ms

			}
	exit:   // 线程中出现错误，线程退出
	    	MY_LOG_ERROR("thread exit due to read error");
			pthread_exit((void *) 0);

	}

/****************************************************************************
 * 这个全局函数对非静态成员函数进行了包装，使之可以作为线程的回调函数，我们也可以通过定义
 * 静态成员函数然后将this指针作为参数传入实现相同的目的
 * *********************************************************************************/
	void * BreathSensor::recv_thread_wrap(void *  arg)
	{
		    MY_LOG_DEBUG("the thread  wrap is starting....");
		   BreathSensor * ptr = (BreathSensor * ) arg ;
		   MY_LOG_DEBUG("m_fd= %d, m_bufAdr=0X%x,m_bufLen=%u",ptr->m_fd, ptr->m_bufAdr,ptr->m_bufLen);
		   ptr ->recv_thread();  //非静态成员函数默认会传入this指针
			pthread_exit((void *) 0);
	}

	/*******************************************************************
	* 名称：                start_recv_thread
	* 功能：               启动数据接收线程,在start sampling 命令后调用
	* 入口参数：
	*
	* 出口参数：        正确返回为命令的长度，错误返回为-1
	*******************************************************************/
	int  BreathSensor::start_recv_thread()
	{
		int err = 0;
		pthread_t ntid;
		pthread_attr_t  attr;
		if( pthread_attr_init(&attr) != 0 ) // 因为还没有初始化成功，不需要销毁attr 直接返回
		{
			MY_LOG_ERROR("pthread_attr_init failed");
			err = -1;
			return -1;
		}
		if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0 )
		{
			MY_LOG_ERROR("pthread_attr_setdetachstate failed");
			err = -1;
			goto exit;
		}
		if( ThreadExist_Flag == false ) //线程不存在，则创建线程
		{
			ThreadExist_Flag = true;
			ThreadStop_Flag = false;
			tcflush(m_fd, TCIFLUSH); // 清空linux内核残留的数据
			m_writePointer = 0; // 复位接收缓冲区的读写指针
			m_readPointer =0;
			err = pthread_create (&ntid, &attr, recv_thread_wrap,  (void *)  this );

			if(err != 0)
			{
				MY_LOG_ERROR("Thread creation failed \n");
				err = -1;
				goto exit;
			}
		}
		else //线程已经存在
		{
				pthread_mutex_lock(&mut);
				if( ThreadStop_Flag == true) // 线程已存在且处于阻塞状态
				{
						ThreadStop_Flag = false;  // 将停止的线程启动
						tcflush(m_fd, TCIFLUSH); // 清空早期接收的数据
						m_writePointer = 0; // 复位接收缓冲区的读写指针
						m_readPointer =0;
						pthread_mutex_unlock(&mut); // 释放锁
						pthread_cond_signal(&cond); //唤醒阻塞在条件变量上的线程
				}
				else //线程已处于启动状态，提示已经开始接收数据，请先停止
				{
						MY_LOG_INFO("Test is doing, please stop first ! \n");

				}
		}
	exit:
		pthread_attr_destroy(&attr);
		return err;
	}



