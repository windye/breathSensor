#include<unistd.h>
#include"breathSensor.h"
#include"generateCmd.h"
#include <string.h>
#include <jni.h>
#define NORMAL_SAMPLE_MODE                 0x00
#define PEAK_SAMPLE_MODE                       0x01

static BreathSensor * sigInstance_nddTF  = NULL;
 static void  device_init(JNIEnv *env, jobject thiz, jstring deviceName, jobject recv_buf)
 {
	 void *bufAdr;
	 jlong bufLen;
	 int  deviceStatus =0;  // 0表示设备初始化成功，-1表示失败

	bufAdr = env-> GetDirectBufferAddress(recv_buf);
	bufLen = env->GetDirectBufferCapacity(recv_buf);
    const  char * deviceStr = env->GetStringUTFChars(deviceName,NULL); //
	sigInstance_nddTF = BreathSensor::getInstance(env,thiz,deviceStr,(unsigned  char *)bufAdr, bufLen,deviceStatus);
	env->ReleaseStringUTFChars(deviceName, deviceStr); // 使用完后释放本地资源，避免资源浪费
	if(deviceStatus == -1) // 想java层抛出异常
	{
		 		jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
		 		env->ThrowNew(exceptionClazz, "Unable to open the device file, no permission or no such file");
		 		return;
	}


 }


 static void start_button_handler( JNIEnv *env, jobject thiz, jint mod)  // mod ==0  start_sampling  mod ==1 start_peak_sampling
{
	const unsigned char * cmd_buf; // 设备命令存放的地址

	if(mod ==NORMAL_SAMPLE_MODE )
	  	cmd_buf = generate_cmd( START_SAMPLING);
	else
		cmd_buf = generate_cmd( START_PEAK_SAMPLING);
	if( (sigInstance_nddTF->send_cmd(cmd_buf,    strlen( (char*)cmd_buf) ) )  == -1 )
	{
		jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
		env->ThrowNew(exceptionClazz, "Error when write the [start sampling] command ");
		goto exit;

	}
	sleep(2); // 等待设备完成初始化
	if(sigInstance_nddTF->start_recv_thread( )  == -1) // 启动接收线程
	{
			jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
			env->ThrowNew(exceptionClazz, "create the data-receiving thread failed");
			goto exit;

	}
exit:
			return;

}
/*********************************************************************************************************************
 * 将条件变量ThreadStop_Flag设为true时接收线程阻塞
 **********************************************************************************************************************/
static void stop_button_handler(JNIEnv *env, jobject thiz)
{
	const unsigned char * cmd_buf;
	cmd_buf = generate_cmd( STANDBY_ON); // 跳过ARMED 模式，直接进入STANDBY 模式
	if(sigInstance_nddTF->send_cmd(cmd_buf,    strlen( (char*)cmd_buf) )  == -1)
	{
			jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
			env->ThrowNew(exceptionClazz, "Error when write the [standby_on] command ");
			goto exit;
	}


	pthread_mutex_lock(&	sigInstance_nddTF->mut); //改变条件变量，停止数据接收线程
	if( 	sigInstance_nddTF->ThreadStop_Flag == false ) // 线程停止后，linux内核缓冲区中可能还残留数据，但不要紧，
		sigInstance_nddTF->ThreadStop_Flag = true;// 下次启动测量的时候，会做清空操作，并复位读写指针
	pthread_mutex_unlock(&	sigInstance_nddTF->mut);
exit:
				return;

}
static  void update_readPointer(JNIEnv *env, jobject thiz, jint readPointer)
{
		sigInstance_nddTF->m_readPointer = readPointer;
}

static  jint  get_writePointer(JNIEnv *env, jobject thiz)
{
		return  sigInstance_nddTF->m_writePointer;
}

static JNINativeMethod gMethods[] = {
		{ "device_native_init",           "(Ljava/lang/String;Ljava/nio/ByteBuffer;)V", (void *) device_init} ,
		{ "start_button_native_handler",    "(I)V",   (void *) start_button_handler} ,
		{ "stop_button_native_handler",    "( )V",   (void *) stop_button_handler} ,
		{ "update_native_readPointer",    "(I)V",   (void *) update_readPointer} ,
		{ " get_native_writePointer",         "()I",   (void *)  get_writePointer} ,

};

//JNI_OnLoad默认会在System.loadLibrary过程中自动调用到，因而可利用此函数，进行动态注册
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env = NULL;
	jint result = JNI_FALSE;

	//获取env指针
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) {
		return result;
	}
	if (env == NULL) {
		return result;
	}
	//获取类引用
	jclass clazz = env->FindClass("com/example/breathsensorjni/BreathSensor");
	if (clazz == NULL) {
		return result;
	}
	//注册方法
	if (env->RegisterNatives(clazz, gMethods,
			sizeof(gMethods) / sizeof(gMethods[0])) < 0) {
		return result;
	}
	//成功
	result = JNI_VERSION_1_6;
	return result;
}

