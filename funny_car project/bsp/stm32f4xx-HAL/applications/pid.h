/**
  ******************************************************************************
  * @file		 pid.h
  * @author  Ginger
  * @version V1.0.0
  * @date    2015/11/14
  * @brief   
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#ifndef _PID_H
#define _PID_H

#include "stdint.h"
typedef enum
{
	PID_Position,
	PID_Speed
}PID_ID;

typedef struct _PID_TypeDef
{
	PID_ID id;
	
	float target;							//Ŀ��ֵ
	float lastNoneZeroTarget;
	float kp;
	float ki;
	float kd;
	
	float   measure;					//����ֵ
	float   err;							//���
	float   last_err;      		//�ϴ����
	
	float pout;
	float iout;
	float dout;
	
	float output;						//�������
	float last_output;			//�ϴ����
	
	float MaxOutput;				//����޷�
	float IntegralLimit;		//�����޷�
	float DeadBand;			  //����������ֵ��
	float ControlPeriod;		//��������
	float  Max_Err;					//������
	
					uint32_t thistime;
					uint32_t lasttime;
					uint8_t dtime;	
	
	void (*f_param_init)(struct _PID_TypeDef *pid,  //PID������ʼ��
				   PID_ID id,
				   float maxOutput,
				   float integralLimit,
				   float deadband,
				   float controlPeriod,
					 float max_err,     
					 float  target,
				   float kp,
				   float ki,
				   float kd);
				   
	void (*f_pid_reset)(struct _PID_TypeDef *pid, float kp,float ki, float kd);		//pid���������޸�
	float (*f_cal_pid)(struct _PID_TypeDef *pid, float measure, float target);   //pid����
}PID_TypeDef;

void pid_reset(PID_TypeDef * pid, float kp, float ki, float kd);

void pid_init(void);
#endif

