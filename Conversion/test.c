#include <STC15F2K60S2.H>
#include <intrins.h>
#include <stdlib.h>
#define uint  unsigned int
#define uchar unsigned char

/*---------�궨��---------*/
#define cstAdcPower 0X80     /*ADC��Դ����*/
#define cstAdcFlag 0X10      /*��A/Dת����ɺ�cstAdcFlagҪ��������*/
#define cstAdcStart 0X08     /*��A/Dת����ɺ�cstAdcStart���Զ����㣬����Ҫ��ʼ��һ��ת��������Ҫ��λ*/
#define cstAdcSpeed90 0X60   /*ADCת���ٶ� 90��ʱ������ת��һ��*/
#define cstAdcChs17 0X07     /*ѡ��P1.7��ΪA/D����*/

uchar arrSegSelect[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x00};
uchar arrDigitSelect[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};   //�����0-7
uchar arrLEDselect[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}; //LED��

/*---------���ű�������---------*/
sbit sbtLedSel = P2 ^ 3;  //�������LED���л�����

/*---------��������---------*/
uchar uiLed = 0x00;        //LED��ֵ�Ĵ�
uchar tmpLed = 0x01;       //LED��ֵ�ݴ�
uchar i = 0;               //�����ɨ����ʾѭ��
uint answer=0; 					   //��
uint score=0; 						 //�÷�
uint highest_sc=0;				 //��߷�
uint ques=0;							 //����
uint ques0,ques1,ques2;
uint ques_flag=0;		       //�����־


/*---------���ű�������---------*/
sbit sbtBeep = P3 ^ 4;                  //����������
sbit sbtKey1 = P3 ^ 2;                  //����1����
sbit sbtKey2 = P3 ^ 3;
sbit sbtKey3 = P1 ^ 7;
/*---------��������---------*/
bit btBeepFlag;                        //���Ʒ��������صı�־λ

/*---------��ʼ������--------*/
void init()
{
		P0M1 = 0x00;
    P0M0 = 0xff;
    P2M1 = 0x00;
    P2M0 = 0x08;
    P3M1 = 0x00;
    P3M0 = 0x10;                  //����P3^4Ϊ����ģʽ

    TMOD = 0x00;                  //���ö�ʱ��0��������ʽ0��16λ�Զ���װ��ʱ��
    TH0 = 0xff;                   //�趨��ʱ��0�ĳ�ֵ
    TL0 = 0x03;
    EA = 1;                       //�����ж�
    ET0 = 1;                      //�򿪶�ʱ��0�ж�����λ
    TR0 = 1;
		IT0 = 0;            //����IT0�����ش���
    IT1 = 0;
    btBeepFlag = 0;                //��־λ��1
    P0 = 0x00;                    //�ر�P0�˿�
    sbtBeep = 0;                  //������������0���Ա���������
		ques_flag=0;
    sbtLedSel = 0;  //��ѡ���������
		uiLed = 0x00;
}

/*---------��ʱ�Ӻ���--------*/
void delay( uint n )
{
    while( n-- );
}



void upload()
{
		uiLed = 0x00;
		tmpLed = 0x01; 
		ques_flag=0;
		answer=P0;
		if(answer==ques)
		{
			score++;
			btBeepFlag = ~btBeepFlag;
			delay(600);
			btBeepFlag = ~btBeepFlag;
			if(score==15)
			{
					btBeepFlag = ~btBeepFlag;
					delay(600);
					btBeepFlag = ~btBeepFlag;
			}
		}
		else
		{
				if(score>highest_sc)
						highest_sc=score;
				btBeepFlag = ~btBeepFlag;
				delay(600);
				btBeepFlag = ~btBeepFlag;
				score=0;
		}
}

/*---------��ȡADֵ�Ӻ���--------*/
unsigned char GetADC()
{
    uchar ucAdcRes;
    ADC_CONTR = cstAdcPower | cstAdcStart | cstAdcSpeed90 | cstAdcChs17;//û�н�cstAdcFlag��1�������ж�A/D�Ƿ����
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    while( !( ADC_CONTR & cstAdcFlag ) ); //�ȴ�ֱ��A/Dת������
    ADC_CONTR &= ~cstAdcFlag;           //cstAdcFlagE������0
    ucAdcRes = ADC_RES;                 //��ȡAD��ֵ
    return ucAdcRes;
}

/*---------��ȡ��������ֵ�Ӻ���--------*/
uchar NavKeyCheck()
{
    unsigned char key;
    key = GetADC();     //��ȡAD��ֵ
    if( key != 255 )    //�а�������ʱ
    {
        delay(200);
        key = GetADC();
        if( key != 255 )            //�������� ���а�������
        {
            key = key & 0xE0;       //��ȡ��3λ������λ����
            key = _cror_( key, 5 ); //ѭ������5λ ��ȡA/Dת������λֵ����С���
            return key;
        }
    }
    return 0x07;        //û�а�������ʱ����ֵ0x07
}

/*---------�������������Ӻ���--------*/
void NavKey_Process()
{
    uchar ucNavKeyCurrent;  //����������ǰ��״̬
    uchar ucNavKeyPast;     //��������ǰһ��״̬

    ucNavKeyCurrent = NavKeyCheck();    //��ȡ��ǰADCֵ
    if( ucNavKeyCurrent != 0x07 )       //���������Ƿ񱻰��� ������0x07��ʾ�а���
    {
        ucNavKeyPast = ucNavKeyCurrent;
        while( ucNavKeyCurrent != 0x07 )        //�ȴ����������ɿ�
            ucNavKeyCurrent = NavKeyCheck();

        switch( ucNavKeyPast )
        {
            case 0x00 :                     //K3
                upload();
                break;
            case 0x01 :                     //��
						{
								if( tmpLed == 0x01 )        //value����0x80ʱ�����¸���ֵ0x01
										tmpLed = 0x80;
								else
										tmpLed = tmpLed >> 1;
                break;
						}
            case 0x05 :                     //��
            {
								if( tmpLed == 0x80 )        //value����0x80ʱ�����¸���ֵ0x01
										tmpLed = 0x01;
								else
										tmpLed = tmpLed << 1;
                break;
								
						}
        }
    }
}

void question()
{
		ques=rand()%256;
		ques0=ques%10;
		ques1=ques/10%10;
		ques2=ques/100%10;
}

void print()
{
		P2=arrDigitSelect[0];
		P0=arrSegSelect[highest_sc];
		delay( 500 );
		P2=arrDigitSelect[3];
		P0=arrSegSelect[score];
		delay( 500 );
		P2=arrDigitSelect[5];
		P0=arrSegSelect[ques2];
		delay( 500 );
		P2=arrDigitSelect[6];
		P0=arrSegSelect[ques1];
		delay( 500 );
		P2=arrDigitSelect[7];
		P0=arrSegSelect[ques0];
		delay( 500 );
}
/*---------������--------*/
void main()
{
    init();
    while( 1 )
    {
				sbtLedSel = 0;
				if(ques_flag==0)
				{
						question();
						ques_flag=1;
				}
				print();
        sbtLedSel = 1;
				P0 = 0x00;
        P0 = uiLed|tmpLed;                     //LED��ʾ
        delay( 100 );                  //��ʱ1ms
				NavKey_Process();
        if( sbtKey1 == 0 )
        {
            delay( 100 );                     //��ʱ����
            if( sbtKey1 == 0 )
            {
                uiLed=uiLed|tmpLed;
            }
        }
				else if (sbtKey2 ==0)
				{
					delay( 100 );                     //��ʱ����
            if( sbtKey2 == 0 )
            {
								tmpLed=tmpLed&0x00;
						}
				}
    }
}

/*---------T0��ʱ���жϷ���������--------*/
void T0_Process() interrupt 1
{
    if( btBeepFlag )
    {
        sbtBeep = ~sbtBeep;            //��������ʹ�÷���������
    }
    else
    {
        sbtBeep = 0;                  //ֹͣ����������sbtBeep�˿����ڵ͵�ƽ
    }
}