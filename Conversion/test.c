#include <STC15F2K60S2.H>
#include <intrins.h>
#include <stdlib.h>
#define uint unsigned int
#define uchar unsigned char

/*---------宏定义---------*/
#define cstAdcPower 0X80   /*ADC电源开关*/
#define cstAdcFlag 0X10	   /*当A/D转换完成后，cstAdcFlag要软件清零*/
#define cstAdcStart 0X08   /*当A/D转换完成后，cstAdcStart会自动清零，所以要开始下一次转换，则需要置位*/
#define cstAdcSpeed90 0X60 /*ADC转换速度 90个时钟周期转换一次*/
#define cstAdcChs17 0X07   /*选择P1.7作为A/D输入*/

uchar arrSegSelect[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x00};
uchar arrDigitSelect[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07}; //数码管0-7
uchar arrLEDselect[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};   //LED灯
uint arrPrintSelect[]={16,16,16,16,16,16,16,16};

/*---------引脚别名定义---------*/
sbit sbtLedSel = P2 ^ 3; //数码管与LED灯切换引脚

/*---------变量定义---------*/
uchar uiLed = 0x00;	 //LED灯值寄存
uchar tmpLed = 0x01; //LED灯值暂存
uchar i = 0;		 //数码管扫描显示循环
uint answer = 0;	 //答案
uint score = 0;		 //得分
uint highest_sc = 0; //最高分
uint ques = 0;		 //问题
uint ques0, ques1, ques2;
uint ques_flag = 0; //问题标志

/*---------引脚别名定义---------*/
sbit sbtBeep = P3 ^ 4; //蜂鸣器引脚
sbit sbtKey1 = P3 ^ 2; //按键1引脚
sbit sbtKey2 = P3 ^ 3;
sbit sbtKey3 = P1 ^ 7;
/*---------变量定义---------*/
bit btBeepFlag; //控制蜂鸣器开关的标志位
 

/*---------初始化函数--------*/
void Init()
{
	P0M1 = 0x00;
	P0M0 = 0xff;
	P2M1 = 0x00;
	P2M0 = 0x08;
	P3M1 = 0x00;
	P3M0 = 0x10; //设置P3^4为推挽模式
	
    sbtLedSel = 0;      //选择数码管作为输出
    P1ASF = 0x80;       //P1.7作为模拟功能A/D使用
    ADC_RES = 0;        //转换结果清零
    ADC_CONTR = 0x8F;   //cstAdcPower = 1
    CLK_DIV = 0X00;     //ADRJ = 0    ADC_RES存放高八位结果
    IT0 = 0;            //设置IT0上升沿触发
    IT1 = 0;
    EA = 1;             //CPU开放中断
	TMOD = 0x00; //设置定时器0，工作方式0，16位自动重装定时器
	TH0 = 0xff;	 //设定定时器0的初值
	TL0 = 0x03;
	ET0 = 1; //打开定时器0中断允许位
	TR0 = 1;
	btBeepFlag = 0; //标志位置1
	P0 = 0x00;		//关闭P0端口
	sbtBeep = 0;	//蜂鸣器引脚置0，以保护蜂鸣器
	ques_flag = 0;
	sbtLedSel = 0; //先选择数码管亮
	uiLed = 0x00;
}
/*---------延时子函数--------*/
void delay(uint n)
{
	while (n--)
		;
}
void delay_ms( uint n )
{
    while( n )
    {
        uchar i, j;
        i = 11;
        j = 190;
        do
        {
            while ( --j );
        }
        while ( --i );
        n--;
    }
}
void upload()
{
	uiLed = 0x00;
	tmpLed = 0x01;
	ques_flag = 0;
	answer = P0;
	if (answer == ques)
	{
		score++;
		btBeepFlag = ~btBeepFlag;
		delay(600);
		btBeepFlag = ~btBeepFlag;
		if (score == 2)
		{
			btBeepFlag = ~btBeepFlag;
			delay(600);
			btBeepFlag = ~btBeepFlag;
		}
	}
	else
	{
		if (score > highest_sc)
			highest_sc = score;
		btBeepFlag = ~btBeepFlag;
		delay(600);
		btBeepFlag = ~btBeepFlag;
		score = 0;
	}
}

/*---------获取AD值子函数--------*/
unsigned char GetADC()
{
    uchar ucAdcRes;
    ADC_CONTR = cstAdcPower | cstAdcStart | cstAdcSpeed90 | cstAdcChs17;//没有将cstAdcFlag置1，用于判断A/D是否结束
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    while( !( ADC_CONTR & cstAdcFlag ) ); //等待直到A/D转换结束
    ADC_CONTR &= ~cstAdcFlag;           //cstAdcFlagE软件清0
    ucAdcRes = ADC_RES;                 //获取AD的值
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

        switch( ucNavKeyPast )
        {
            case 0x00 :                     //里
			if (tmpLed == 0x00)
					tmpLed = 0x01;
				else
					tmpLed = tmpLed & 0x00;
                break;
            case 0x01 :                     //上键：显示的数字加1
                if (tmpLed == 0x01) //value等于0x80时，重新赋初值0x01
				tmpLed = 0x80;
				else
				tmpLed = tmpLed >> 1;
                break;
            case 0x04 :                     //下键：显示的数字减1
                if (tmpLed == 0x80) //value等于0x80时，重新赋初值0x01
					tmpLed = 0x01;
				else
					tmpLed = tmpLed << 1;
                break;
        }
    }

    delay(1000);
}

void question()
{
	ques = rand() % 256;
	ques0 = ques % 10;
	ques1 = ques / 10 % 10;
	ques2 = ques / 100 % 10;
}

void print()
{
	for( i = 0; i < 8; i++ )
        {
            P0 = 0;
            P2 = arrDigitSelect[i];     //选择数码管的位数
            P0 = arrSegSelect[arrPrintSelect[i]];  //显示对应的数值
            delay_ms( 1 );
        }
}

void print_game()
{
	arrPrintSelect[0]=highest_sc;
	arrPrintSelect[3]=score;
	arrPrintSelect[5]=ques2;
	arrPrintSelect[6]=ques1;
	arrPrintSelect[7]=ques0;
}

/*---------主函数--------*/
void main()
{
    Init();
    while( 1 )
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
		NavKey_Process();         //获取按键按下情况
        if (sbtKey1 == 0)
		{
			delay_ms(5); //延时消抖
			if (sbtKey1 == 0)
			{
				while( !sbtKey1 );
				uiLed = uiLed | tmpLed;
			}
		}
		if (sbtKey2 == 0)
		{
			delay_ms(5); //延时消抖
			if (sbtKey2 == 0)
			{
				while( !sbtKey2 );
                upload();
			}
		}
    }
}

/*---------T0定时器中断服务处理函数--------*/
void T0_Process() interrupt 1
{
	if (btBeepFlag)
	{
		sbtBeep = ~sbtBeep; //产生方波使得蜂鸣器发声
	}
	else
	{
		sbtBeep = 0; //停止发声，并将sbtBeep端口置于低电平
	}
}
