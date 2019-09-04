#include "gy-86.h"

static struct rt_i2c_bus_device *i2c_bus = RT_NULL;     /* I2C�����豸��� */
#define I2C_BUS_NAME  "i2c1"

rt_err_t mpu6050_read_byte(struct rt_i2c_bus_device *bus, rt_uint16_t reg, rt_uint8_t *value)
{
		struct rt_i2c_msg msgs[2];
		rt_uint8_t tmp = reg;
	
		msgs[0].addr = MPU_ADDR;            /* �ӻ���ַ */
    msgs[0].flags = RT_I2C_WR;     /* ����־ */
    msgs[0].len = 1;
		msgs[0].buf = &tmp;

		msgs[1].addr  = MPU_ADDR;  /* Slave address */
    msgs[1].flags = RT_I2C_RD ;        /* Read flag */
    msgs[1].buf   = value;              /* Read data pointer */
    msgs[1].len   = 1;
		if (rt_i2c_transfer(bus, msgs, 2) == 2)
    {
        return RT_EOK;
    }
    else
    {
				rt_kprintf("from mpu6050 reg %x read byte error\n",reg);
        return -RT_ERROR;
    }
}
rt_err_t mpu6050_read_bytes(struct rt_i2c_bus_device *bus, rt_uint16_t reg, rt_uint8_t *buf, rt_uint8_t len)
{
		struct rt_i2c_msg msgs[2];
		rt_uint8_t tmp = reg;
	
		msgs[0].addr = MPU_ADDR;            /* �ӻ���ַ */
    msgs[0].flags = RT_I2C_WR;     /* ����־ */
    msgs[0].len = 1;
		msgs[0].buf = &tmp;

		msgs[1].addr  = MPU_ADDR;  /* Slave address */
    msgs[1].flags = RT_I2C_RD ;        /* Read flag */
    msgs[1].buf   = buf;              /* Read data pointer */
    msgs[1].len   = len;
		if (rt_i2c_transfer(bus, msgs, 2) == 2)
    {
        return RT_EOK;
    }
    else
    {
				rt_kprintf("from mpu6050 reg %x read bytes error\n",reg);
        return -RT_ERROR;
    }
}

rt_err_t mpu6050_init(void)
{
	mpu6050_write_byte(i2c_bus, MPU_PWR_MGMT1_REG, 0X80);	//��λMPU6050
  rt_thread_mdelay(100);
	mpu6050_write_byte(i2c_bus, MPU_PWR_MGMT1_REG,0X00);	//����MPU6050 
	
	mpu6050_write_byte(i2c_bus, MPU_CFG_REG, 0x06);
	mpu6050_write_byte(i2c_bus, MPU_GYRO_CFG_REG, 0x18);//��2000dps
	mpu6050_write_byte(i2c_bus, MPU_ACCEL_CFG_REG, 0x00);//��2g
	
	//������Ҫ�����Լ����������üĴ���
	//�����õ�ͨ�˲������Ի���������ݸ��¡�
	//��ʵ������ʿ죬�����ʿ�Ӧ�ò���Ҫ����Ҫ��Ӧ���Ǵ���260Hz
	rt_uint16_t samplae_rate = 8000;
	mpu6050_write_byte(i2c_bus, MPU_SAMPLE_RATE_REG, 8000/samplae_rate -1);
	mpu6050_write_byte(i2c_bus, MPU_CFG_REG, 0);//�����õ�ͨ�˲�
	//��ʵ�ֲ���Ǹ������棬MPU_CFG_REG������Ϊ0�Ļ�����������8kHz�����ٶ���1kHz�Ĳ����ʣ����ߣ��������ǲ�ʹ�õ�ͨ�˲�����
	//�����ϵ�һЩ˵���������ͨ�˲������˵���Ƶ��������ʵ�ʽǶȱ仯��Ƶ�ʿ϶����ܳ������Ƶ�ʣ������˾Ͳ�����mpu6050�ˣ�������������
	//5.5ms���������е�ͣ������õĻ�����3.8ms����ʵҲ�ò�������ȥ��
	//���Ժ�����pid��������Ҫ��С��3.8ms��û�����ˡ�
	
	rt_uint8_t read_value = 0;
	mpu6050_read_byte(i2c_bus, MPU_DEVICE_ID_REG, &read_value);
	
	if(read_value != 0x68)
	{
		return -RT_ERROR;
	}
	mpu6050_write_byte(i2c_bus, MPU_PWR_MGMT1_REG, 0X01);
	mpu6050_write_byte(i2c_bus, MPU_PWR_MGMT2_REG, 0X00);
	
	mpu6050_write_byte(i2c_bus, MPU_SAMPLE_RATE_REG, 8000/samplae_rate -1);
	mpu6050_write_byte(i2c_bus, MPU_CFG_REG, 0);
	return RT_EOK;
}
void gy86_init(void)
{
		i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(I2C_BUS_NAME);
		if (i2c_bus == RT_NULL)
    {
        rt_kprintf("\ncan't find %s device!\n", I2C_BUS_NAME);
				return;
    }
		/*init MPU6050*/
		if(mpu6050_init() == -RT_ERROR)
		{
				rt_kprintf("\nmpu6050 init error\n");
				return;
		}
		rt_kprintf("\nmpu6050 init success\n");
		
		/*init HMC5883L*/
		if(hmc5883l_init() == -RT_ERROR)
		{
				rt_kprintf("\nhmc5883l_init init error\n");
				return;
		}
		rt_kprintf("\nhmc5883l_init init success\n");
		return ;
}
rt_err_t mpu6050_get_accelerometer(struct imu_data* imu)
{
	rt_uint8_t buf[6];
	if(mpu6050_read_bytes(i2c_bus, MPU_ACCEL_XOUTH_REG, buf, 6) != RT_EOK)return -RT_ERROR;
	imu->ax=((rt_uint16_t)buf[0]<<8)|buf[1];  
	imu->ay=((rt_uint16_t)buf[2]<<8)|buf[3];  
	imu->az=((rt_uint16_t)buf[4]<<8)|buf[5];
	return RT_EOK;
}
rt_err_t mpu6050_get_gyroscope(struct imu_data* imu)
{
	rt_uint8_t buf[6];
	if(mpu6050_read_bytes(i2c_bus, MPU_GYRO_XOUTH_REG, buf, 6) != RT_EOK)return -RT_ERROR;
	imu->gx=((rt_uint16_t)buf[0]<<8)|buf[1];  
	imu->gy=((rt_uint16_t)buf[2]<<8)|buf[3];  
	imu->gz=((rt_uint16_t)buf[4]<<8)|buf[5];
	return RT_EOK;
}
rt_err_t mpu6050_write_byte(struct rt_i2c_bus_device *bus, rt_uint16_t reg, rt_uint8_t value)
{
		struct rt_i2c_msg msgs;
		rt_uint8_t buf[2];
		msgs.addr = MPU_ADDR;            /* �ӻ���ַ */
    msgs.flags = RT_I2C_WR;     /* д��־ */
		buf[0] = reg;
		buf[1] = value;
    msgs.len = 2;
		msgs.buf = buf;
		if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
				rt_kprintf("mpu6050 write reg &x error\n",reg);
        return -RT_ERROR;
    }
}
rt_err_t hmc5883l_write_byte(struct rt_i2c_bus_device *bus, rt_uint16_t reg, rt_uint8_t value)
{
		struct rt_i2c_msg msgs;
		rt_uint8_t buf[2];
		msgs.addr = HMC5883L_Addr;            /* �ӻ���ַ */
    msgs.flags = RT_I2C_WR;     /* д��־ */
		buf[0] = reg;
		buf[1] = value;
    msgs.len = 2;
		msgs.buf = buf;
		if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
				rt_kprintf("hmc5883l write reg %x error\n",reg);
        return -RT_ERROR;
    }
}
rt_err_t hmc5883l_read_bytes(struct rt_i2c_bus_device *bus, rt_uint16_t reg, rt_uint8_t *buf, rt_uint8_t len)
{
		struct rt_i2c_msg msgs[2];
		rt_uint8_t tmp = reg;
	
		msgs[0].addr = HMC5883L_Addr;            /* �ӻ���ַ */
    msgs[0].flags = RT_I2C_WR;     /* ����־ */
    msgs[0].len = 1;
		msgs[0].buf = &tmp;

		msgs[1].addr  = HMC5883L_Addr;  /* Slave address */
    msgs[1].flags = RT_I2C_RD ;        /* Read flag */
    msgs[1].buf   = buf;              /* Read data pointer */
    msgs[1].len   = len;
		if (rt_i2c_transfer(bus, msgs, 2) == 2)
    {
        return RT_EOK;
    }
    else
    {
				rt_kprintf("from hmc5883l reg %x read bytes error\n",reg);
        return -RT_ERROR;
    }
}

rt_err_t hmc5883l_init(void)
{
	mpu6050_write_byte(i2c_bus, MPU_USER_CTRL_REG,0X00);//close Master Mode 
	mpu6050_write_byte(i2c_bus, MPU_INTBP_CFG_REG,0X02);//turn on Bypass Mode 
	
	hmc5883l_write_byte(i2c_bus,HMC58X3_R_CONFA,0x78);   
	//75Hz��������ֲ��������0x78����8����ƽ�����˲��ˣ���һ����
	
  hmc5883l_write_byte(i2c_bus,HMC58X3_R_CONFB,0x00);   //������Χ+��4.7Ga	390 counts/��˹
  hmc5883l_write_byte(i2c_bus,HMC58X3_R_MODE,0x00);    //
	return RT_EOK;
}
rt_err_t hmc5883l_get_magnetic(struct imu_data* imu)
{
	rt_uint8_t buf[6] = {0};
	
	hmc5883l_read_bytes(i2c_bus, HMC5883L_RA_DATA_OUTPUT_X_MSB, buf, 6);

	imu->mx=(buf[0] << 8) | buf[1]; //Combine MSB and LSB of X Data output register
	imu->mz=(buf[2] << 8) | buf[3]; //Combine MSB and LSB of Y Data output register
	imu->my=(buf[4] << 8) | buf[5]; //Combine MSB and LSB of Z Data output register
	
	if(imu->mx>0x7fff)imu->mx-=0xffff;	  
	if(imu->my>0x7fff)imu->my-=0xffff;
	if(imu->mz>0x7fff)imu->mz-=0xffff;
	return RT_EOK;
}
