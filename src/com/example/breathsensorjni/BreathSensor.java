package com.example.breathsensorjni;
import java.lang.String;
import java.nio.ByteBuffer;


public class BreathSensor
{
	   static
	    {
	    		System.loadLibrary("breathSensor_jni"); //会自动加载 libbreathSensor_jni.so 库
	    		// 在这里可以启动对设备插入和拔出的监听线程，然后由native回调java函数，通知java层
	    		
	    }
	   
	   private static final int 	 NORMAL_SAMPLE_MODE = 0x00;
	   private static final int 	 PEAK_SAMPLE_MODE = 0x01;
	   

	    private  volatile boolean  test_stopFlag; //   voaltile  
	    private ByteBuffer m_buf;
	    private int  m_bufSize;
	    private int m_readPointer;
	    String  m_deviceName;  // ttyUSB0
	    
	    public native void  device_native_init(String deviceName, ByteBuffer recv_buf);
	    public  native void start_button_native_handler(int sample_mode);
	    public  native void stop_button_native_handler(BreathSensor device);
	    public native void update_native_readPointer(int readPointer);
	    public native int  	 get_native_writePointer();
	    
	    public BreathSensor(String  deviceName, int  bufSize) // BreathSensor 类的构造函数 传入设备名 和缓冲区大小
	    {
	    	m_deviceName = deviceName;	
	    	m_readPointer = 0;
	    	m_bufSize = bufSize;
	    	m_buf = ByteBuffer.allocateDirect(bufSize);
	    	device_native_init(deviceName, m_buf);	 // 在native层创建 全局BreathSensor 对象
	    	//update_native_readPointer(m_readPointer);// 不是必须的因位native 层初始化时已经将 readPointer =0
	    }
	    
	    // 留给业务逻辑的接口
	    public void show_data(int readPtr, int writePtr)
	    {
	    	
	    }
	    
	    //java层中画图应该会有一个单独的线程，否则会影响UI 的流畅性
	    public void  read_buf_data() throws InterruptedException
	    {
			    	while(test_stopFlag == false  )  // 测试没有停止
			    	{
					    	int bytesToRead;
					    	int  native_writePointer = get_native_writePointer(); 
					    	bytesToRead = (native_writePointer + m_bufSize - m_readPointer )% m_bufSize;  // 缓冲区可读的字节数, 缓冲区为空时返回0
					    	if  (   bytesToRead != 0 )   // 缓冲区可读
						    {
						    		if(native_writePointer > m_readPointer)	
						    		{
							    			show_data(m_readPointer, native_writePointer - 1);
							    			m_readPointer += (native_writePointer -m_readPointer);
							    			update_native_readPointer(m_readPointer);
							    			
						    		}
						    		else //native_writePointer <m_readPointer
						    		{
						    			show_data(m_readPointer, m_bufSize -1);
						    			m_readPointer = 0 ;
						    			update_native_readPointer(m_readPointer);
						    		}
					    			
					    	}
					    	else
					    		Thread.sleep(100);
				    	
			    }
	    }
}