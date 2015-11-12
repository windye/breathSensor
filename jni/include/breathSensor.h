#ifndef   _BREATH_SENSOR_H_
#define     _BREATH_SENSOR_H_     1

#include<pthread.h>
#include<jni.h>


class BreathSensor{
public:
	volatile bool ThreadExist_Flag;  // 读取线程只创建一次,这个标志表明线程是否已经创建
	volatile bool ThreadStop_Flag;  // 标志线程是否停止
   pthread_mutex_t mut ;
	pthread_cond_t  cond;
	int    m_fd;
	unsigned char * m_bufAdr;
	JNIEnv * m_env;
	size_t m_bufLen;
	volatile int m_readPointer;
	volatile int m_writePointer;
	jobject  m_breathSensor;

	// 单例模式，由于这里只有主线程会创建类，所以就不用加锁了

	static BreathSensor * getInstance(JNIEnv *env, jobject breathSensor,
			const  char * device_name, unsigned char * bufAdr, int bufLen,int &status );

	static void * recv_thread_wrap(void *  arg);

	int open_port(const char *device_name);
	int config_port(); //
	int  send_cmd(const unsigned char * cmd_buf, size_t  cmd_len);
	int  start_recv_thread( ); // 在发送 start sampling 命令后需要马上调用该函数
    void recv_thread ( );

	int    inc_read_pointer(unsigned int  read_len);
private:
	static BreathSensor *  m_sigInstance ;
	BreathSensor(JNIEnv *env, jobject breathSensor,
			const  char * device_name, unsigned char * bufAdr, int bufLen, int &status);

};
// 将非静态成员包一下，使之可以作为线程的回调函数
void * recv_thread_wrap(void *  ptr);


#endif
