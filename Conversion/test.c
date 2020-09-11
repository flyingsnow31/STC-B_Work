#include <STC15F2K60S2.H>
#include <intrins.h>
#include <stdlib.h>

/*---------宏定义---------*/
#define uint unsigned int
#define uchar unsigned char
#define cstAdcPower 0X80   /*ADC电源开关*/
#define cstAdcFlag 0X10	   /*当A/D转换完成后，cstAdcFlag要软件清零*/
#define cstAdcStart 0X08   /*当A/D转换完成后，cstAdcStart会自动清零，所以要开始下一次转换，则需要置位*/
#define cstAdcSpeed90 0X60 /*ADC转换速度 90个时钟周期转换一次*/
#define cstAdcChs17 0X07   /*选择P1.7作为A/D输入*/

/*---------统一使用的表单---------*/
//数码管段选信号
//0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
//0 1 2 3 4 5 6 7 8 9 A  B  C  D  E  F  空 L  h	 q  ?   
uchar arrSegSelect[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x00, 0x38, 0x74, 0x67, 0xa7};
//数码管位选信号
uchar arrDigitSelect[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07}; //数码管0-7
//数码管显示表单
uint arrPrintSelect[] = {16, 16, 16, 16, 16, 16, 16, 16};

/*---------引脚别名定义---------*/
sbit sbtLedSel = P2 ^ 3; 	//数码管与LED灯切换引脚
sbit SDA = P4 ^ 0;		 	//I2C总线的数据线
sbit SCL = P5 ^ 5;		 	//I2C总线的时钟线
sbit sbtBeep = P3 ^ 4;	 	//蜂鸣器引脚
sbit sbtKey1 = P3 ^ 2; 		//按键1引脚
sbit sbtKey2 = P3 ^ 3; 		//按键2引脚
sbit sbtKey3 = P1 ^ 7; 		//按键3引脚

/*---------变量定义---------*/
uchar uiLed = 0x00;		  //LED灯值寄存
uchar tmpLed = 0x01;	  //LED灯值暂存
uchar i = 0;			  //数码管扫描显示循环
uint answer = 0;		  //答案
uint score = 0;			  //得分
uint highest_sc = 0;	  //最高分
uint highest_er = 0;	  //最高分玩家
uint player = 0;		  //当前游戏玩家
uint ques = 0;			  //问题
uint ques0, ques1, ques2; //问题
uint ques_flag = 0;		  //问题标志
uchar option = 0x00;	  //选项
uint back = 0;			  //回退标志
uchar addr = 0x00;		  //地址

/*---------延时子函数--------*/
void delay(uint n)		//通用延时
{
	while (n--)
		;
}
void delay_ms(uint n)	//延时n毫秒
{
	while (n)
	{
		uchar i, j;
		i = 11;
		j = 190;
		do
		{
			while (--j)
				;
		} while (--i);
		n--;
	}
}
void delay4() 			//延时4us
{
	;
	;
}

//I2C总线初始化
void IIC_init() 
{
	SCL = 1;
	delay4();
	SDA = 1;
	delay4();
}

//主机启动信号
void start() 
{
	SDA = 1;
	delay4();
	SCL = 1;
	delay4();
	SDA = 0;
	delay4();
}

//停止信号
void stop() 
{
	SDA = 0;
	delay4();
	SCL = 1;
	delay4();
	SDA = 1;
	delay4();
}

//从机应答信号
void respons() 
{
	uchar i = 0;
	SCL = 1;
	delay4();
	while (SDA == 1 && (i < 255)) //表示若在一段时间内没有收到从器件的应答则
		i++;					  //主器件默认从期间已经收到数据而不再等待应答信号。
	SCL = 0;
	delay4();
}

//对24C02写一个字节数据
void writebyte(uchar date) 
{
	uchar i, temp;
	temp = date;
	for (i = 0; i < 8; i++)
	{
		temp = temp << 1;
		SCL = 0;
		delay4();
		SDA = CY;
		delay4();
		SCL = 1;
		delay4();
	}
	SCL = 0;
	delay4();
	SDA = 1;
	delay4();
}

//从24C02读一个字节数据
uchar readbyte() 
{
	uchar i, k;
	SCL = 0;
	delay4();
	SDA = 1;
	delay4();
	for (i = 0; i < 8; i++)
	{
		SCL = 1;
		delay4();
		k = (k << 1) | SDA;
		delay4();
		SCL = 0;
		delay4();
	}
	delay4();
	return k;
}

//对24C02的地址addr，写入一个数据date
void write_add(uchar addr, uchar date) 
{
	start();
	writebyte(0xa0);
	respons();
	writebyte(addr);
	respons();
	writebyte(date);
	respons();
	stop();
}

//从24C02的addr地址，读一个字节数据
uchar read_add(uchar addr) 
{
	uchar date;
	start();
	writebyte(0xa0);
	respons();
	writebyte(addr);
	respons();
	start();
	writebyte(0xa1);
	respons();
	date = readbyte();
	stop();
	return date;
}

int game_back();

//上传函数 游戏中确定后对答案进行上传和比对，结果存入存储器
void upload()
{
	uiLed = 0x00;
	tmpLed = 0x01;
	ques_flag = 0;
	answer = P0;
	addr=player+2;
	if (answer == ques)		//答案正确
	{
		score++;
		if (score == 15)
		{
			highest_sc = score;
			highest_er = player;
			write_add(0, score);
			delay_ms(5);
			write_add(1, player);
			delay_ms(5);
			write_add(player + 2, score);
			delay_ms(5);
			score = 0;
		}
		
	}
	else					//错误
	{
		if (score > highest_sc)
		{
			highest_sc = score;
			highest_er = player;
			write_add(0, score);
			delay_ms(5);
			write_add(1, player);
			delay_ms(5);
		}
		
		write_add(addr, score);
		delay_ms(5);
		score = 0;
	}
}

/*---------获取AD值子函数--------*/
unsigned char GetADC()
{
	uchar ucAdcRes;
	ADC_CONTR = cstAdcPower | cstAdcStart | cstAdcSpeed90 | cstAdcChs17; //没有将cstAdcFlag置1，用于判断A/D是否结束
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	while (!(ADC_CONTR & cstAdcFlag))
		;					  //等待直到A/D转换结束
	ADC_CONTR &= ~cstAdcFlag; //cstAdcFlagE软件清0
	ucAdcRes = ADC_RES;		  //获取AD的值
	return ucAdcRes;
}

/*---------获取导航按键值子函数--------*/
uchar NavKeyCheck()
{
	unsigned char key;
	key = GetADC(); //获取AD的值
	if (key != 255) //有按键按下时
	{
		delay(200);
		key = GetADC();
		if (key != 255) //按键消抖 仍有按键按下
		{
			key = key & 0xE0;	  //获取高3位，其他位清零
			key = _cror_(key, 5); //循环右移5位 获取A/D转换高三位值，减小误差
			return key;
		}
	}
	return 0x07; //没有按键按下时返回值0x07
}

/*---------导航按键处理子函数--------*/
void NavKey_Process()
{
	uchar ucNavKeyCurrent; //导航按键当前的状态
	uchar ucNavKeyPast;	   //导航按键前一个状态

	ucNavKeyCurrent = NavKeyCheck(); //获取当前ADC值
	if (ucNavKeyCurrent != 0x07)	 //导航按键是否被按下 不等于0x07表示有按下
	{
		ucNavKeyPast = ucNavKeyCurrent;
		while (ucNavKeyCurrent != 0x07) //等待导航按键松开
			ucNavKeyCurrent = NavKeyCheck();

		switch (ucNavKeyPast)
		{
		case 0x00: //里
			if (tmpLed == 0x00)
				tmpLed = 0x01;
			else
				tmpLed = tmpLed & 0x00;
			break;
		case 0x01:				//上键：显示的数字加1
			if (tmpLed == 0x01) //value等于0x80时，重新赋初值0x01
				tmpLed = 0x80;
			else
				tmpLed = tmpLed >> 1;
			break;
		case 0x04:				//下键：显示的数字减1
			if (tmpLed == 0x80) //value等于0x80时，重新赋初值0x01
				tmpLed = 0x01;
			else
				tmpLed = tmpLed << 1;
			break;
		case 0x02:
			back = game_back();
			break;
		}
	}

	delay(1000);
}

/*---------问题--------*/
void question()
{
	ques = rand() % 256;
	ques0 = ques % 10;
	ques1 = ques / 10 % 10;
	ques2 = ques / 100 % 10;
}

/*---------清屏--------*/
void clear()
{
	for (i = 0; i < 8; i++)
		arrPrintSelect[i] = 16;
}

/*---------通用输出--------*/
void print()
{
	for (i = 0; i < 8; i++)
	{
		P0 = 0;
		P2 = arrDigitSelect[i];				  //选择数码管的位数
		P0 = arrSegSelect[arrPrintSelect[i]]; //显示对应的数值
		delay_ms(1);
	}
}

/*---------游戏输出--------*/
void print_game()
{
	clear();
	arrPrintSelect[0] = highest_sc;
	arrPrintSelect[3] = score;
	arrPrintSelect[5] = ques2;
	arrPrintSelect[6] = ques1;
	arrPrintSelect[7] = ques0;
}

/*---------菜单输出--------*/
void print_manu()
{
	clear();
	if (option == 0x00) //GO
	{
		arrPrintSelect[0] = 0;
		arrPrintSelect[4] = 9;
		arrPrintSelect[5] = 0;
	}
	else if (option == 0x01)
	{
		arrPrintSelect[0] = 1;
		arrPrintSelect[4] = 17;
		arrPrintSelect[5] = 1;
		arrPrintSelect[6] = 5;
		arrPrintSelect[7] = 7;
	}
	else
	{
		arrPrintSelect[0] = 2;
		arrPrintSelect[6] = 10;
		arrPrintSelect[7] = 14;
	}
}

/*---------排行榜输出--------*/
void print_list()
{
	clear();
	if (option == 0x00) //high
	{
		arrPrintSelect[0] = 18;
		arrPrintSelect[1] = 1;
		arrPrintSelect[2] = 9;
		arrPrintSelect[3] = 18;
		arrPrintSelect[5] = highest_er;
		arrPrintSelect[7] = highest_sc;
	}
	else
	{
		arrPrintSelect[0] = 17;
		arrPrintSelect[1] = 1;
		arrPrintSelect[2] = 5;
		arrPrintSelect[3] = 7;
		arrPrintSelect[5] = option - 1;
		arrPrintSelect[7] = read_add(option + 1);
		delay_ms(5);
	}
}

/*---------登录界面输出--------*/
void print_login()
{
	clear();
	arrPrintSelect[2] = 1;
	arrPrintSelect[3] = 13;
	arrPrintSelect[7] = player;
}

/*---------问号输出--------*/
void print_back()
{
	clear();
	arrPrintSelect[6] = 19;
	arrPrintSelect[7] = 20;
}

/*---------返回--------*/
int game_back()
{
	while (1)
	{
		sbtLedSel = 0;
		print_back();
		print();
		if (sbtKey1 == 0)
		{
			delay_ms(5); //延时消抖
			if (sbtKey1 == 0)
			{
				while (!sbtKey1)
					;
				{
					return 1;
				}
			}
		}
		if (sbtKey2 == 0)
		{
			delay_ms(5); //延时消抖
			if (sbtKey2 == 0)
			{
				while (!sbtKey2)
					;
				{
					return 0;
				}
			}
		}
	}
}

/*---------游戏主函数--------*/
void game()
{
	clear();
	while (1)
	{
		sbtLedSel = 0;
		if (ques_flag == 0)
		{
			question();
			ques_flag = 1;
		}
		print_game();
		print();
		sbtLedSel = 1;
		P0 = 0;
		P0 = uiLed | tmpLed; //LED显示
		delay_ms(1);
		NavKey_Process(); //获取按键按下情况
		if (sbtKey1 == 0)
		{
			delay_ms(5); //延时消抖
			if (sbtKey1 == 0)
			{
				while (!sbtKey1)
					;
				upload();
			}
		}
		if (sbtKey2 == 0)
		{
			delay_ms(5); //延时消抖
			if (sbtKey2 == 0)
			{
				while (!sbtKey2)
					;
				uiLed = uiLed ^ tmpLed;
			}
		}
		if (back == 1)
			return;

		//后加退出方法
	}
}

/*---------注册--------*/
void login()
{
	option = 0x00;
	back = 0;
	while (1)
	{
		sbtLedSel = 0;
		print_login();
		print();
		if (sbtKey1 == 0)
		{
			delay_ms(5); //延时消抖
			if (sbtKey1 == 0)
			{
				while (!sbtKey1)
					;
				{
					game();
					return;
				}
			}
		}
		if (sbtKey2 == 0)
		{
			delay_ms(5); //延时消抖
			if (sbtKey2 == 0)
			{
				while (!sbtKey2)
					;
				if (player == 15)
					player = 0;
				else
					player++;
				//next
			}
		}
	}
}

/*---------排行--------*/
void list()
{
	option = 0x00;
	while (1)
	{
		sbtLedSel = 0;
		print_list();
		print();
		if (sbtKey1 == 0)
		{
			delay_ms(5); //延时消抖
			if (sbtKey1 == 0)
			{
				while (!sbtKey1)
					;
				{
					option = 0x00;
					break;
				}
			}
		}
		if (sbtKey2 == 0)
		{
			delay_ms(5); //延时消抖
			if (sbtKey2 == 0)
			{
				while (!sbtKey2)
					;
				if (option == 0x10)
					option = 0x00;
				else
					option++;
				//next
			}
		}
	}
}

/*---------重置存储器--------*/
void restart()
{
	option = 0x00;
	clear();
	while (1)
	{
		sbtLedSel = 0;
		print_back();
		print();
		if (sbtKey1 == 0)
		{
			delay_ms(5); //延时消抖
			if (sbtKey1 == 0)
			{
				while (!sbtKey1)
					;

				for (i = 0; i < 21; i++)
				{
					write_add(i, 0);
					delay_ms(10);
				}
				return;
			}
		}
		if (sbtKey2 == 0)
		{
			delay_ms(5); //延时消抖
			if (sbtKey2 == 0)
			{
				while (!sbtKey2)
					;
				return;
			}
		}
	}
}

/*---------初始化函数--------*/
void Init()
{
	P0M1 = 0x00;
	P0M0 = 0xff;
	P2M1 = 0x00;
	P2M0 = 0x08;
	P3M1 = 0x00;
	P3M0 = 0x10;	  //设置P3^4为推挽模式
	sbtBeep = 0;	  //蜂鸣器引脚置0，以保护蜂鸣器
	sbtLedSel = 0;	  //选择数码管作为输出
	P1ASF = 0x80;	  //P1.7作为模拟功能A/D使用
	ADC_RES = 0;	  //转换结果清零
	ADC_CONTR = 0x8F; //cstAdcPower = 1
	CLK_DIV = 0X00;	  //ADRJ = 0    ADC_RES存放高八位结果
	IT0 = 0;		  //设置IT0上升沿触发
	IT1 = 0;
	EA = 1;		 //CPU开放中断
	TMOD = 0x00; //设置定时器0，工作方式0，16位自动重装定时器
	TH0 = 0xff;	 //设定定时器0的初值
	TL0 = 0x03;
	ET0 = 1; //打开定时器0中断允许位
	TR0 = 1;
	P0 = 0x00; //关闭P0端口
	ques_flag = 0;
	sbtLedSel = 0; //先选择数码管亮
	uiLed = 0x00;
	highest_sc = read_add(0); //读取最高分
	highest_er = read_add(1); //读取最高玩家
}

/*---------主函数--------*/
void main()
{
	Init();
	IIC_init(); //IIC总线初始化
	while (1)
	{
		sbtLedSel = 0;
		print_manu();
		print();
		if (sbtKey1 == 0)
		{
			delay_ms(5); //延时消抖
			if (sbtKey1 == 0)
			{
				while (!sbtKey1)
					;
				if (option == 0x00)
					login(); //注册并开始游戏
				else if (option == 0x01)
					list(); //查看排行
				else
					restart(); //重置
			}
		}
		if (sbtKey2 == 0)
		{
			delay_ms(5); //延时消抖
			if (sbtKey2 == 0)
			{
				while (!sbtKey2)
					;
				if (option == 0x2)
					option = 0;
				else
					option++;
			}
		}
	}
}
