//-------------------------------------------------------------
// 文件名： "RS232.h"
//-------------------------------------------------------------
//功能：	1.包含RS232.c所用头文件
//		2.进行相关宏定义
//      3.声明全局变量
//-------------------------------------------------------------
// 作者： 刘亚彬
// 创建日期：2016年12月27日
// 修改日期：
// 版本：v0.1
//-------------------------------------------------------------

#ifndef _RS232_H
#define _RS232_H

//-------------------------------------------------------------
// INCLUDE
//-------------------------------------------------------------
#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"
#include "GlobalDefine.h"
#include "communication.h"

//-------------------------------------------------------------
// 宏定义
//-------------------------------------------------------------
#define RS232_ADD 1							//RS232设备地址

#define RS232 myCommunication.RS232Com
#define FIFO_SEND_NUM 10						//一次向FIFO发送缓冲区中写入的最多数据个数，不能超过14
#define SEND_END_TICK 3							//定义数据扔到缓冲区后到发送完毕后的时间N~N-1(系统滴答数)
#define RECF_END_TICK 3							//定义一帧内两个接收字节间隔最大值N~N+1(系统滴答数)

//-------------------------------------------------------------
// 全局变量声明
//-------------------------------------------------------------

//-------------------------------------------------------------
// 函数声明
//-------------------------------------------------------------
void UseRS232(void);
interrupt void ScibTx_ISR(void);
interrupt void ScibRx_ISR(void);

#endif
