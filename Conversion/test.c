#include <STC15F2K60S2.H>
#include <intrins.h>
#include <stdlib.h>

/*---------�궨��---------*/
#define uint unsigned int
#define uchar unsigned char
#define cstAdcPower 0X80   /*ADC��Դ����*/
#define cstAdcFlag 0X10	   /*��A/Dת����ɺ�cstAdcFlagҪ�������*/
#define cstAdcStart 0X08   /*��A/Dת����ɺ�cstAdcStart���Զ����㣬����Ҫ��ʼ��һ��ת��������Ҫ��λ*/
#define cstAdcSpeed90 0X60 /*ADCת���ٶ� 90��ʱ������ת��һ��*/
#define cstAdcChs17 0X07   /*ѡ��P1.7��ΪA/D����*/

/*---------ͳһʹ�õı�---------*/
//����ܶ�ѡ�ź�
//0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
//0 1 2 3 4 5 6 7 8 9 A  B  C  D  E  F  �� L  h	 q  ?   
uchar arrSegSelect[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x00, 0x38, 0x74, 0x67, 0xa7};
//�����λѡ�ź�
uchar arrDigitSelect[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07}; //�����0-7
//�������ʾ��
uint arrPrintSelect[] = {16, 16, 16, 16, 16, 16, 16, 16};

/*---------���ű�������---------*/
sbit sbtLedSel = P2 ^ 3; 	//�������LED���л�����
sbit SDA = P4 ^ 0;		 	//I2C���ߵ�������
sbit SCL = P5 ^ 5;		 	//I2C���ߵ�ʱ����
sbit sbtBeep = P3 ^ 4;	 	//����������
sbit sbtKey1 = P3 ^ 2; 		//����1����
sbit sbtKey2 = P3 ^ 3; 		//����2����
sbit sbtKey3 = P1 ^ 7; 		//����3����

/*---------��������---------*/
uchar uiLed = 0x00;		  //LED��ֵ�Ĵ�
uchar tmpLed = 0x01;	  //LED��ֵ�ݴ�
uchar i = 0;			  //�����ɨ����ʾѭ��
uint answer = 0;		  //��
uint score = 0;			  //�÷�
uint highest_sc = 0;	  //��߷�
uint highest_er = 0;	  //��߷����
uint player = 0;		  //��ǰ��Ϸ���
uint ques = 0;			  //����
uint ques0, ques1, ques2; //����
uint ques_flag = 0;		  //�����־
uchar option = 0x00;	  //ѡ��
uint back = 0;			  //���˱�־
uchar addr = 0x00;		  //��ַ

/*---------��ʱ�Ӻ���--------*/
void delay(uint n)		//ͨ����ʱ
{
	while (n--)
		;
}
void delay_ms(uint n)	//��ʱn����
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
void delay4() 			//��ʱ4us
{
	;
	;
}

//I2C���߳�ʼ��
void IIC_init() 
{
	SCL = 1;
	delay4();
	SDA = 1;
	delay4();
}

//���������ź�
void start() 
{
	SDA = 1;
	delay4();
	SCL = 1;
	delay4();
	SDA = 0;
	delay4();
}

//ֹͣ�ź�
void stop() 
{
	SDA = 0;
	delay4();
	SCL = 1;
	delay4();
	SDA = 1;
	delay4();
}

//�ӻ�Ӧ���ź�
void respons() 
{
	uchar i = 0;
	SCL = 1;
	delay4();
	while (SDA == 1 && (i < 255)) //��ʾ����һ��ʱ����û���յ���������Ӧ����
		i++;					  //������Ĭ�ϴ��ڼ��Ѿ��յ����ݶ����ٵȴ�Ӧ���źš�
	SCL = 0;
	delay4();
}

//��24C02дһ���ֽ�����
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

//��24C02��һ���ֽ�����
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

//��24C02�ĵ�ַaddr��д��һ������date
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

//��24C02��addr��ַ����һ���ֽ�����
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

//�ϴ����� ��Ϸ��ȷ����Դ𰸽����ϴ��ͱȶԣ��������洢��
void upload()
{
	uiLed = 0x00;
	tmpLed = 0x01;
	ques_flag = 0;
	answer = P0;
	addr=player+2;
	if (answer == ques)		//����ȷ
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
	else					//����
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

/*---------��ȡADֵ�Ӻ���--------*/
unsigned char GetADC()
{
	uchar ucAdcRes;
	ADC_CONTR = cstAdcPower | cstAdcStart | cstAdcSpeed90 | cstAdcChs17; //û�н�cstAdcFlag��1�������ж�A/D�Ƿ����
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	while (!(ADC_CONTR & cstAdcFlag))
		;					  //�ȴ�ֱ��A/Dת������
	ADC_CONTR &= ~cstAdcFlag; //cstAdcFlagE�����0
	ucAdcRes = ADC_RES;		  //��ȡAD��ֵ
	return ucAdcRes;
}

/*---------��ȡ��������ֵ�Ӻ���--------*/
uchar NavKeyCheck()
{
	unsigned char key;
	key = GetADC(); //��ȡAD��ֵ
	if (key != 255) //�а�������ʱ
	{
		delay(200);
		key = GetADC();
		if (key != 255) //�������� ���а�������
		{
			key = key & 0xE0;	  //��ȡ��3λ������λ����
			key = _cror_(key, 5); //ѭ������5λ ��ȡA/Dת������λֵ����С���
			return key;
		}
	}
	return 0x07; //û�а�������ʱ����ֵ0x07
}

/*---------�������������Ӻ���--------*/
void NavKey_Process()
{
	uchar ucNavKeyCurrent; //����������ǰ��״̬
	uchar ucNavKeyPast;	   //��������ǰһ��״̬

	ucNavKeyCurrent = NavKeyCheck(); //��ȡ��ǰADCֵ
	if (ucNavKeyCurrent != 0x07)	 //���������Ƿ񱻰��� ������0x07��ʾ�а���
	{
		ucNavKeyPast = ucNavKeyCurrent;
		while (ucNavKeyCurrent != 0x07) //�ȴ����������ɿ�
			ucNavKeyCurrent = NavKeyCheck();

		switch (ucNavKeyPast)
		{
		case 0x00: //��
			if (tmpLed == 0x00)
				tmpLed = 0x01;
			else
				tmpLed = tmpLed & 0x00;
			break;
		case 0x01:				//�ϼ�����ʾ�����ּ�1
			if (tmpLed == 0x01) //value����0x80ʱ�����¸���ֵ0x01
				tmpLed = 0x80;
			else
				tmpLed = tmpLed >> 1;
			break;
		case 0x04:				//�¼�����ʾ�����ּ�1
			if (tmpLed == 0x80) //value����0x80ʱ�����¸���ֵ0x01
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

/*---------����--------*/
void question()
{
	ques = rand() % 256;
	ques0 = ques % 10;
	ques1 = ques / 10 % 10;
	ques2 = ques / 100 % 10;
}

/*---------����--------*/
void clear()
{
	for (i = 0; i < 8; i++)
		arrPrintSelect[i] = 16;
}

/*---------ͨ�����--------*/
void print()
{
	for (i = 0; i < 8; i++)
	{
		P0 = 0;
		P2 = arrDigitSelect[i];				  //ѡ������ܵ�λ��
		P0 = arrSegSelect[arrPrintSelect[i]]; //��ʾ��Ӧ����ֵ
		delay_ms(1);
	}
}

/*---------��Ϸ���--------*/
void print_game()
{
	clear();
	arrPrintSelect[0] = highest_sc;
	arrPrintSelect[3] = score;
	arrPrintSelect[5] = ques2;
	arrPrintSelect[6] = ques1;
	arrPrintSelect[7] = ques0;
}

/*---------�˵����--------*/
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

/*---------���а����--------*/
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

/*---------��¼�������--------*/
void print_login()
{
	clear();
	arrPrintSelect[2] = 1;
	arrPrintSelect[3] = 13;
	arrPrintSelect[7] = player;
}

/*---------�ʺ����--------*/
void print_back()
{
	clear();
	arrPrintSelect[6] = 19;
	arrPrintSelect[7] = 20;
}

/*---------����--------*/
int game_back()
{
	while (1)
	{
		sbtLedSel = 0;
		print_back();
		print();
		if (sbtKey1 == 0)
		{
			delay_ms(5); //��ʱ����
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
			delay_ms(5); //��ʱ����
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

/*---------��Ϸ������--------*/
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
		P0 = uiLed | tmpLed; //LED��ʾ
		delay_ms(1);
		NavKey_Process(); //��ȡ�����������
		if (sbtKey1 == 0)
		{
			delay_ms(5); //��ʱ����
			if (sbtKey1 == 0)
			{
				while (!sbtKey1)
					;
				upload();
			}
		}
		if (sbtKey2 == 0)
		{
			delay_ms(5); //��ʱ����
			if (sbtKey2 == 0)
			{
				while (!sbtKey2)
					;
				uiLed = uiLed ^ tmpLed;
			}
		}
		if (back == 1)
			return;

		//����˳�����
	}
}

/*---------ע��--------*/
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
			delay_ms(5); //��ʱ����
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
			delay_ms(5); //��ʱ����
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

/*---------����--------*/
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
			delay_ms(5); //��ʱ����
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
			delay_ms(5); //��ʱ����
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

/*---------���ô洢��--------*/
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
			delay_ms(5); //��ʱ����
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
			delay_ms(5); //��ʱ����
			if (sbtKey2 == 0)
			{
				while (!sbtKey2)
					;
				return;
			}
		}
	}
}

/*---------��ʼ������--------*/
void Init()
{
	P0M1 = 0x00;
	P0M0 = 0xff;
	P2M1 = 0x00;
	P2M0 = 0x08;
	P3M1 = 0x00;
	P3M0 = 0x10;	  //����P3^4Ϊ����ģʽ
	sbtBeep = 0;	  //������������0���Ա���������
	sbtLedSel = 0;	  //ѡ���������Ϊ���
	P1ASF = 0x80;	  //P1.7��Ϊģ�⹦��A/Dʹ��
	ADC_RES = 0;	  //ת���������
	ADC_CONTR = 0x8F; //cstAdcPower = 1
	CLK_DIV = 0X00;	  //ADRJ = 0    ADC_RES��Ÿ߰�λ���
	IT0 = 0;		  //����IT0�����ش���
	IT1 = 0;
	EA = 1;		 //CPU�����ж�
	TMOD = 0x00; //���ö�ʱ��0��������ʽ0��16λ�Զ���װ��ʱ��
	TH0 = 0xff;	 //�趨��ʱ��0�ĳ�ֵ
	TL0 = 0x03;
	ET0 = 1; //�򿪶�ʱ��0�ж�����λ
	TR0 = 1;
	P0 = 0x00; //�ر�P0�˿�
	ques_flag = 0;
	sbtLedSel = 0; //��ѡ���������
	uiLed = 0x00;
	highest_sc = read_add(0); //��ȡ��߷�
	highest_er = read_add(1); //��ȡ������
}

/*---------������--------*/
void main()
{
	Init();
	IIC_init(); //IIC���߳�ʼ��
	while (1)
	{
		sbtLedSel = 0;
		print_manu();
		print();
		if (sbtKey1 == 0)
		{
			delay_ms(5); //��ʱ����
			if (sbtKey1 == 0)
			{
				while (!sbtKey1)
					;
				if (option == 0x00)
					login(); //ע�Ტ��ʼ��Ϸ
				else if (option == 0x01)
					list(); //�鿴����
				else
					restart(); //����
			}
		}
		if (sbtKey2 == 0)
		{
			delay_ms(5); //��ʱ����
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
